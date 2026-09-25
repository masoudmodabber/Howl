#include "AttackPlaces.h"
#include "BoardInitializer.h"
#include "BoardMaker.h"
#include "HashMemoryBudget.h"
#include "KingSetup.h"
#include "MoveLogic.h"
#include "Option.h"
#include "PVSSearch.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "RepetitionHistory.h"
#include "Search.h"
#include "TranspositionTable.h"
#include "UCI.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
constexpr int64_t DefaultNodeBudget = 20000;
constexpr double DefaultLogisticK = 1.0;
constexpr int DefaultScoreClampCp = 2000;

enum class Split { Train, Validation, Test, All };

struct Options
{
    std::vector<std::string> corpora;
    Split split = Split::All;
    int64_t nodes = DefaultNodeBudget;
    double logisticK = DefaultLogisticK;
    int scoreClamp = DefaultScoreClampCp;
    std::size_t limit = 0;
    bool verbose = false;
    std::string observationsOut;
    std::string knightOutpostName;
    int knightOutpostValue = 0;
    std::vector<std::pair<std::string, int>> rookFileOverrides;
    std::vector<std::pair<std::string, int>> isolatedPawnOverrides;
    std::string samplePath;
    std::string resultsOut;
};

struct CorpusPosition
{
    std::string id;
    std::string fen;
    double whiteResult = 0.5;
    double weight = 1.0;
    std::string group;
};

class NullBuffer : public std::streambuf
{
protected:
    int overflow(int c) override { return traits_type::not_eof(c); }
};

uint64_t StableHash(const std::string& text)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    for (unsigned char c : text)
    {
        hash ^= c;
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

bool InSplit(const std::string& identity, Split split)
{
    if (split == Split::All) return true;
    const uint64_t bucket = StableHash(identity) % 100;
    if (split == Split::Train) return bucket < 70;
    if (split == Split::Validation) return bucket >= 70 && bucket < 85;
    return bucket >= 85;
}

void InitializeEngine()
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
        throw std::runtime_error("Default Hash configuration failed: " + diagnostics.str());
    Option::MultiPV = 1;
    UCI::IsRelease = true;
}

void CleanupEngine()
{
    AttackPlaces::Cleanup();
    BoardInitializer::Cleanup();
    PieceMoves::Cleanup();
    MoveLogic::Cleanup();
    KingSetup::Cleanup();
    PassedPawnSetup::Cleanup();
}

bool ParseSplit(const std::string& text, Split& split)
{
    if (text == "train") split = Split::Train;
    else if (text == "validation") split = Split::Validation;
    else if (text == "test") split = Split::Test;
    else if (text == "all") split = Split::All;
    else return false;
    return true;
}

Options ParseOptions(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];
        auto value = [&](const char* name) -> std::string {
            if (++i >= argc) throw std::runtime_error(std::string("Missing value for ") + name);
            return argv[i];
        };
        if (arg == "--corpus") options.corpora.push_back(value("--corpus"));
        else if (arg == "--sample") options.samplePath = value("--sample");
        else if (arg == "--results-out") options.resultsOut = value("--results-out");
        else if (arg == "--split")
        {
            const std::string text = value("--split");
            if (!ParseSplit(text, options.split)) throw std::runtime_error("Unknown split: " + text);
        }
        else if (arg == "--nodes") options.nodes = std::stoll(value("--nodes"));
        else if (arg == "--k") options.logisticK = std::stod(value("--k"));
        else if (arg == "--score-clamp") options.scoreClamp = std::stoi(value("--score-clamp"));
        else if (arg == "--limit") options.limit = std::stoull(value("--limit"));
        else if (arg == "--observations-out") options.observationsOut = value("--observations-out");
        else if (arg == "--set-knight-outpost")
        {
            options.knightOutpostName = value("--set-knight-outpost");
            options.knightOutpostValue = std::stoi(value("--set-knight-outpost"));
        }
        else if (arg == "--set-rook-file")
        {
            const std::string name = value("--set-rook-file");
            options.rookFileOverrides.emplace_back(name, std::stoi(value("--set-rook-file")));
        }
        else if (arg == "--set-isolated-pawn")
        {
            const std::string name = value("--set-isolated-pawn");
            options.isolatedPawnOverrides.emplace_back(name, std::stoi(value("--set-isolated-pawn")));
        }
        else if (arg == "--candidate-attack")
        {
            Option::UseExperimentalAttackModel = true;
        }
        else if (arg == "--verbose") options.verbose = true;
        else if (arg == "--help")
        {
            std::cout << "Usage: howl_search_tuner_objective [--corpus PATH | --sample PATH] "
                         "[--split train|validation|test|all] "
                         "[--nodes N] [--k K] [--score-clamp CP] [--limit N] "
                         "[--observations-out PATH] [--results-out PATH] "
                         "[--set-knight-outpost NAME VALUE] [--verbose]\n";
            std::exit(0);
        }
        else throw std::runtime_error("Unknown argument: " + arg);
    }
    if (options.corpora.empty()) options.corpora.push_back("tuner-train.tsv");
    if (options.nodes <= 0 || options.logisticK <= 0.0 || options.scoreClamp <= 0)
        throw std::runtime_error("--nodes, --k, and --score-clamp must be positive");
    return options;
}

