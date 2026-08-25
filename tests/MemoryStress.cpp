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
#include <thread>
#include <vector>

#ifndef HOWL_MEMORY_STRESS_RELEASE
#define HOWL_MEMORY_STRESS_RELEASE 0
#endif

#ifndef HOWL_SOURCE_DIR
#define HOWL_SOURCE_DIR "."
#endif

namespace
{
constexpr int RepeatCount = 3;
constexpr int MinimumScore = -200000;
constexpr int MaximumScore = 200000;
constexpr long long EvaluationTableBytes = 8LL * 1024 * 1024;
constexpr long long ExchangeTableBytes = 256LL * 1024;
constexpr long long ExchangeWithoutTableBytes = 64LL * 1024;
constexpr long long KnownFixedCacheBytes =
    EvaluationTableBytes + ExchangeTableBytes + ExchangeWithoutTableBytes;

struct StressCase
{
    const char* name;
    const char* purpose;
    const char* fen;
    int depth;
    int multiPV;
};

const StressCase StressCases[] = {
    {
        "Kiwipete branching",
        "high legal-move branching and complex ordering",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        7,
        1
    },
    {
        "King safety tactics",
        "king attacks, captures, and QSearch continuations",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        7,
        1
    },
    {
        "Checking QSearch line",
        "forced checking continuations beyond the nominal PVS horizon",
        "1r4k1/4nppp/8/4Pb2/8/1P5P/r1PR4/3R3K w - - 0 27",
        6,
        1
    },
    {
        "Promotion tactics",
        "promotion generation, captures, and tactical QSearch",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        7,
        1
    },
    {
        "Advanced pawns and checks",
        "check evasions, advanced pawns, promotions, and deep recursion",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        8,
        1
    },
    {
        "Kiwipete MultiPV 4",
        "production-style four-line root breadth with complex move ordering",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        6,
        4
    }
};

struct MemorySnapshot
{
    std::string stage;
    long long rssBytes = 0;
    long long hwmBytes = 0;
    std::size_t evalEntries = 0;
    std::size_t pawnEntries = 0;
    std::size_t exchangeEntries = 0;
    std::size_t exchangeWithoutEntries = 0;
};

struct StressResult
{
    int caseIndex = 0;
    long long elapsedNanoseconds = 0;
    long long moveCount = 0;
    std::string bestMove;
    int score = MinimumScore;
    std::string pv;
    std::vector<MemorySnapshot> snapshots;
};

class GeneratedMoves
{
public:
    GeneratedMoves(Board& board, int depth, int depthGone)
        : moves(MoveLogic::MoveGenerator(board, depth, depthGone))
    {
    }

    ~GeneratedMoves()
    {
        for (int index = 0; index < moves.count; index++)
        {
            delete moves.moves[index];
        }
    }

    GeneratedMoves(const GeneratedMoves&) = delete;
    GeneratedMoves& operator=(const GeneratedMoves&) = delete;

