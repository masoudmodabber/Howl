#include "BoardInitializer.h"
#include "BoardLogic.h"
#include "BoardMaker.h"
#include "ChessStringManipulation.h"
#include "EvaluationLogic.h"
#include "GameLogic.h"
#include "HashMemoryBudget.h"
#include "KingSetup.h"
#include "MissingInfoAboutPrevStateFromMove.h"
#include "MoveLogic.h"
#include "Option.h"
#include "PVSSearch.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "Search.h"
#include "UCI.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/resource.h>
#endif

#ifndef HOWL_BENCHMARK_RELEASE
#define HOWL_BENCHMARK_RELEASE 0
#endif

#ifndef HOWL_SOURCE_DIR
#define HOWL_SOURCE_DIR "."
#endif

namespace
{
constexpr int ProcessCount = 5;
constexpr int MinimumScore = -200000;
constexpr int MaximumScore = 200000;

struct BenchmarkPosition
{
    const char* name;
    const char* fen;
    int depth;
};

const BenchmarkPosition Positions[] = {
    {
        "Quiet middlegame",
        "rnbq1rk1/ppp2pbp/3p1np1/4p3/4P3/3P1NP1/PPPN1PBP/R1BQ1RK1 b - - 0 7",
        4
    },
    {
        "Kiwipete",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        4
    },
    {
        "King safety",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        4
    },
    {
        "Endgame",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        5
    },
    {
        "Promotion tactic",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        4
    },
    {
        "Advanced pawns/check evasion",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        5
    }
};

struct RootSearchResult
{
    std::string bestMove;
    int score = MinimumScore;
    std::string pv;
};

struct PositionResult
{
    std::string name;
    int depth = 0;
    long long elapsedNanoseconds = 0;
    long long moveCount = 0;
    long long nps = 0;
    std::string bestMove;
    int rootScore = MinimumScore;
    std::string pv;
};

struct MemorySnapshot
{
    std::string stage;
    long long currentRSSBytes = 0;
    long long peakRSSBytes = 0;
    std::size_t evalEntries = 0;
    std::size_t pawnEntries = 0;
    std::size_t exchangeEntries = 0;
    std::size_t exchangeWithoutEntries = 0;
};

struct CorpusResult
{
    std::vector<PositionResult> positions;
    std::vector<MemorySnapshot> memorySnapshots;
    long long elapsedNanoseconds = 0;
    long long moveCount = 0;
    long long nps = 0;
};

class GeneratedMoves
{
public:
    GeneratedMoves(Board& board, int depth, int depthGone)
        : moveList(MoveLogic::MoveGenerator(board, depth, depthGone))
    {
    }

    ~GeneratedMoves()
    {
        for (int counter = 0; counter < moveList.count; counter++)
        {
            delete moveList.moves[counter];
            moveList.moves[counter] = nullptr;
        }
    }

    GeneratedMoves(const GeneratedMoves&) = delete;
    GeneratedMoves& operator=(const GeneratedMoves&) = delete;

