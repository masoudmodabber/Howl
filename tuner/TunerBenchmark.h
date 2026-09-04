#ifndef HOWL_TUNER_BENCHMARK_H
#define HOWL_TUNER_BENCHMARK_H

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "Board.h"
#include "BoardMaker.h"
#include "Option.h"
#include "tuner/TunerEvaluationState.h"
#include "tuner/TunerEvaluator.h"
#include "tuner/TunerParameter.h"

namespace Tuner
{

struct MultiThreadBenchmarkResult
{
    std::size_t loadedPositions = 0;
    std::size_t totalEvaluations = 0;
    int numPasses = 0;

    double timeMs1Thread = 0.0;
    double evalsPerSec1Thread = 0.0;
    double avgMicrosec1Thread = 0.0;
    long long checksum1Thread = 0;

    double timeMs8Threads = 0.0;
    double evalsPerSec8Threads = 0.0;
    double avgMicrosec8Threads = 0.0;
    long long checksum8Threads = 0;

    double speedupFactor = 0.0;
    bool checksumsMatch = false;
};

class TunerBenchmark
{
public:
    static MultiThreadBenchmarkResult RunMultiThread(const std::string& trainPath = "tuner-train.tsv", int numPasses = 10)
    {
        MultiThreadBenchmarkResult res;
        res.numPasses = numPasses;

        std::ifstream file(trainPath);
        if (!file.is_open())
        {
            std::cerr << "Could not open " << trainPath << "\n";
            return res;
        }

        std::vector<std::unique_ptr<Board>> boards;
        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty()) continue;
            auto tabPos = line.rfind('\t');
            std::string fen = (tabPos != std::string::npos) ? line.substr(0, tabPos) : line;

            Board* b = BoardMaker::MakeInitialBoard(fen);
            if (b)
            {
                boards.emplace_back(b);
            }
        }

        res.loadedPositions = boards.size();
        if (boards.empty())
        {
            return res;
        }

        res.totalEvaluations = boards.size() * numPasses;

        Option::Initialize();
        TunerRegistry registry = TunerRegistry::CreateRegistry();
        TunerEvaluationState state;
        state.LoadFromRegistry(registry);

        // 1-thread evaluation benchmark
        {
            long long checksum = 0;
            auto start = std::chrono::high_resolution_clock::now();
            for (int pass = 0; pass < numPasses; ++pass)
            {
                for (std::size_t i = 0; i < boards.size(); ++i)
                {
                    checksum += TunerEvaluator::Evaluate(*boards[i], state);
                }
            }
            auto end = std::chrono::high_resolution_clock::now();

            res.timeMs1Thread = std::chrono::duration<double, std::milli>(end - start).count();
            res.evalsPerSec1Thread = (res.totalEvaluations / (res.timeMs1Thread / 1000.0));
            res.avgMicrosec1Thread = (res.timeMs1Thread * 1000.0) / res.totalEvaluations;
            res.checksum1Thread = checksum;
        }

        // 8-thread evaluation benchmark
        {
            const int numThreads = 8;
            std::vector<long long> threadChecksums(numThreads, 0);
            std::vector<std::thread> workers;
            workers.reserve(numThreads);

            auto start = std::chrono::high_resolution_clock::now();
            for (int t = 0; t < numThreads; ++t)
            {
                workers.emplace_back([&, t]() {
                    TunerEvaluationState threadState = state;
                    std::size_t startIdx = (t * boards.size()) / numThreads;
                    std::size_t endIdx = ((t + 1) * boards.size()) / numThreads;
                    long long localSum = 0;

                    for (int pass = 0; pass < numPasses; ++pass)
                    {
                        for (std::size_t i = startIdx; i < endIdx; ++i)
                        {
                            localSum += TunerEvaluator::Evaluate(*boards[i], threadState);
                        }
                    }
                    threadChecksums[t] = localSum;
                });
            }

            for (auto& w : workers)
            {
                w.join();
            }
            auto end = std::chrono::high_resolution_clock::now();

            long long totalSum = 0;
            for (long long c : threadChecksums)
            {
                totalSum += c;
            }

            res.timeMs8Threads = std::chrono::duration<double, std::milli>(end - start).count();
            res.evalsPerSec8Threads = (res.totalEvaluations / (res.timeMs8Threads / 1000.0));
            res.avgMicrosec8Threads = (res.timeMs8Threads * 1000.0) / res.totalEvaluations;
            res.checksum8Threads = totalSum;
        }

        res.speedupFactor = res.evalsPerSec8Threads / res.evalsPerSec1Thread;
        res.checksumsMatch = (res.checksum1Thread == res.checksum8Threads);

        return res;
    }
};

} // namespace Tuner

#endif // HOWL_TUNER_BENCHMARK_H