    MoveList moves;
};

long long ReadStatusBytes(const std::string& requestedKey)
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

MemorySnapshot CaptureMemory(const std::string& stage)
{
    return {
        stage,
        ReadStatusBytes("VmRSS:"),
        ReadStatusBytes("VmHWM:"),
        EvaluationLogic::EvalCacheSize(),
        EvaluationLogic::PawnEvalCacheSize(),
        MoveLogic::ExchangeCacheSize(),
        MoveLogic::ExchangeWithoutBeginPieceCacheSize()
    };
}

StressResult FixedDepthProductionRoot(Board& board, int depth, int multiPV);
void WriteSample(const std::filesystem::path& path, const StressResult& result);

void InitializeEngine(std::vector<MemorySnapshot>& snapshots, int hashMiB = 40)
{
    snapshots.push_back(CaptureMemory("process entry before table allocation"));
    Option::Initialize();
    snapshots.push_back(CaptureMemory("after Option initialization"));
    AttackPlaces::Initialize();
    snapshots.push_back(CaptureMemory("after attack-table initialization"));
    BoardInitializer::Initialize();
    snapshots.push_back(CaptureMemory("after board/Zobrist initialization"));
    PieceMoves::Initialize();
    snapshots.push_back(CaptureMemory("after move-table initialization"));
    MoveLogic::Initialize();
    KingSetup::Initialize();
    PassedPawnSetup::Initialize();
    std::ostringstream diagnostics;
    if (!HashMemoryBudget::ConfigureMiB(hashMiB, diagnostics))
    {
        throw std::runtime_error("Default Hash configuration failed: " + diagnostics.str());
    }
    snapshots.push_back(CaptureMemory("after Hash table configuration"));
    UCI::IsRelease = false;
    snapshots.push_back(CaptureMemory("after complete engine initialization"));
}

int RunHashChild(int hashMiB, const std::filesystem::path& outputPath)
{
    StressResult result;
    result.caseIndex = hashMiB;
    InitializeEngine(result.snapshots, hashMiB);
    UCI::IsRelease = true;
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"));
    result.snapshots.push_back(CaptureMemory("after loading representative position"));
    Search::moveCount = 0;
    const auto begin = std::chrono::steady_clock::now();
    StressResult search = FixedDepthProductionRoot(*board, 4, 1);
    const auto end = std::chrono::steady_clock::now();
    result.elapsedNanoseconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
    result.moveCount = Search::moveCount;
    result.bestMove = search.bestMove;
    result.score = search.score;
    result.pv = search.pv;
    result.snapshots.push_back(CaptureMemory("after representative search"));
    board.reset();
    result.snapshots.push_back(CaptureMemory("final process state"));
    WriteSample(outputPath, result);
    return 0;
}

bool IsLegalAfterMove(Board& board, int movingSide)
{
    return !BoardLogic::UnderAttack(
        board,
        board.pieces[movingSide * 8 + 6].front(),
        board.sideToMove);
}

int KthBestScore(std::vector<int> scores, int multiPV)
{
    std::sort(scores.begin(), scores.end(), std::greater<int>());
    return scores[std::min<int>(multiPV, scores.size()) - 1];
}

StressResult FixedDepthProductionRoot(Board& board, int depth, int multiPV)
{
    GeneratedMoves generated(board, -1, -1);
    Move move2{};
    Move move3{};
    Move move4{};
    const int movingSide = board.sideToMove ? 1 : 0;
    int alpha = MinimumScore;
    int kthBestValue = MinimumScore;
    bool firstLegalMove = true;
    int legalMoveCount = 0;
    std::vector<int> rootScores;
    StressResult result;

    for (int index = 0; index < generated.moves.count; index++)
    {
        Move& move = *generated.moves.moves[index];
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
        }
        else if (legalMoveCount < multiPV)
        {
            std::unique_ptr<MovePrintValue> searched(PVSSearch::PVS(
                true, MinimumScore, MaximumScore, depth - 1, move,
                move2, move3, move4, board, mateSearch, true, 1, false, false));
            value = -searched->value;
            if (value < -159800)
            {
                value++;
            }
            childPV = searched->printString;
        }
        else
        {
            std::unique_ptr<MovePrintValue> searched(PVSSearch::PVS(
                false, -kthBestValue - Option::nullWindowSize, -kthBestValue,
                depth - 1, move, move2, move3, move4, board, mateSearch,
                true, 1, false, true));
            value = -searched->value;
            childPV = searched->printString;
            if (value > kthBestValue || value > 159800 || value < -159800)
            {
                searched.reset(PVSSearch::PVS(
                    true, -MaximumScore, -kthBestValue, depth - 1, move,
                    move2, move3, move4, board, mateSearch, true, 1, false, false));
                value = -searched->value;
                if (value < -159800)
                {
                    value++;
                }
                childPV = searched->printString;
            }
        }
        move.value = value;
        GameLogic::UndoMove(board, move, missingInfo);

        const std::string rootMove =
            ChessStringManipulation::PVToString(move, 0, false, board);
        rootScores.push_back(value);
        kthBestValue = KthBestScore(rootScores, multiPV);
        if (firstLegalMove || value > alpha)
        {
            alpha = value;
            result.bestMove = rootMove;
            result.score = value;
            result.pv = rootMove + (childPV.empty() ? "" : " " + childPV);
        }
        firstLegalMove = false;
        legalMoveCount++;
    }