    MoveList moveList;
};

void InitializeEngine(bool releaseMode)
{
    Option::Initialize();
    AttackPlaces::Initialize();
    BoardInitializer::Initialize();
    PieceMoves::Initialize();
    MoveLogic::Initialize();
    KingSetup::Initialize();
    PassedPawnSetup::Initialize();
    std::ostringstream diagnostics;
    if (!HashMemoryBudget::EnsureDefaultConfigured(diagnostics))
    {
        throw std::runtime_error("Default Hash configuration failed: " + diagnostics.str());
    }
    UCI::IsRelease = releaseMode;
}

bool IsLegalAfterMove(Board& board, int movingSide)
{
    return !BoardLogic::UnderAttack(
        board,
        board.pieces[movingSide * 8 + 6].front(),
        board.sideToMove);
}

std::string RootMoveToString(const Move& move, const Board& board)
{
    return ChessStringManipulation::PVToString(move, 0, false, board);
}

std::string JoinPV(const std::string& rootMove, const std::string& childPV)
{
    if (childPV.empty())
    {
        return rootMove;
    }
    return rootMove + " " + childPV;
}

RootSearchResult FixedDepthProductionRoot(Board& board, int depth)
{
    // Production creates this list once and carries its ordering through the root loop.
    GeneratedMoves generatedMoves(board, -1, -1);
    Move move2{};
    Move move3{};
    Move move4{};
    const int movingSide = board.sideToMove ? 1 : 0;
    int alpha = MinimumScore;
    const int beta = MaximumScore;
    int kthBestValue = MinimumScore;
    bool firstLegalMove = true;
    RootSearchResult result;

    for (int counter = 0; counter < generatedMoves.moveList.count; counter++)
    {
        Move& move = *generatedMoves.moveList.moves[counter];
        const int orderingValue = move.value;
        MissingInfoAboutPrevStateFromMove missingInfo(board);
        GameLogic::DoMove(board, move, move4, -1, -1);

        if (!IsLegalAfterMove(board, movingSide))
        {
            GameLogic::UndoMove(board, move, missingInfo);
            continue;
        }

        int value = MinimumScore;
        std::string childPV;
        const bool sameMove = MoveLogic::Same(move2, move3, move4, move);
        const bool mateSearch = orderingValue > 159800 || orderingValue < -159800;

        if (sameMove)
        {
            value = 0;
            move.value = 0;
        }
        else if (firstLegalMove)
        {
            std::unique_ptr<MovePrintValue> searched(PVSSearch::PVS(
                true,
                MinimumScore,
                MaximumScore,
                depth - 1,
                move,
                move2,
                move3,
                move4,
                board,
                mateSearch,
                true,
                1,
                false,
                false));
            value = -searched->value;
            if (value < -159800)
            {
                value++;
            }
            move.value = value;
            childPV = searched->printString;
        }
        else
        {
            std::unique_ptr<MovePrintValue> searched(PVSSearch::PVS(
                false,
                -kthBestValue - Option::nullWindowSize,
                -kthBestValue,
                depth - 1,
                move,
                move2,
                move3,
                move4,
                board,
                mateSearch,
                true,
                1,
                false,
                true));
            value = -searched->value;
            move.value = value;
            childPV = searched->printString;

            if (value > kthBestValue || value > 159800 || value < -159800)
            {
                searched.reset(PVSSearch::PVS(
                    true,
                    -beta,
                    -kthBestValue,
                    depth - 1,
                    move,
                    move2,
                    move3,
                    move4,
                    board,
                    mateSearch,
                    true,
                    1,
                    false,
                    false));
                value = -searched->value;
                if (value < -159800)
                {
                    value++;
                }
                move.value = value;
                childPV = searched->printString;
            }
        }

        GameLogic::UndoMove(board, move, missingInfo);
        const std::string rootMove = RootMoveToString(move, board);

        if (firstLegalMove || value > kthBestValue)
        {
            kthBestValue = value;
        }
        if (firstLegalMove || value > alpha)
        {
            alpha = value;
            result.bestMove = rootMove;
            result.score = value;
            result.pv = JoinPV(rootMove, childPV);
        }
        firstLegalMove = false;
    }

    if (firstLegalMove)
    {
        throw std::runtime_error("Fixed-depth benchmark root found no legal move");
    }

    return result;
}

long long CalculateNPS(long long moveCount, long long elapsedNanoseconds)
{
    if (elapsedNanoseconds <= 0)
    {
        return 0;
    }
    return static_cast<long long>(
        static_cast<long double>(moveCount) * 1000000000.0L /
        static_cast<long double>(elapsedNanoseconds));
}

long long ReadProcStatusBytes(const std::string& requestedKey)
{
#if defined(__linux__)
    std::ifstream status("/proc/self/status");
    std::string key;
    while (status >> key)
    {
        if (key == requestedKey)
        {
            long long kibibytes = 0;
            std::string unit;
            status >> kibibytes >> unit;
            return kibibytes * 1024;
        }
        std::string remainder;
        std::getline(status, remainder);
    }
#endif
    return 0;
}

long long ReadCurrentRSSBytes()
{
    return ReadProcStatusBytes("VmRSS:");
}

long long ReadPeakRSSBytes()
{
#if defined(__linux__)
    return ReadProcStatusBytes("VmHWM:");
#elif defined(__unix__) || defined(__APPLE__)
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
#if defined(__APPLE__)
        return usage.ru_maxrss;
#else
        return static_cast<long long>(usage.ru_maxrss) * 1024;
#endif
    }
#endif
    return 0;
}

