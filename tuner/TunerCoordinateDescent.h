#ifndef HOWL_TUNER_COORDINATE_DESCENT_H
#define HOWL_TUNER_COORDINATE_DESCENT_H

#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "Board.h"
#include "BoardMaker.h"
#include "Option.h"
#include "tuner/TunerEvaluationState.h"
#include "tuner/TunerEvaluator.h"
#include "tuner/TunerLoss.h"
#include "tuner/TunerParameter.h"

namespace Tuner
{

struct TunerPosition
{
    std::unique_ptr<Board> board;
    double result = 0.5;
};

struct ChangedParameter
{
    std::string name;
    ParameterFamily family;
    int semanticIndex = 0;
    int initialValue = 0;
    int finalValue = 0;
    int delta = 0;
};

struct CoordinateDescentResult
{
    double baselineTrainLoss = 0.0;
    double baselineValLoss = 0.0;
    double finalTrainLoss = 0.0;
    double finalValLoss = 0.0;
    int parametersExamined = 0;
    int parametersChanged = 0;
    std::string outputFile = "tuner/tuned_parameters.tsv";
    double totalRuntimeSeconds = 0.0;
    int maxWorkerThreads = 8;
    std::vector<ChangedParameter> changedParameters;
    std::map<std::string, int> countChangedByFamily;
};

class TunerThreadPoolEvaluator
{
public:
    TunerThreadPoolEvaluator(const std::vector<TunerPosition>& dataset, double scale = 554.17, int numThreads = 8)
        : dataset_(dataset), scale_(scale), numThreads_(numThreads), stop_(false), taskCount_(0)
    {
        lut_.resize(65536);
        for (int s = -32768; s < 32768; ++s)
        {
            lut_[s + 32768] = 1.0 / (1.0 + std::pow(10.0, -static_cast<double>(s) / scale_));
        }

        threadErrors_.resize(numThreads_, 0.0);
        threadStates_.resize(numThreads_);

        for (int t = 0; t < numThreads_; ++t)
        {
            workers_.emplace_back(&TunerThreadPoolEvaluator::WorkerLoop, this, t);
        }
    }

    ~TunerThreadPoolEvaluator()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
            taskCount_++;
        }
        cvStart_.notify_all();
        for (auto& w : workers_)
        {
            if (w.joinable())
            {
                w.join();
            }
        }
    }

    double Evaluate(const TunerEvaluationState& state)
    {
        for (int t = 0; t < numThreads_; ++t)
        {
            threadStates_[t] = state;
        }
        completedCount_ = 0;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            taskCount_++;
        }
        cvStart_.notify_all();

        {
            std::unique_lock<std::mutex> lock(mutex_);
            cvDone_.wait(lock, [this]() { return completedCount_ == numThreads_; });
        }

        double totalSqError = 0.0;
        for (double err : threadErrors_)
        {
            totalSqError += err;
        }
        return totalSqError / static_cast<double>(dataset_.size());
    }

private:
    void WorkerLoop(int threadId)
    {
        std::size_t startIdx = (threadId * dataset_.size()) / numThreads_;
        std::size_t endIdx = ((threadId + 1) * dataset_.size()) / numThreads_;
        uint64_t lastTaskId = 0;

        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cvStart_.wait(lock, [this, lastTaskId]() {
                    return stop_ || (taskCount_ > lastTaskId);
                });
                if (stop_) break;
                lastTaskId = taskCount_;
            }

            const auto& localState = threadStates_[threadId];
            double localSqError = 0.0;

            for (std::size_t i = startIdx; i < endIdx; ++i)
            {
                int stmScore = TunerEvaluator::Evaluate(*dataset_[i].board, localState);
                int whiteScore = (!dataset_[i].board->sideToMove) ? stmScore : -stmScore;
                if (whiteScore < -32768) whiteScore = -32768;
                else if (whiteScore > 32767) whiteScore = 32767;
                double expected = lut_[whiteScore + 32768];
                double diff = dataset_[i].result - expected;
                localSqError += diff * diff;
            }

            threadErrors_[threadId] = localSqError;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                completedCount_++;
                if (completedCount_ == numThreads_)
                {
                    cvDone_.notify_one();
                }
            }
        }
    }

    const std::vector<TunerPosition>& dataset_;
    double scale_;
    int numThreads_;
    std::vector<std::thread> workers_;
    std::vector<TunerEvaluationState> threadStates_;
    std::vector<double> threadErrors_;
    std::vector<double> lut_;

    std::mutex mutex_;
    std::condition_variable cvStart_;
    std::condition_variable cvDone_;
    bool stop_;
    uint64_t taskCount_;
    int completedCount_ = 0;
};