    if (firstLegalMove)
    {
        throw std::runtime_error("Memory stress root found no legal move");
    }
    return result;
}

void WriteSample(const std::filesystem::path& path, const StressResult& result)
{
    std::ofstream output(path, std::ios::trunc);
    if (!output)
    {
        throw std::runtime_error("Cannot write memory stress sample");
    }
    output << result.caseIndex << ' ' << result.elapsedNanoseconds << ' '
           << result.moveCount << ' ' << std::quoted(result.bestMove) << ' '
           << result.score << ' ' << std::quoted(result.pv) << '\n'
           << result.snapshots.size() << '\n';
    for (const MemorySnapshot& snapshot : result.snapshots)
    {
        output << std::quoted(snapshot.stage) << ' ' << snapshot.rssBytes << ' '
               << snapshot.hwmBytes << ' ' << snapshot.evalEntries << ' '
               << snapshot.pawnEntries << ' ' << snapshot.exchangeEntries << ' '
               << snapshot.exchangeWithoutEntries << '\n';
    }
}

StressResult ReadSample(const std::filesystem::path& path)
{
    std::ifstream input(path);
    StressResult result;
    input >> result.caseIndex >> result.elapsedNanoseconds >> result.moveCount
          >> std::quoted(result.bestMove) >> result.score >> std::quoted(result.pv);
    std::size_t count = 0;
    input >> count;
    for (std::size_t index = 0; index < count; index++)
    {
        MemorySnapshot snapshot;
        input >> std::quoted(snapshot.stage) >> snapshot.rssBytes >> snapshot.hwmBytes
              >> snapshot.evalEntries >> snapshot.pawnEntries
              >> snapshot.exchangeEntries >> snapshot.exchangeWithoutEntries;
        result.snapshots.push_back(snapshot);
    }
    if (!input)
    {
        throw std::runtime_error("Invalid memory stress sample");
    }
    return result;
}

std::string ShellQuote(const std::string& value)
{
    std::string quoted = "'";
    for (char character : value)
    {
        quoted += character == '\'' ? "'\\''" : std::string(1, character);
    }
    return quoted + "'";
}

double Mebibytes(long long bytes)
{
    return static_cast<double>(bytes) / (1024.0 * 1024.0);
}

double Milliseconds(long long nanoseconds)
{
    return static_cast<double>(nanoseconds) / 1000000.0;
}

long long FinalHWM(const StressResult& result)
{
    return result.snapshots.empty() ? 0 : result.snapshots.back().hwmBytes;
}

int RunChild(int caseIndex, const std::filesystem::path& outputPath)
{
    if (caseIndex < 0 || caseIndex >= static_cast<int>(std::size(StressCases)))
    {
        throw std::runtime_error("Invalid memory stress case index");
    }
    StressResult result;
    result.caseIndex = caseIndex;
    InitializeEngine(result.snapshots);
    const StressCase& stressCase = StressCases[caseIndex];
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(stressCase.fen));
    result.snapshots.push_back(CaptureMemory("after loading stress position"));
    result.snapshots.push_back(CaptureMemory("immediately before search"));
    Search::moveCount = 0;
    StressResult search;
    std::exception_ptr searchFailure;
    const auto begin = std::chrono::steady_clock::now();
    std::thread searchThread([&]()
    {
        try
        {
            search = FixedDepthProductionRoot(*board, stressCase.depth, stressCase.multiPV);
        }
        catch (...)
        {
            searchFailure = std::current_exception();
        }
    });
    searchThread.join();
    const auto end = std::chrono::steady_clock::now();
    if (searchFailure)
    {
        std::rethrow_exception(searchFailure);
    }
    result.elapsedNanoseconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
    result.moveCount = Search::moveCount;
    result.bestMove = search.bestMove;
    result.score = search.score;
    result.pv = search.pv;
    result.snapshots.push_back(CaptureMemory("immediately after search"));
    board.reset();
    result.snapshots.push_back(CaptureMemory("final process state"));
    WriteSample(outputPath, result);
    return 0;
}