MemorySnapshot CaptureMemory(const std::string& stage)
{
    MemorySnapshot snapshot;
    snapshot.stage = stage;
    snapshot.currentRSSBytes = ReadCurrentRSSBytes();
    snapshot.peakRSSBytes = ReadPeakRSSBytes();
    snapshot.evalEntries = EvaluationLogic::EvalCacheSize();
    snapshot.pawnEntries = EvaluationLogic::PawnEvalCacheSize();
    snapshot.exchangeEntries = MoveLogic::ExchangeCacheSize();
    snapshot.exchangeWithoutEntries = MoveLogic::ExchangeWithoutBeginPieceCacheSize();
    return snapshot;
}

CorpusResult RunCorpus(
    bool measure,
    const std::string& phase,
    std::vector<MemorySnapshot>& memorySnapshots)
{
    CorpusResult corpus;

    for (const BenchmarkPosition& position : Positions)
    {
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(position.fen));
        memorySnapshots.push_back(CaptureMemory(phase + " loaded: " + position.name));
        Search::moveCount = 0;
        const auto begin = std::chrono::steady_clock::now();
        RootSearchResult root = FixedDepthProductionRoot(*board, position.depth);
        const auto end = std::chrono::steady_clock::now();
        memorySnapshots.push_back(CaptureMemory(phase + " after search: " + position.name));

        if (!measure)
        {
            continue;
        }

        PositionResult measured;
        measured.name = position.name;
        measured.depth = position.depth;
        measured.elapsedNanoseconds =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
        measured.moveCount = Search::moveCount;
        measured.nps = CalculateNPS(measured.moveCount, measured.elapsedNanoseconds);
        measured.bestMove = root.bestMove;
        measured.rootScore = root.score;
        measured.pv = root.pv;
        corpus.elapsedNanoseconds += measured.elapsedNanoseconds;
        corpus.moveCount += measured.moveCount;
        corpus.positions.push_back(measured);
    }

    corpus.nps = CalculateNPS(corpus.moveCount, corpus.elapsedNanoseconds);
    return corpus;
}

void WriteSample(const std::filesystem::path& path, const CorpusResult& result)
{
    std::ofstream output(path, std::ios::trunc);
    if (!output)
    {
        throw std::runtime_error("Cannot write benchmark sample: " + path.string());
    }

    output << result.positions.size() << '\n';
    for (const PositionResult& position : result.positions)
    {
        output << std::quoted(position.name) << ' '
               << position.depth << ' '
               << position.elapsedNanoseconds << ' '
               << position.moveCount << ' '
               << position.nps << ' '
               << std::quoted(position.bestMove) << ' '
               << position.rootScore << ' '
               << std::quoted(position.pv) << '\n';
    }
    output << result.elapsedNanoseconds << ' '
           << result.moveCount << ' '
           << result.nps << '\n';
    output << result.memorySnapshots.size() << '\n';
    for (const MemorySnapshot& snapshot : result.memorySnapshots)
    {
        output << std::quoted(snapshot.stage) << ' '
               << snapshot.currentRSSBytes << ' '
               << snapshot.peakRSSBytes << ' '
               << snapshot.evalEntries << ' '
               << snapshot.pawnEntries << ' '
               << snapshot.exchangeEntries << ' '
               << snapshot.exchangeWithoutEntries << '\n';
    }
}