void ApplyKnightOutpostOverride(const Options& options)
{
    if (options.knightOutpostName.empty()) return;
    if (options.knightOutpostName == "KnightOutpostMiddleGame")
        Option::KnightOutpostMiddleGame = options.knightOutpostValue;
    else if (options.knightOutpostName == "KnightOutpostEndGame")
        Option::KnightOutpostEndGame = options.knightOutpostValue;
    else if (options.knightOutpostName == "KnightSupportedOutpostMiddleGame")
        Option::KnightSupportedOutpostMiddleGame = options.knightOutpostValue;
    else if (options.knightOutpostName == "KnightSupportedOutpostEndGame")
        Option::KnightSupportedOutpostEndGame = options.knightOutpostValue;
    else
        throw std::runtime_error("Unsupported KnightOutpost parameter: " +
                                 options.knightOutpostName);
}

void ApplyRookFileOverride(const Options& options)
{
    for (const auto& overrideValue : options.rookFileOverrides)
    {
        const std::string& name = overrideValue.first;
        const int value = overrideValue.second;
        if (name == "RookOpenFileMiddleGame") Option::RookOpenFileMiddleGame = value;
        else if (name == "RookOpenFileEndGame") Option::RookOpenFileEndGame = value;
        else if (name == "RookSemiOpenFileMiddleGame") Option::RookSemiOpenFileMiddleGame = value;
        else if (name == "RookSemiOpenFileEndGame") Option::RookSemiOpenFileEndGame = value;
        else throw std::runtime_error("Unsupported RookFile parameter: " + name);
    }
}

void ApplyIsolatedPawnOverride(const Options& options)
{
    for (const auto& overrideValue : options.isolatedPawnOverrides)
    {
        const std::string& name = overrideValue.first;
        const int value = overrideValue.second;
        if (name == "IsolatedPawnMiddleGame") Option::IsolatedPawnMiddleGame = value;
        else if (name == "IsolatedPawnEndGame") Option::IsolatedPawnEndGame = value;
        else throw std::runtime_error("Unsupported IsolatedPawn parameter: " + name);
    }
}

std::vector<CorpusPosition> LoadCorpus(const Options& options, std::size_t& rejected)
{
    std::vector<CorpusPosition> positions;
    rejected = 0;
    if (!options.samplePath.empty())
    {
        std::ifstream input(options.samplePath);
        if (!input) throw std::runtime_error("Could not open sample: " + options.samplePath);
        std::string line; std::getline(input, line);
        while (std::getline(input, line))
        {
            std::vector<std::string> fields; std::size_t start = 0;
            for (std::size_t tab; (tab = line.find('\t', start)) != std::string::npos; start = tab + 1)
                fields.push_back(line.substr(start, tab - start));
            fields.push_back(line.substr(start));
            if (fields.size() != 5 && fields.size() != 6) { ++rejected; continue; }
            CorpusPosition p; p.id=fields[0];p.fen=fields[1];p.whiteResult=std::stod(fields[2]);
            p.group=fields[3];p.weight=std::stod(fields.back());positions.push_back(std::move(p));
        }
        return positions;
    }
    for (const std::string& path : options.corpora)
    {
        std::ifstream input(path);
        if (!input) throw std::runtime_error("Could not open corpus: " + path);
        std::string line;
        std::size_t lineNumber = 0;
        while (std::getline(input, line))
        {
            ++lineNumber;
            if (line.empty()) continue;
            const std::size_t tab = line.rfind('\t');
            if (tab == std::string::npos) { ++rejected; continue; }
            CorpusPosition position;
            position.fen = line.substr(0, tab);
            position.id = path + ":" + std::to_string(lineNumber);
            try { position.whiteResult = std::stod(line.substr(tab + 1)); }
            catch (...) { ++rejected; continue; }
            if ((position.whiteResult != 0.0 && position.whiteResult != 0.5 &&
                 position.whiteResult != 1.0) || !InSplit(position.fen, options.split))
            {
                if (position.whiteResult != 0.0 && position.whiteResult != 0.5 &&
                    position.whiteResult != 1.0) ++rejected;
                continue;
            }
            positions.push_back(std::move(position));
            if (options.limit && positions.size() >= options.limit) return positions;
        }
    }
    return positions;
}

int ParseSearchScore(const std::string& scoreText, int clamp, bool& clamped)
{
    std::istringstream input(scoreText);
    std::string kind;
    int value = 0;
    input >> kind >> value;
    if (!input) throw std::runtime_error("Search returned no parseable score: " + scoreText);
    if (kind == "mate")
    {
        clamped = true;
        return value < 0 ? -clamp : clamp;
    }
    if (kind != "cp") throw std::runtime_error("Unknown search score: " + scoreText);
    const int bounded = std::clamp(value, -clamp, clamp);
    clamped = bounded != value;
    return bounded;
}

double ExpectedResult(int score, double k)
{
    return 1.0 / (1.0 + std::pow(10.0, -k * static_cast<double>(score) / 400.0));
}
}