void WriteReport(std::ostream& output,
                 const std::vector<std::vector<StressResult>>& allResults)
{
    output << "## Diagnostic recursive search stress\n\n"
           << "- Build: Release (`-O3 -DNDEBUG -std=gnu++17`)\n"
           << "- Diagnostic copies: enabled intentionally (`UCI::IsRelease=false`)\n"
           << "- Protocol: three fresh processes per case; one joined production-style search thread; no cache clears or resizing\n"
           << "- Known fixed cache allocation: " << std::fixed << std::setprecision(3)
           << Mebibytes(KnownFixedCacheBytes)
           << " MiB (8 MiB evaluation + 256 KiB Exchange + 64 KiB ExchangeWithoutBeginPiece)\n\n"
           << "| Case | Depth | MultiPV | Repeat | Elapsed ms | Search::moveCount | Best move | Score | Final HWM MiB | HWM minus fixed caches MiB |\n"
           << "|---|---:|---:|---:|---:|---:|---|---:|---:|---:|\n";

    for (std::size_t caseIndex = 0; caseIndex < allResults.size(); caseIndex++)
    {
        const StressCase& stressCase = StressCases[caseIndex];
        for (std::size_t repeat = 0; repeat < allResults[caseIndex].size(); repeat++)
        {
            const StressResult& result = allResults[caseIndex][repeat];
            output << "| " << stressCase.name << " | " << stressCase.depth
                   << " | " << stressCase.multiPV << " | " << repeat + 1
                   << " | " << Milliseconds(result.elapsedNanoseconds)
                   << " | " << result.moveCount << " | " << result.bestMove
                   << " | " << result.score << " | " << Mebibytes(FinalHWM(result))
                   << " | " << Mebibytes(FinalHWM(result) - KnownFixedCacheBytes)
                   << " |\n";
        }
    }

    output << "\n### Per-stage samples\n\n";
    for (std::size_t caseIndex = 0; caseIndex < allResults.size(); caseIndex++)
    {
        const StressResult& selected = allResults[caseIndex][0];
        output << "#### " << StressCases[caseIndex].name << " (repeat 1)\n\n"
               << StressCases[caseIndex].purpose << ".\n\n"
               << "| Stage | RSS MiB | HWM MiB | Eval entries | Pawn entries | Exchange entries | Exchange-without entries |\n"
               << "|---|---:|---:|---:|---:|---:|---:|\n";
        for (const MemorySnapshot& snapshot : selected.snapshots)
        {
            output << "| " << snapshot.stage << " | " << Mebibytes(snapshot.rssBytes)
                   << " | " << Mebibytes(snapshot.hwmBytes)
                   << " | " << snapshot.evalEntries << " | " << snapshot.pawnEntries
                   << " | " << snapshot.exchangeEntries
                   << " | " << snapshot.exchangeWithoutEntries << " |\n";
        }
        output << "\nPV: `" << selected.pv << "`\n\n";
    }
}

int RunParent(const std::filesystem::path& executable)
{
    std::vector<std::vector<StressResult>> allResults(std::size(StressCases));
    const auto unique = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    for (std::size_t caseIndex = 0; caseIndex < std::size(StressCases); caseIndex++)
    {
        for (int repeat = 0; repeat < RepeatCount; repeat++)
        {
            const std::filesystem::path samplePath =
                std::filesystem::temp_directory_path() /
                ("howl-memory-stress-" + std::to_string(unique) + "-" +
                 std::to_string(caseIndex) + "-" + std::to_string(repeat) + ".txt");
            const std::string command = ShellQuote(std::filesystem::absolute(executable).string()) +
                " --child " + std::to_string(caseIndex) + " " + ShellQuote(samplePath.string());
            if (std::system(command.c_str()) != 0)
            {
                std::filesystem::remove(samplePath);
                throw std::runtime_error("Memory stress child process failed");
            }
            allResults[caseIndex].push_back(ReadSample(samplePath));
            std::filesystem::remove(samplePath);
        }
    }

    WriteReport(std::cout, allResults);
    const std::filesystem::path resultPath =
        std::filesystem::path(HOWL_SOURCE_DIR) / "benchmarks" / "memory-results.md";
    std::ofstream results(resultPath, std::ios::app);
    if (!results)
    {
        throw std::runtime_error("Cannot append memory stress results");
    }
    results << "\n";
    WriteReport(results, allResults);
    return 0;
}

