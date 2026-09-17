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
#include <unordered_map>
#include <vector>
#include "Board.h"
#include "BoardMaker.h"
#include "EvaluationLogic.h"
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
    std::string positionKey;
    double gameResult = 0.5;
    int baselineHowlScoreCp = 0;
    double baselineHowlProbability = 0.5;
    int teacherScoreCp = 0;
    double teacherProbability = 0.5;
    bool hasTeacherScore = false;
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
    HybridLossComponents baselineTrainComponents;
    HybridLossComponents baselineValComponents;
    HybridLossComponents finalTrainComponents;
    HybridLossComponents finalValComponents;
    int parametersExamined = 0;
    int parametersChanged = 0;
    int sweeps = 0;
    int optimizerSteps = 0;
    std::string terminationReason;
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
            lut_[s + 32768] = TunerLossEvaluator::ScoreToProbability(s, scale_);
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
                if (stmScore < -32768) stmScore = -32768;
                else if (stmScore > 32767) stmScore = 32767;
                const double probability = lut_[stmScore + 32768];
                localSqError += TunerLossEvaluator::PositionLoss(
                    probability, dataset_[i].gameResult,
                    dataset_[i].baselineHowlProbability,
                    dataset_[i].hasTeacherScore,
                    dataset_[i].teacherProbability).combined;
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
    static bool IsFamilyTunable(ParameterFamily family,
                                const std::vector<ParameterFamily>& tunableFamilies)
    {
        return std::find(tunableFamilies.begin(), tunableFamilies.end(), family) !=
               tunableFamilies.end();
    }

    static bool IsParameterTunable(const TunerParameter& parameter,
                                   const std::vector<ParameterFamily>& selections)
    {
        if (IsFamilyTunable(parameter.family, selections)) return true;

        const auto selected = [&selections](ParameterFamily group)
        {
            return IsFamilyTunable(group, selections);
        };

        if (selected(ParameterFamily::PST) &&
            parameter.family == ParameterFamily::PieceSquare)
            return true;

        if (selected(ParameterFamily::Pawns))
        {
            if (parameter.family == ParameterFamily::PawnStructure ||
                parameter.family == ParameterFamily::PassedPawnV2 ||
                parameter.family == ParameterFamily::IsolatedPawn ||
                parameter.family == ParameterFamily::RookBehindPassedPawn)
                return true;
            if (parameter.family == ParameterFamily::Inline && parameter.semanticIndex == 6)
                return true;
        }

        if (selected(ParameterFamily::Pieces) &&
            (parameter.family == ParameterFamily::KnightMobility ||
             parameter.family == ParameterFamily::BishopMobility ||
             parameter.family == ParameterFamily::RookMobility ||
             parameter.family == ParameterFamily::QueenMobility ||
             parameter.family == ParameterFamily::RookFile ||
             parameter.family == ParameterFamily::KnightOutpost))
            return true;

        if (selected(ParameterFamily::Threats) &&
            (parameter.family == ParameterFamily::Attack ||
             (parameter.family == ParameterFamily::Inline && parameter.semanticIndex == 7)))
            return true;

        if (selected(ParameterFamily::Endgame) &&
            (parameter.family == ParameterFamily::EndgameWeights ||
             (parameter.family == ParameterFamily::Inline &&
              (parameter.semanticIndex == 4 || parameter.semanticIndex == 5))))
            return true;

        if (selected(ParameterFamily::King) &&
            parameter.family == ParameterFamily::KingSafety)
            return true;

        if (selected(ParameterFamily::BaseScalars))
        {
            if (parameter.family == ParameterFamily::PieceValue) return true;
            if (parameter.family == ParameterFamily::Inline)
            {
                switch (parameter.semanticIndex)
                {
                case 0: case 1: case 2: case 3:
                    return true;
                default:
                    break;
                }
            }
        }

        return false;
    }

    static const std::vector<ParameterFamily>& Refine1Families()
    {
        static const std::vector<ParameterFamily> families = {
            ParameterFamily::PawnStructure,
            ParameterFamily::PassedPawnV2,
            ParameterFamily::PieceSquare,
            ParameterFamily::CenterPresence,
            ParameterFamily::KingSafety
        };
        return families;
    }

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
                const double sideToMoveResult = b->sideToMove ? 1.0 - res : res;
                TunerPosition position;
                position.board.reset(b);
                position.positionKey = fen;
                position.gameResult = sideToMoveResult;
                positions.push_back(std::move(position));
            }
        }
        return true;
    }

    static bool LoadScoreFile(const std::string& filepath,
                              std::unordered_map<std::string, int>& scores,
                              std::size_t& malformedRows,
                              const char* description)
    {
        malformedRows = 0;
        if (filepath.empty()) return true;
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Could not open " << description << " score file: " << filepath << "\n";
            return false;
        }
        std::string line;
        while (std::getline(file, line))
        {
            const auto tab = line.rfind('\t');
            if (tab == std::string::npos || tab == 0 || tab + 1 >= line.size())
            {
                ++malformedRows;
                continue;
            }
            try
            {
                std::size_t consumed = 0;
                const int score = std::stoi(line.substr(tab + 1), &consumed);
                if (consumed != line.size() - tab - 1) throw std::invalid_argument("suffix");
                scores[line.substr(0, tab)] = score;
            }
            catch (...)
            {
                ++malformedRows;
            }
        }
        return true;
    }

    static void PrepareTargets(std::vector<TunerPosition>& positions,
                               const std::unordered_map<std::string, int>& teacherScores,
                               const std::unordered_map<std::string, int>* anchorScores,
                               double scale)
    {
        for (auto& position : positions)
        {
            if (anchorScores)
            {
                const auto anchor = anchorScores->find(position.positionKey);
                if (anchor == anchorScores->end())
                    throw std::runtime_error("Anchor score missing for position: " + position.positionKey);
                position.baselineHowlScoreCp = anchor->second;
            }
            else
                position.baselineHowlScoreCp = EvaluationLogic::Evaluate(*position.board);
            position.baselineHowlProbability = TunerLossEvaluator::ScoreToProbability(
                position.baselineHowlScoreCp, scale);
            const auto teacher = teacherScores.find(position.positionKey);
            if (teacher != teacherScores.end())
            {
                position.teacherScoreCp = teacher->second;
                position.teacherProbability = TunerLossEvaluator::ScoreToProbability(
                    position.teacherScoreCp, scale);
                position.hasTeacherScore = true;
            }
        }
    }

    static HybridLossComponents ComputeLossComponents(
        const std::vector<TunerPosition>& dataset,
        const TunerEvaluationState& state,
        double scale,
        int numThreads = 8)
    {
        if (dataset.empty()) return {};
        Detail::InitializeKnightDistance();
        std::vector<HybridLossComponents> threadLosses(numThreads);
        std::vector<std::thread> workers;
        workers.reserve(numThreads);
        for (int t = 0; t < numThreads; ++t)
            workers.emplace_back([&, t]() {
                TunerEvaluationState localState = state;
                const std::size_t begin = (t * dataset.size()) / numThreads;
                const std::size_t end = ((t + 1) * dataset.size()) / numThreads;
                for (std::size_t i = begin; i < end; ++i)
                {
                    const int score = TunerEvaluator::Evaluate(*dataset[i].board, localState);
                    const double probability = TunerLossEvaluator::ScoreToProbability(score, scale);
                    threadLosses[t] += TunerLossEvaluator::PositionLoss(
                        probability, dataset[i].gameResult,
                        dataset[i].baselineHowlProbability,
                        dataset[i].hasTeacherScore, dataset[i].teacherProbability);
                }
            });
        for (auto& worker : workers) worker.join();
        HybridLossComponents total;
        for (const auto& loss : threadLosses) total += loss;
        total.Divide(static_cast<double>(dataset.size()));
        return total;
    }

    static double ComputeLoss(const std::vector<TunerPosition>& dataset,
                              const TunerEvaluationState& state,
                              double scale,
                              int numThreads = 8)
    {
        if (dataset.empty()) return 0.0;

        // Passed-pawn accessibility owns a lazily initialized shared distance
        // table. Initialize it before worker threads can enter the evaluator.
        Detail::InitializeKnightDistance();

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
                    double probability = TunerLossEvaluator::ScoreToProbability(stmScore, scale);
                    localSqError += TunerLossEvaluator::PositionLoss(
                        probability, dataset[i].gameResult,
                        dataset[i].baselineHowlProbability,
                        dataset[i].hasTeacherScore,
                        dataset[i].teacherProbability).combined;
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
        case ParameterFamily::PassedPawnV2: return 2;
        case ParameterFamily::PieceSquare: return 2;
        case ParameterFamily::CenterPresence: return 1;
        case ParameterFamily::CenterMove: return 1;
        case ParameterFamily::KingSafety: return 2;
        case ParameterFamily::KnightMobility: return 2;
        case ParameterFamily::BishopMobility: return 2;
        case ParameterFamily::RookMobility: return 2;
        case ParameterFamily::QueenMobility: return 2;
        case ParameterFamily::Attack: return 2;
        case ParameterFamily::RookFile: return 2;
        case ParameterFamily::KnightOutpost: return 2;
        case ParameterFamily::IsolatedPawn: return 2;
        case ParameterFamily::RookBehindPassedPawn: return 2;
        case ParameterFamily::EndgameWeights: return 2;
        default: return 0;
        }
    }

    static int GetParameterDelta(const TunerParameter& parameter,
                                 const std::vector<ParameterFamily>& selections)
    {
        if (parameter.family == ParameterFamily::Inline)
        {
            if (IsFamilyTunable(ParameterFamily::BaseScalars, selections) &&
                (parameter.semanticIndex <= 3 ||
                 (parameter.semanticIndex >= 6 && parameter.semanticIndex <= 8)))
                return 2;
            if (IsFamilyTunable(ParameterFamily::Pawns, selections) && parameter.semanticIndex == 9)
                return 2;
            if (IsFamilyTunable(ParameterFamily::Threats, selections) && parameter.semanticIndex == 10)
                return 2;
            if (IsFamilyTunable(ParameterFamily::Endgame, selections) &&
                (parameter.semanticIndex == 4 || parameter.semanticIndex == 5))
                return 2;
        }
        return GetFamilyDelta(parameter.family);
    }

    static int GetFamilyDeltaPass2(ParameterFamily family)
    {
        switch (family)
        {
        case ParameterFamily::PieceValue: return 10;
        case ParameterFamily::PawnStructure: return 4;
        case ParameterFamily::PassedPawnV2: return 4;
        case ParameterFamily::PieceSquare: return 4;
        case ParameterFamily::CenterPresence: return 2;
        case ParameterFamily::KingSafety: return 4;
        default: return 0; // CenterMove, mobility v2, Attack, Inline are frozen
        }
    }

    static int GetFamilyDeltaPass3(ParameterFamily family)
    {
        switch (family)
        {
        case ParameterFamily::PawnStructure: return 4;
        case ParameterFamily::PassedPawnV2: return 4;
        case ParameterFamily::PieceSquare: return 4;
        case ParameterFamily::CenterPresence: return 2;
        case ParameterFamily::KingSafety: return 4;
        default: return 0; // PieceValue, CenterMove, mobility v2, Attack, Inline are frozen
        }
    }

    static int GetFamilyDeltaRefine1(ParameterFamily family)
    {
        switch (family)
        {
        case ParameterFamily::PawnStructure: return 2;
        case ParameterFamily::PassedPawnV2: return 2;
        case ParameterFamily::PieceSquare: return 2;
        case ParameterFamily::CenterPresence: return 1;
        case ParameterFamily::KingSafety: return 2;
        default: return 0; // PieceValue, CenterMove, mobility v2, Attack, Inline are frozen
        }
    }

    static std::string FamilyToString(ParameterFamily family)
    {
        switch (family)
        {
        case ParameterFamily::PieceValue: return "PieceValue";
        case ParameterFamily::PawnStructure: return "PawnStructure";
        case ParameterFamily::PassedPawnV2: return "PassedPawnV2";
        case ParameterFamily::PieceSquare: return "PieceSquare";
        case ParameterFamily::CenterPresence: return "CenterPresence";
        case ParameterFamily::CenterMove: return "CenterMove";
        case ParameterFamily::KingSafety: return "KingSafety";
        case ParameterFamily::KnightMobility: return "KnightMobility";
        case ParameterFamily::BishopMobility: return "BishopMobility";
        case ParameterFamily::RookMobility: return "RookMobility";
        case ParameterFamily::QueenMobility: return "QueenMobility";
        case ParameterFamily::Attack: return "Attack";
        case ParameterFamily::Inline: return "Inline";
        case ParameterFamily::RookFile: return "RookFile";
        case ParameterFamily::KnightOutpost: return "KnightOutpost";
        case ParameterFamily::IsolatedPawn: return "IsolatedPawn";
        case ParameterFamily::RookBehindPassedPawn: return "RookBehindPassedPawn";
        case ParameterFamily::EndgameWeights: return "EndgameWeights";
        case ParameterFamily::BaseScalars: return "BaseScalars";
        case ParameterFamily::Pawns: return "Pawns";
        case ParameterFamily::Pieces: return "Pieces";
        case ParameterFamily::Threats: return "Threats";
        case ParameterFamily::Endgame: return "Endgame";
        case ParameterFamily::King: return "King";
        case ParameterFamily::PST: return "PST";
        default: return "Unknown";
        }
    }

    static bool FamilyFromString(const std::string& name, ParameterFamily& family)
    {
        const ParameterFamily families[] = {
            ParameterFamily::PieceValue, ParameterFamily::PawnStructure,
            ParameterFamily::PassedPawnV2, ParameterFamily::PieceSquare,
            ParameterFamily::CenterPresence, ParameterFamily::CenterMove,
            ParameterFamily::KingSafety, ParameterFamily::KnightMobility,
            ParameterFamily::BishopMobility, ParameterFamily::RookMobility,
            ParameterFamily::QueenMobility,
            ParameterFamily::Attack, ParameterFamily::Inline,
            ParameterFamily::RookFile, ParameterFamily::KnightOutpost,
            ParameterFamily::IsolatedPawn, ParameterFamily::RookBehindPassedPawn,
            ParameterFamily::EndgameWeights,
            ParameterFamily::BaseScalars, ParameterFamily::Pawns,
            ParameterFamily::Pieces, ParameterFamily::Threats,
            ParameterFamily::Endgame, ParameterFamily::King, ParameterFamily::PST
        };
        for (ParameterFamily candidate : families)
        {
            if (name == FamilyToString(candidate))
            {
                family = candidate;
                return true;
            }
        }
        return false;
    }

    static CoordinateDescentResult Tune(const std::vector<TunerPosition>& trainPositions,
                                        const std::vector<TunerPosition>& valPositions,
                                        TunerEvaluationState& state,
                                        const TunerRegistry& registry,
                                        const std::vector<ParameterFamily>& tunableFamilies,
                                        double scale = 554.17,
                                        int numThreads = 8)
    {
        CoordinateDescentResult result;
        result.maxWorkerThreads = numThreads;
        if (trainPositions.empty() || valPositions.empty() || tunableFamilies.empty())
        {
            return result;
        }

        std::vector<int> initialValues(registry.Size(), 0);
        for (std::size_t i = 0; i < registry.Size(); ++i)
        {
            const int* ptr = state.GetParameterPointer(registry[i].family, registry[i].semanticIndex);
            initialValues[i] = ptr ? *ptr : registry[i].currentValue;
        }

        result.baselineTrainComponents = ComputeLossComponents(trainPositions, state, scale, numThreads);
        result.baselineValComponents = ComputeLossComponents(valPositions, state, scale, numThreads);
        result.baselineTrainLoss = result.baselineTrainComponents.combined;
        result.baselineValLoss = result.baselineValComponents.combined;
        TunerThreadPoolEvaluator pool(trainPositions, scale, numThreads);
        auto startTime = std::chrono::high_resolution_clock::now();

        for (std::size_t p = 0; p < registry.Size(); ++p)
        {
            const auto& param = registry[p];
            if (IsParameterTunable(param, tunableFamilies) &&
                GetParameterDelta(param, tunableFamilies) > 0)
                result.parametersExamined++;
        }

        bool sweepChanged = false;
        do
        {
            sweepChanged = false;
            result.sweeps++;
            for (std::size_t p = 0; p < registry.Size(); ++p)
            {
                const auto& param = registry[p];
                if (!IsParameterTunable(param, tunableFamilies)) continue;

                const int delta = GetParameterDelta(param, tunableFamilies);
                if (delta <= 0) continue;

                int* targetPtr = state.GetParameterPointer(param.family, param.semanticIndex);
                if (!targetPtr) continue;

                const int currentValue = *targetPtr;
                int candidates[5] = {
                    currentValue - 2 * delta, currentValue - delta, currentValue,
                    currentValue + delta, currentValue + 2 * delta
                };
                if (param.minValue != param.maxValue)
                    for (int& candidate : candidates)
                        candidate = std::clamp(candidate, param.minValue, param.maxValue);
                double candidateLosses[5];
                for (int c = 0; c < 5; ++c)
                {
                    *targetPtr = candidates[c];
                    state.Derive();
                    candidateLosses[c] = pool.Evaluate(state);
                    result.optimizerSteps++;
                }

                int bestIndex = 2;
                double bestLoss = candidateLosses[2];
                for (int c = 0; c < 5; ++c)
                {
                    if (c == 2) continue;
                    if (candidateLosses[c] < bestLoss)
                    {
                        bestLoss = candidateLosses[c];
                        bestIndex = c;
                    }
                    else if (candidateLosses[c] == bestLoss)
                    {
                        const int candidateDistance = std::abs(candidates[c] - currentValue);
                        const int bestDistance = std::abs(candidates[bestIndex] - currentValue);
                        if (candidateDistance < bestDistance ||
                            (candidateDistance == bestDistance &&
                             (std::abs(candidates[c]) < std::abs(candidates[bestIndex]) ||
                              (std::abs(candidates[c]) == std::abs(candidates[bestIndex]) &&
                               candidates[c] < candidates[bestIndex]))))
                        {
                            bestIndex = c;
                        }
                    }
                }

                *targetPtr = candidates[bestIndex];
                state.Derive();
                sweepChanged = sweepChanged || (*targetPtr != currentValue);
            }
        } while (sweepChanged);
        result.terminationReason = "converged: complete sweep with no parameter changes";

        result.totalRuntimeSeconds = std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now() - startTime).count();
        result.finalTrainComponents = ComputeLossComponents(trainPositions, state, scale, numThreads);
        result.finalValComponents = ComputeLossComponents(valPositions, state, scale, numThreads);
        result.finalTrainLoss = result.finalTrainComponents.combined;
        result.finalValLoss = result.finalValComponents.combined;

        for (std::size_t i = 0; i < registry.Size(); ++i)
        {
            const int* ptr = state.GetParameterPointer(registry[i].family, registry[i].semanticIndex);
            const int finalValue = ptr ? *ptr : initialValues[i];
            if (finalValue != initialValues[i])
            {
                result.changedParameters.push_back({registry[i].name, registry[i].family,
                    registry[i].semanticIndex, initialValues[i], finalValue,
                    finalValue - initialValues[i]});
                result.countChangedByFamily[FamilyToString(registry[i].family)]++;
            }
        }
        result.parametersChanged = static_cast<int>(result.changedParameters.size());
        return result;
    }

    static CoordinateDescentResult RunFamilies(
        const std::string& trainPath,
        const std::string& valPath,
        const std::vector<ParameterFamily>& tunableFamilies,
        double scale = 554.17,
        int numThreads = 8,
        const std::string& teacherScoresPath = "",
        const std::string& anchorScoresPath = "")
    {
        CoordinateDescentResult result;
        std::vector<TunerPosition> trainPositions;
        std::vector<TunerPosition> valPositions;
        if (!LoadDataset(trainPath, trainPositions) || trainPositions.empty() ||
            !LoadDataset(valPath, valPositions) || valPositions.empty())
        {
            std::cerr << "Failed to load tuner datasets.\n";
            return result;
        }

        Option::Initialize();
        TunerRegistry registry = TunerRegistry::CreateRegistry();
        TunerEvaluationState state;
        state.LoadFromRegistry(registry);
        std::unordered_map<std::string, int> teacherScores;
        std::size_t malformedTeacherRows = 0;
        if (!LoadScoreFile(teacherScoresPath, teacherScores, malformedTeacherRows, "teacher"))
            return result;
        if (!teacherScoresPath.empty())
            std::cout << "Malformed teacher rows skipped: " << malformedTeacherRows << '\n';
        std::unordered_map<std::string, int> anchorScores;
        std::size_t malformedAnchorRows = 0;
        if (!LoadScoreFile(anchorScoresPath, anchorScores, malformedAnchorRows, "anchor"))
            return result;
        if (!anchorScoresPath.empty() && malformedAnchorRows != 0)
        {
            std::cerr << "Malformed anchor rows: " << malformedAnchorRows << '\n';
            return result;
        }
        const auto* anchors = anchorScoresPath.empty() ? nullptr : &anchorScores;
        try
        {
            PrepareTargets(trainPositions, teacherScores, anchors, scale);
            PrepareTargets(valPositions, teacherScores, anchors, scale);
        }
        catch (const std::exception& error)
        {
            std::cerr << error.what() << '\n';
            return result;
        }
        return Tune(trainPositions, valPositions, state, registry, tunableFamilies, scale, numThreads);
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
        result.countChangedByFamily["PassedPawnV2"] = 0;
        result.countChangedByFamily["PieceSquare"] = 0;
        result.countChangedByFamily["CenterPresence"] = 0;
        result.countChangedByFamily["KingSafety"] = 0;

        for (std::size_t p = 0; p < registry.Size(); ++p)
        {
            const auto& param = registry[p];

            int delta = IsFamilyTunable(param.family, Refine1Families())
                ? GetFamilyDeltaRefine1(param.family) : 0;
            if (delta <= 0)
            {
                // Frozen families: PieceValue, CenterMove, mobility v2, Attack, Inline
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