int main(int argc, char** argv)
{
    try
    {
        const Options options = ParseOptions(argc, argv);
        std::size_t rejected = 0;
        const std::vector<CorpusPosition> positions = LoadCorpus(options, rejected);
        InitializeEngine();
        ApplyKnightOutpostOverride(options);
        ApplyRookFileOverride(options);
        ApplyIsolatedPawnOverride(options);
        const auto started = std::chrono::steady_clock::now();
        int64_t totalNodes = 0;
        int64_t scoreSum = 0;
        std::size_t processed = 0;
        std::size_t clampedCount = 0;
        double squaredErrorSum = 0.0;
        double weightSum = 0.0;
        NullBuffer nullBuffer;
        std::ofstream observations;
        std::ofstream results;
        if (!options.resultsOut.empty())
        {
            results.open(options.resultsOut);
            if (!results) throw std::runtime_error("Could not open results output: " + options.resultsOut);
            results << "position_id\tsearched_score\tbest_move\tactual_result\tsquared_error\tweight\tnodes\n";
        }
        if (!options.observationsOut.empty())
        {
            observations.open(options.observationsOut);
            if (!observations) throw std::runtime_error(
                "Could not open observations output: " + options.observationsOut);
            observations << "fen\tsearched_score\tactual_result\n";
        }

        for (const CorpusPosition& position : positions)
        {
            try
            {
                std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(position.fen));
                if (!board) { ++rejected; continue; }
                TranspositionTable::Clear();
                TranspositionTable::ResetStats();
                PVSSearch::ResetHistory();
                PVSSearch::ResetKillers();
                RepetitionHistory::ResetWithRoot(board->ZobristHashCode);
                Search::searchNodeCount = 0;
                Search::moveCount = 0;
                Search::maxNodes = options.nodes;
                Search::maxDepth = -1;
                Search::strictNodeLimit = true;
                Search::finiteSearch = false;
                Search::isMoveTime = false;
                Search::allowedTime = 0.0;
                Search::stopRequested.store(false, std::memory_order_relaxed);
                Search::active.store(true, std::memory_order_relaxed);
                Search::startTime = std::chrono::high_resolution_clock::now();
                Move m1{}, m2{}, m3{}, m4{};
                std::streambuf* oldOutput = std::cout.rdbuf(&nullBuffer);
                Search::MainSearch(m1, m2, m3, m4, *board);
                std::cout.rdbuf(oldOutput);
                Search::strictNodeLimit = false;

                bool clamped = false;
                const int score = ParseSearchScore(Search::Score, options.scoreClamp, clamped);
                const double actual = board->sideToMove ? 1.0 - position.whiteResult : position.whiteResult;
                const double predicted = ExpectedResult(score, options.logisticK);
                const double error = predicted - actual;
                squaredErrorSum += position.weight * error * error;
                weightSum += position.weight;
                scoreSum += score;
                totalNodes += Search::searchNodeCount;
                clampedCount += clamped;
                ++processed;
                if (observations)
                    observations << position.fen << '\t' << score << '\t'
                                 << std::setprecision(17) << actual << '\n';
                if (results)
                    results << position.id << '\t' << score << '\t'
                            << (!Search::completedBestMove.empty()?Search::completedBestMove:Search::bestMove)
                            << '\t' << std::setprecision(17) << actual << '\t' << error*error << '\t'
                            << position.weight << '\t' << Search::searchNodeCount << '\n';
                if (options.verbose)
                    std::cout << std::setprecision(12) << "position=" << position.id
                              << " fen=\"" << position.fen << "\" actual=" << actual
                              << " score=" << score << " predicted=" << predicted
                              << " squared_error=" << error * error
                              << " bestmove=" << (!Search::completedBestMove.empty()
                                  ? Search::completedBestMove : Search::bestMove)
                              << " nodes=" << Search::searchNodeCount << '\n';
            }
            catch (const std::exception& error)
            {
                Search::strictNodeLimit = false;
                ++rejected;
                if (options.verbose) std::cerr << "rejected=" << position.id << " error=" << error.what() << '\n';
            }
        }

        const double mse = weightSum > 0.0 ? squaredErrorSum / weightSum : 0.0;
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        std::cout << std::fixed << std::setprecision(12)
                  << "positions_processed: " << processed << '\n'
                  << "positions_rejected: " << rejected << '\n'
                  << "node_budget: " << options.nodes << '\n'
                  << "total_nodes: " << totalNodes << '\n'
                  << "mean_searched_score: " << (processed ? static_cast<double>(scoreSum) / processed : 0.0) << '\n'
                  << "mse: " << mse << '\n'
                  << "rmse: " << std::sqrt(mse) << '\n'
                  << "mate_or_clamped_scores: " << clampedCount << '\n'
                  << "wall_time_seconds: " << seconds << '\n'
                  << "positions_per_second: " << (seconds > 0.0 ? processed / seconds : 0.0) << '\n'
                  << "split_identity: position FEN (corpus has no game identity)\n";
        CleanupEngine();
        return processed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