CorpusResult ReadSample(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input)
    {
        throw std::runtime_error("Cannot read benchmark sample: " + path.string());
    }

    std::size_t positionCount = 0;
    input >> positionCount;
    CorpusResult result;
    for (std::size_t counter = 0; counter < positionCount; counter++)
    {
        PositionResult position;
        input >> std::quoted(position.name)
              >> position.depth
              >> position.elapsedNanoseconds
              >> position.moveCount
              >> position.nps
              >> std::quoted(position.bestMove)
              >> position.rootScore
              >> std::quoted(position.pv);
        result.positions.push_back(position);
    }
    input >> result.elapsedNanoseconds >> result.moveCount >> result.nps;
    std::size_t snapshotCount = 0;
    input >> snapshotCount;
    for (std::size_t counter = 0; counter < snapshotCount; counter++)
    {
        MemorySnapshot snapshot;
        input >> std::quoted(snapshot.stage)
              >> snapshot.currentRSSBytes
              >> snapshot.peakRSSBytes
              >> snapshot.evalEntries
              >> snapshot.pawnEntries
              >> snapshot.exchangeEntries
              >> snapshot.exchangeWithoutEntries;
        result.memorySnapshots.push_back(snapshot);
    }
    if (!input)
    {
        throw std::runtime_error("Invalid benchmark sample: " + path.string());
    }
    return result;
}

std::string ShellQuote(const std::string& value)
{
#ifdef _WIN32
    std::string quoted = "\"";
    for (char character : value)
    {
        if (character == '\"')
        {
            quoted += "\\\"";
        }
        else
        {
            quoted += character;
        }
    }
    return quoted + "\"";
#else
    std::string quoted = "'";
    for (char character : value)
    {
        if (character == '\'')
        {
            quoted += "'\\''";
        }
        else
        {
            quoted += character;
        }
    }
    return quoted + "'";
#endif
}

std::string CompilerDescription()
{
#if defined(__GNUC__)
    return "g++ " + std::to_string(__GNUC__) + "." +
        std::to_string(__GNUC_MINOR__) + "." + std::to_string(__GNUC_PATCHLEVEL__);
#else
    return "unknown compiler";
#endif
}

double Milliseconds(long long nanoseconds)
{
    return static_cast<double>(nanoseconds) / 1000000.0;
}

double Mebibytes(long long bytes)
{
    return static_cast<double>(bytes) / (1024.0 * 1024.0);
}

void WriteMemorySnapshots(std::ostream& output, const CorpusResult& result)
{
    output << "\n| Memory stage | Current RSS MiB | Process HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |\n"
           << "|---|---:|---:|---:|---:|---:|---:|\n";
    for (const MemorySnapshot& snapshot : result.memorySnapshots)
    {
        output << "| " << snapshot.stage
               << " | " << Mebibytes(snapshot.currentRSSBytes)
               << " | " << Mebibytes(snapshot.peakRSSBytes)
               << " | " << snapshot.evalEntries
               << " | " << snapshot.pawnEntries
               << " | " << snapshot.exchangeEntries
               << " | " << snapshot.exchangeWithoutEntries
               << " |\n";
    }
}

void WriteResult(
    std::ostream& output,
    const std::string& title,
    const CorpusResult& selected,
    const std::vector<CorpusResult>& samples,
    int selectedProcess)
{
    output << "## " << title << "\n\n"
           << "- Compiler: " << CompilerDescription() << "\n"
           << "- Build type: Release\n"
           << "- Expected flags: `-O3 -DNDEBUG -std=gnu++17`\n"
           << "- Protocol: five independent processes; one complete warmup and one measured corpus per process\n"
           << "- Selected process: " << selectedProcess << " (median aggregate elapsed time)\n\n"
           << "| Position | Depth | Elapsed ms | Search::moveCount | NPS | Best move | Root score | PV |\n"
           << "|---|---:|---:|---:|---:|---|---:|---|\n";

    output << std::fixed << std::setprecision(3);
    for (const PositionResult& position : selected.positions)
    {
        output << "| " << position.name
               << " | " << position.depth
               << " | " << Milliseconds(position.elapsedNanoseconds)
               << " | " << position.moveCount
               << " | " << position.nps
               << " | " << position.bestMove
               << " | " << position.rootScore
               << " | " << position.pv
               << " |\n";
    }

    output << "\n| Aggregate elapsed ms | Aggregate Search::moveCount | Aggregate NPS |\n"
           << "|---:|---:|---:|\n"
           << "| " << Milliseconds(selected.elapsedNanoseconds)
           << " | " << selected.moveCount
           << " | " << selected.nps << " |\n\n"
           << "Aggregate elapsed times for all five processes: ";

    for (std::size_t counter = 0; counter < samples.size(); counter++)
    {
        if (counter != 0)
        {
            output << ", ";
        }
        output << "P" << counter + 1 << "="
               << Milliseconds(samples[counter].elapsedNanoseconds) << " ms";
    }
    output << "\n";
    WriteMemorySnapshots(output, selected);
}