int RunHashValidation(const std::filesystem::path& executable)
{
    const int hashSizes[] = {8, 16, 40, 1024};
    const auto unique = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::vector<StressResult> results;
    for (int hashMiB : hashSizes)
    {
        const std::filesystem::path samplePath =
            std::filesystem::temp_directory_path() /
            ("howl-hash-memory-" + std::to_string(unique) + "-" +
             std::to_string(hashMiB) + ".txt");
        const std::string command = ShellQuote(std::filesystem::absolute(executable).string()) +
            " --hash-child " + std::to_string(hashMiB) + " " +
            ShellQuote(samplePath.string());
        if (std::system(command.c_str()) != 0)
        {
            std::filesystem::remove(samplePath);
            throw std::runtime_error("Hash memory validation child process failed");
        }
        results.push_back(ReadSample(samplePath));
        std::filesystem::remove(samplePath);
    }

    std::ostringstream report;
    report << "## UCI Hash total-process validation\n\n"
           << "Fresh Release process for each Hash value; Kiwipete fixed-depth 4 search.\n\n"
           << "| Hash MiB | Entry RSS MiB | After configuration RSS MiB | "
              "After search RSS MiB | Final RSS MiB | Final HWM MiB | Under limit |\n"
           << "|---:|---:|---:|---:|---:|---:|---|\n";
    bool allUnderLimit = true;
    for (const StressResult& result : results)
    {
        const long long limitBytes =
            static_cast<long long>(result.caseIndex) * HashMemoryBudget::Mebibyte;
        const MemorySnapshot& entry = result.snapshots.front();
        const MemorySnapshot& configured = result.snapshots[4];
        const MemorySnapshot& searched = result.snapshots[result.snapshots.size() - 2];
        const MemorySnapshot& final = result.snapshots.back();
        const bool underLimit = final.hwmBytes <= limitBytes;
        allUnderLimit = allUnderLimit && underLimit;
        report << "| " << result.caseIndex << " | " << std::fixed
               << std::setprecision(3) << Mebibytes(entry.rssBytes) << " | "
               << Mebibytes(configured.rssBytes) << " | "
               << Mebibytes(searched.rssBytes) << " | "
               << Mebibytes(final.rssBytes) << " | "
               << Mebibytes(final.hwmBytes) << " | "
               << (underLimit ? "yes" : "NO") << " |\n";
    }
    std::cout << report.str();
    const std::filesystem::path resultPath =
        std::filesystem::path(HOWL_SOURCE_DIR) / "benchmarks" / "memory-results.md";
    std::ofstream output(resultPath, std::ios::app);
    if (!output)
    {
        throw std::runtime_error("Cannot append Hash memory validation results");
    }
    output << "\n" << report.str();
    return allUnderLimit ? 0 : 1;
}
}

int main(int argc, char* argv[])
{
    try
    {
        if (!HOWL_MEMORY_STRESS_RELEASE)
        {
            throw std::runtime_error("Howl memory stress must be built in Release configuration");
        }
        if (argc == 4 && std::string(argv[1]) == "--child")
        {
            return RunChild(std::stoi(argv[2]), argv[3]);
        }
        if (argc == 4 && std::string(argv[1]) == "--hash-child")
        {
            return RunHashChild(std::stoi(argv[2]), argv[3]);
        }
        if (argc == 2 && std::string(argv[1]) == "--hash-validation")
        {
            return RunHashValidation(argv[0]);
        }
        if (argc != 1)
        {
            throw std::runtime_error(
                "Usage: howl_memory_stress [--hash-validation]");
        }
        return RunParent(argv[0]);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Memory stress failure: " << error.what() << '\n';
        return 1;
    }
}