class TunerCoordinateDescent
{
public:
    static bool LoadDataset(const std::string& filepath, std::vector<TunerPosition>& positions)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Could not open dataset file: " << filepath << "\n";
            return false;
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty()) continue;
            auto tabPos = line.rfind('\t');
            if (tabPos == std::string::npos) continue;

            std::string fen = line.substr(0, tabPos);
            double res = 0.5;
            try
            {
                res = std::stod(line.substr(tabPos + 1));
            }
            catch (...)
            {
                continue;
            }

            Board* b = BoardMaker::MakeInitialBoard(fen);
            if (b)
            {
                positions.push_back({std::unique_ptr<Board>(b), res});
            }
        }
        return true;
    }

    static double ComputeLoss(const std::vector<TunerPosition>& dataset,
                              const TunerEvaluationState& state,
                              double scale,
                              int numThreads = 8)
    {
        if (dataset.empty()) return 0.0;

        std::vector<double> threadErrors(numThreads, 0.0);
        std::vector<std::thread> workers;
        workers.reserve(numThreads);

        for (int t = 0; t < numThreads; ++t)
        {
            workers.emplace_back([&, t]() {
                TunerEvaluationState localState = state;
                std::size_t startIdx = (t * dataset.size()) / numThreads;
                std::size_t endIdx = ((t + 1) * dataset.size()) / numThreads;
                double localSqError = 0.0;

                for (std::size_t i = startIdx; i < endIdx; ++i)
                {
                    int stmScore = TunerEvaluator::Evaluate(*dataset[i].board, localState);
                    int whiteScore = (!dataset[i].board->sideToMove) ? stmScore : -stmScore;
                    double expected = TunerLossEvaluator::CentipawnsToExpectedWhiteScore(whiteScore, scale);
                    double diff = dataset[i].result - expected;
                    localSqError += diff * diff;
                }
                threadErrors[t] = localSqError;
            });
        }

        for (auto& w : workers) w.join();

        double totalSqError = 0.0;
        for (double err : threadErrors) totalSqError += err;
        return totalSqError / static_cast<double>(dataset.size());
    }

    static int GetFamilyDelta(ParameterFamily family)
    {
        switch (family)
        {
        case ParameterFamily::PieceValue: return 5;
        case ParameterFamily::PawnStructure: return 2;
        case ParameterFamily::PassedPawn: return 2;
        case ParameterFamily::PieceSquare: return 2;
        case ParameterFamily::CenterPresence: return 1;
        case ParameterFamily::CenterMove: return 1;
        case ParameterFamily::KingSafety: return 2;
        case ParameterFamily::Mobility: return 2;
        case ParameterFamily::Attack: return 2;
        default: return 0;
        }
    }

    static int GetFamilyDeltaPass2(ParameterFamily family)
    {
        switch (family)
        {
        case ParameterFamily::PieceValue: return 10;
        case ParameterFamily::PawnStructure: return 4;
        case ParameterFamily::PassedPawn: return 4;
        case ParameterFamily::PieceSquare: return 4;
        case ParameterFamily::CenterPresence: return 2;
        case ParameterFamily::KingSafety: return 4;
        default: return 0; // CenterMove, Mobility, Attack, Inline are frozen
        }
    }

    static int GetFamilyDeltaPass3(ParameterFamily family)
    {
        switch (family)
        {
        case ParameterFamily::PawnStructure: return 4;
        case ParameterFamily::PassedPawn: return 4;
        case ParameterFamily::PieceSquare: return 4;
        case ParameterFamily::CenterPresence: return 2;
        case ParameterFamily::KingSafety: return 4;
        default: return 0; // PieceValue, CenterMove, Mobility, Attack, Inline are frozen
        }
    }

    static int GetFamilyDeltaRefine1(ParameterFamily family)
    {
        switch (family)
        {
        case ParameterFamily::PawnStructure: return 2;
        case ParameterFamily::PassedPawn: return 2;
        case ParameterFamily::PieceSquare: return 2;
        case ParameterFamily::CenterPresence: return 1;
        case ParameterFamily::KingSafety: return 2;
        default: return 0; // PieceValue, CenterMove, Mobility, Attack, Inline are frozen
        }
    }

    static std::string FamilyToString(ParameterFamily family)
    {
        switch (family)
        {
        case ParameterFamily::PieceValue: return "PieceValue";
        case ParameterFamily::PawnStructure: return "PawnStructure";
        case ParameterFamily::PassedPawn: return "PassedPawn";
        case ParameterFamily::PieceSquare: return "PieceSquare";
        case ParameterFamily::CenterPresence: return "CenterPresence";
        case ParameterFamily::CenterMove: return "CenterMove";
        case ParameterFamily::KingSafety: return "KingSafety";
        case ParameterFamily::Mobility: return "Mobility";
        case ParameterFamily::Attack: return "Attack";
        case ParameterFamily::Inline: return "Inline";
        default: return "Unknown";
        }
    }

    static CoordinateDescentResult RunRefine1(const std::string& trainPath = "tuner-train.tsv",
                                             const std::string& valPath = "tuner-validation.tsv",
                                             const std::string& pass3InputPath = "tuner/tuned_parameters_pass3.tsv",
                                             const std::string& outputPath = "tuner/tuned_parameters_refine1.tsv",
                                             double scale = 554.17,
                                             int numThreads = 8)
    {
        CoordinateDescentResult result;
        result.outputFile = outputPath;
        result.maxWorkerThreads = numThreads;

        std::cout << "Loading training dataset from " << trainPath << "...\n";
        std::vector<TunerPosition> trainPositions;
        if (!LoadDataset(trainPath, trainPositions) || trainPositions.empty())
        {
            std::cerr << "Failed to load training dataset.\n";
            return result;
        }
        std::cout << "Loaded " << trainPositions.size() << " training positions.\n";

        std::cout << "Loading validation dataset from " << valPath << "...\n";
        std::vector<TunerPosition> valPositions;
        if (!LoadDataset(valPath, valPositions) || valPositions.empty())
        {
            std::cerr << "Failed to load validation dataset.\n";
            return result;
        }
        std::cout << "Loaded " << valPositions.size() << " validation positions.\n";

        Option::Initialize();
        TunerRegistry registry = TunerRegistry::CreateRegistry();
        TunerEvaluationState state;
        state.LoadFromRegistry(registry);

        // Load pass 3 values from pass3InputPath
        std::map<std::string, const TunerParameter*> nameMap;
        for (std::size_t i = 0; i < registry.Size(); ++i)
        {
            nameMap[registry[i].name] = &registry[i];
        }

        std::ifstream pass3File(pass3InputPath);
        if (!pass3File.is_open())
        {
            std::cerr << "Could not open pass 3 file: " << pass3InputPath << "\n";
            return result;
        }

        std::string line;
        std::getline(pass3File, line); // header
        int pass3Loaded = 0;
        while (std::getline(pass3File, line))
        {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string name;
            int initVal, finalVal, delta;
            if (ss >> name >> initVal >> finalVal >> delta)
            {
                auto it = nameMap.find(name);
                if (it != nameMap.end())
                {
                    int* ptr = state.GetParameterPointer(it->second->family, it->second->semanticIndex);
                    if (ptr)
                    {
                        *ptr = finalVal;
                        pass3Loaded++;
                    }
                }
            }
        }
        pass3File.close();
        state.Derive();
        std::cout << "Initialized state with " << pass3Loaded << " pass 3 tuned values.\n";

        // Snapshot of pass 3 candidate values for each parameter
        std::vector<int> pass3CandidateValues(registry.Size(), 0);
        for (std::size_t i = 0; i < registry.Size(); ++i)
        {
            int* ptr = state.GetParameterPointer(registry[i].family, registry[i].semanticIndex);
            pass3CandidateValues[i] = ptr ? *ptr : registry[i].currentValue;
        }

        std::cout << "Evaluating starting losses for refinement pass...\n";
        result.baselineValLoss = ComputeLoss(valPositions, state, scale, numThreads);
        result.baselineTrainLoss = ComputeLoss(trainPositions, state, scale, numThreads);

        std::cout << "Starting Train Loss: " << std::fixed << std::setprecision(6) << result.baselineTrainLoss << "\n";
        std::cout << "Starting Val Loss:   " << std::fixed << std::setprecision(6) << result.baselineValLoss << "\n";

        TunerThreadPoolEvaluator pool(trainPositions, scale, numThreads);

        auto startTime = std::chrono::high_resolution_clock::now();

        double currentLoss = result.baselineTrainLoss;
        int examinedCount = 0;
        int changedRefineCount = 0;

        // Initialize countChangedByFamily for enabled families in refinement
        result.countChangedByFamily["PawnStructure"] = 0;
        result.countChangedByFamily["PassedPawn"] = 0;
        result.countChangedByFamily["PieceSquare"] = 0;
        result.countChangedByFamily["CenterPresence"] = 0;
        result.countChangedByFamily["KingSafety"] = 0;

        for (std::size_t p = 0; p < registry.Size(); ++p)
        {
            const auto& param = registry[p];

            int delta = GetFamilyDeltaRefine1(param.family);
            if (delta <= 0)
            {
                // Frozen families: PieceValue, CenterMove, Mobility, Attack, Inline
                continue;
            }

            examinedCount++;

            int* targetPtr = state.GetParameterPointer(param.family, param.semanticIndex);
            if (!targetPtr)
            {
                continue;
            }

            int currentVal = *targetPtr;
            int pass3Val = pass3CandidateValues[p];

            const int candidates[5] = {
                currentVal - 2 * delta,
                currentVal - delta,
                currentVal,
                currentVal + delta,
                currentVal + 2 * delta
            };

            double candidateLosses[5];

            for (int c = 0; c < 5; ++c)
            {
                *targetPtr = candidates[c];
                state.Derive();
                candidateLosses[c] = pool.Evaluate(state);
            }

            // Selection rule:
            // 5. For exact ties, prefer current.
            // Otherwise prefer the smallest absolute change from current.
            int bestIdx = 2; // candidate 2 is currentVal
            double bestLoss = candidateLosses[2];

            for (int c = 0; c < 5; ++c)
            {
                if (c == 2) continue;

                double loss = candidateLosses[c];
                if (loss < bestLoss)
                {
                    bestLoss = loss;
                    bestIdx = c;
                }
                else if (loss == bestLoss)
                {
                    int candDist = std::abs(candidates[c] - currentVal);
                    int bestDist = std::abs(candidates[bestIdx] - currentVal);
                    if (candDist < bestDist)
                    {
                        bestLoss = loss;
                        bestIdx = c;
                    }
                    else if (candDist == bestDist)
                    {
                        if (std::abs(candidates[c]) < std::abs(candidates[bestIdx]))
                        {
                            bestIdx = c;
                        }
                        else if (std::abs(candidates[c]) == std::abs(candidates[bestIdx]) && candidates[c] < candidates[bestIdx])
                        {
                            bestIdx = c;
                        }
                    }
                }
            }

            int chosenVal = candidates[bestIdx];
            *targetPtr = chosenVal;
            state.Derive();
            currentLoss = bestLoss;

            if (chosenVal != pass3Val)
            {
                changedRefineCount++;
                result.countChangedByFamily[FamilyToString(param.family)]++;
            }

            if (examinedCount % 100 == 0 || examinedCount == 1345)
            {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsedSec = std::chrono::duration<double>(now - startTime).count();
                std::cout << "[" << examinedCount << "/1345] param: " << param.name
                          << " committed: " << chosenVal << " (pass3 was " << pass3Val << ")"
                          << " train loss: " << currentLoss
                          << " elapsed: " << elapsedSec << "s\n" << std::flush;
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        result.totalRuntimeSeconds = std::chrono::duration<double>(endTime - startTime).count();
        result.parametersExamined = examinedCount;
        result.parametersChanged = changedRefineCount;

        // Final losses
        result.finalTrainLoss = ComputeLoss(trainPositions, state, scale, numThreads);
        result.finalValLoss = ComputeLoss(valPositions, state, scale, numThreads);

        // Save resulting full changed-parameter set (against production defaults)
        std::ofstream outFile(outputPath);
        int totalChangedAgainstProduction = 0;
        if (outFile.is_open())
        {
            outFile << "ParameterName\tInitialValue\tFinalValue\tDelta\n";
            for (std::size_t i = 0; i < registry.Size(); ++i)
            {
                int* ptr = state.GetParameterPointer(registry[i].family, registry[i].semanticIndex);
                int finalVal = ptr ? *ptr : registry[i].currentValue;
                int prodDefault = registry[i].currentValue;
                if (finalVal != prodDefault)
                {
                    outFile << registry[i].name << "\t" << prodDefault << "\t" << finalVal << "\t" << (finalVal - prodDefault) << "\n";
                    totalChangedAgainstProduction++;
                }
            }
            outFile.close();
            std::cout << "Saved " << totalChangedAgainstProduction << " full changed parameters to " << outputPath << "\n";
        }
        else
        {
            std::cerr << "Failed to open output file: " << outputPath << "\n";
        }

        return result;
    }
};

} // namespace Tuner

#endif // HOWL_TUNER_COORDINATE_DESCENT_H