std::string JoinTitle(int argc, char* argv[])
{
    std::string title;
    for (int counter = 1; counter < argc; counter++)
    {
        if (!title.empty())
        {
            title += ' ';
        }
        title += argv[counter];
    }
    std::replace(title.begin(), title.end(), '\n', ' ');
    std::replace(title.begin(), title.end(), '\r', ' ');
    return title;
}

int RunSample(const std::filesystem::path& outputPath, bool diagnosticCopies)
{
    InitializeEngine(!diagnosticCopies);
    std::vector<MemorySnapshot> memorySnapshots;
    memorySnapshots.reserve(30);
    memorySnapshots.push_back(CaptureMemory("after engine initialization"));
#if HOWL_EVAL_CACHE_STATS
    EvaluationLogic::ResetEvalCacheStats();
#endif
#if HOWL_EXCHANGE_CACHE_STATS
    MoveLogic::ResetExchangeCacheStats();
#endif
    RunCorpus(false, "warmup", memorySnapshots);
#if HOWL_EVAL_CACHE_STATS
    const EvaluationCacheStatistics warmEvaluation = EvaluationLogic::EvalCacheStats();
    EvaluationLogic::ResetEvalCacheStats();
#endif
#if HOWL_EXCHANGE_CACHE_STATS
    const ExchangeCacheStatistics warmExchange = MoveLogic::ExchangeCacheStats();
    const ExchangeCacheStatistics warmWithout =
        MoveLogic::ExchangeWithoutBeginPieceCacheStats();
    MoveLogic::ResetExchangeCacheStats();
#endif
    memorySnapshots.push_back(CaptureMemory("after complete warmup"));
    CorpusResult measured = RunCorpus(true, "measured", memorySnapshots);
#if HOWL_EVAL_CACHE_STATS
    const EvaluationCacheStatistics measuredEvaluation = EvaluationLogic::EvalCacheStats();
#endif
#if HOWL_EXCHANGE_CACHE_STATS
    const ExchangeCacheStatistics measuredExchange = MoveLogic::ExchangeCacheStats();
    const ExchangeCacheStatistics measuredWithout =
        MoveLogic::ExchangeWithoutBeginPieceCacheStats();
#endif
    memorySnapshots.push_back(CaptureMemory("final process state"));
    measured.memorySnapshots = std::move(memorySnapshots);
    WriteSample(outputPath, measured);
#if HOWL_EVAL_CACHE_STATS
    const auto printEvaluationCacheStatistics = [](
        const char* phase, const EvaluationCacheStatistics& statistics)
    {
        std::cout << "EVAL_CACHE_STATS " << phase << ' '
                  << statistics.probes << ' '
                  << statistics.hits << ' '
                  << statistics.misses << ' '
                  << statistics.stores << ' '
                  << statistics.replacements << ' '
                  << statistics.uniqueEntries << ' '
                  << EvaluationLogic::EvalCacheCapacityBytes() << ' '
                  << EvaluationLogic::EvalCacheEntryCapacity() << ' '
                  << EvaluationLogic::EvalCacheClusterCount() << '\n';
    };
    printEvaluationCacheStatistics("warmup", warmEvaluation);
    printEvaluationCacheStatistics("measured", measuredEvaluation);
#endif
#if HOWL_EXCHANGE_CACHE_STATS
    const auto printCacheStatistics = [](const char* phase, const char* cacheName,
                                         const ExchangeCacheStatistics& statistics)
    {
        std::cout << "CACHE_STATS " << phase << ' ' << cacheName << ' '
                  << statistics.probes << ' '
                  << statistics.hits << ' '
                  << statistics.misses << ' '
                  << statistics.stores << ' '
                  << statistics.replacements << ' '
                  << statistics.uniqueEntries << '\n';
    };
    printCacheStatistics("warmup", "Exchange", warmExchange);
    printCacheStatistics("warmup", "WithoutBegin", warmWithout);
    printCacheStatistics("measured", "Exchange", measuredExchange);
    printCacheStatistics("measured", "WithoutBegin", measuredWithout);
#endif
    return 0;
}

std::vector<CorpusResult> RunIndependentProcesses(
    const std::filesystem::path& executable,
    bool diagnosticCopies,
    const std::string& temporaryPrefix)
{
    std::vector<CorpusResult> samples;
    std::vector<std::filesystem::path> samplePaths;
    const auto uniqueValue = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    try
    {
        for (int process = 0; process < ProcessCount; process++)
        {
            const std::filesystem::path samplePath =
                std::filesystem::temp_directory_path() /
                (temporaryPrefix + "-" + std::to_string(uniqueValue) + "-" +
                 std::to_string(process + 1) + ".txt");
            samplePaths.push_back(samplePath);
            const std::string command = ShellQuote(std::filesystem::absolute(executable).string()) +
                " --sample " + ShellQuote(samplePath.string()) + " " +
                (diagnosticCopies ? "1" : "0");
            if (std::system(command.c_str()) != 0)
            {
                throw std::runtime_error(
                    "Benchmark process " + std::to_string(process + 1) + " failed");
            }
            samples.push_back(ReadSample(samplePath));
            std::filesystem::remove(samplePath);
        }
    }
    catch (...)
    {
        for (const std::filesystem::path& path : samplePaths)
        {
            std::error_code ignored;
            std::filesystem::remove(path, ignored);
        }
        throw;
    }

    return samples;
}

int MedianProcessIndex(const std::vector<CorpusResult>& samples)
{
    std::vector<int> processOrder(ProcessCount);
    for (int counter = 0; counter < ProcessCount; counter++)
    {
        processOrder[counter] = counter;
    }
    std::sort(processOrder.begin(), processOrder.end(), [&](int left, int right)
    {
        return samples[left].elapsedNanoseconds < samples[right].elapsedNanoseconds;
    });

    return processOrder[ProcessCount / 2];
}

long long FinalPeakRSS(const CorpusResult& result)
{
    if (result.memorySnapshots.empty())
    {
        return 0;
    }
    return result.memorySnapshots.back().peakRSSBytes;
}

void WriteDiagnosticCopyComparison(
    std::ostream& output,
    const std::vector<CorpusResult>& disabledSamples,
    const std::vector<CorpusResult>& enabledSamples,
    int disabledIndex,
    int enabledIndex)
{
    const CorpusResult& disabled = disabledSamples[disabledIndex];
    const CorpusResult& enabled = enabledSamples[enabledIndex];
    output << "## Production diagnostic-copy impact\n\n"
           << "Both modes use five independent Release processes with one warmup and one measured corpus per process.\n\n"
           << "| Diagnostic board copies | Selected process | Aggregate elapsed ms | Search::moveCount | NPS | Peak RSS MiB |\n"
           << "|---|---:|---:|---:|---:|---:|\n"
           << std::fixed << std::setprecision(3)
           << "| Disabled (`UCI::IsRelease=true`) | " << disabledIndex + 1
           << " | " << Milliseconds(disabled.elapsedNanoseconds)
           << " | " << disabled.moveCount
           << " | " << disabled.nps
           << " | " << Mebibytes(FinalPeakRSS(disabled)) << " |\n"
           << "| Enabled (`UCI::IsRelease=false`) | " << enabledIndex + 1
           << " | " << Milliseconds(enabled.elapsedNanoseconds)
           << " | " << enabled.moveCount
           << " | " << enabled.nps
           << " | " << Mebibytes(FinalPeakRSS(enabled)) << " |\n\n";

    output << "Disabled aggregate elapsed/HWM samples: ";
    for (std::size_t counter = 0; counter < disabledSamples.size(); counter++)
    {
        if (counter != 0) output << ", ";
        output << "P" << counter + 1 << "="
               << Milliseconds(disabledSamples[counter].elapsedNanoseconds) << " ms/"
               << Mebibytes(FinalPeakRSS(disabledSamples[counter])) << " MiB";
    }
    output << "\nEnabled aggregate elapsed/HWM samples: ";
    for (std::size_t counter = 0; counter < enabledSamples.size(); counter++)
    {
        if (counter != 0) output << ", ";
        output << "P" << counter + 1 << "="
               << Milliseconds(enabledSamples[counter].elapsedNanoseconds) << " ms/"
               << Mebibytes(FinalPeakRSS(enabledSamples[counter])) << " MiB";
    }
    output << "\n";
}

int RunDiagnosticCopyComparison(const std::filesystem::path& executable)
{
    std::vector<CorpusResult> disabled = RunIndependentProcesses(
        executable, false, "howl-memory-release");
    std::vector<CorpusResult> enabled = RunIndependentProcesses(
        executable, true, "howl-memory-diagnostic");
    const int disabledIndex = MedianProcessIndex(disabled);
    const int enabledIndex = MedianProcessIndex(enabled);
    WriteDiagnosticCopyComparison(std::cout, disabled, enabled, disabledIndex, enabledIndex);

    const std::filesystem::path resultsPath =
        std::filesystem::path(HOWL_SOURCE_DIR) / "benchmarks" / "memory-results.md";
    std::ofstream results(resultsPath, std::ios::app);
    if (!results)
    {
        throw std::runtime_error("Cannot append memory results: " + resultsPath.string());
    }
    results << "\n";
    WriteDiagnosticCopyComparison(results, disabled, enabled, disabledIndex, enabledIndex);
    return 0;
}

int RunBenchmark(const std::filesystem::path& executable, const std::string& title)
{
    std::vector<CorpusResult> samples = RunIndependentProcesses(
        executable, false, "howl-benchmark");
    const int selectedIndex = MedianProcessIndex(samples);
    const CorpusResult& selected = samples[selectedIndex];
    WriteResult(std::cout, title, selected, samples, selectedIndex + 1);

    const std::filesystem::path resultsPath =
        std::filesystem::path(HOWL_SOURCE_DIR) / "benchmarks" / "results.md";
    std::ofstream results(resultsPath, std::ios::app);
    if (!results)
    {
        throw std::runtime_error("Cannot append benchmark results: " + resultsPath.string());
    }
    results << "\n";
    WriteResult(results, title, selected, samples, selectedIndex + 1);
    return 0;
}
}

int main(int argc, char* argv[])
{
    try
    {
        if (!HOWL_BENCHMARK_RELEASE)
        {
            throw std::runtime_error("Howl benchmark must be built in Release configuration");
        }
        if (argc == 4 && std::string(argv[1]) == "--sample")
        {
            return RunSample(argv[2], std::string(argv[3]) == "1");
        }
        if (argc == 2 && std::string(argv[1]) == "--compare-diagnostic-copies")
        {
            return RunDiagnosticCopyComparison(argv[0]);
        }
        if (argc < 2)
        {
            throw std::runtime_error("Usage: howl_benchmark <short descriptive title>");
        }
        return RunBenchmark(argv[0], JoinTitle(argc, argv));
    }
    catch (const std::exception& error)
    {
        std::cerr << "Benchmark failure: " << error.what() << '\n';
        return 1;
    }
}
