#include "AttackPlaces.h"
#include "BoardInitializer.h"
#include "KingSetup.h"
#include "MoveLogic.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "tuner/TunerCoordinateDescent.h"
#include "tuner/TunerEvaluationVerifier.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
void InitializeEngine()
{
    Option::Initialize();
    AttackPlaces::Initialize();
    BoardInitializer::Initialize();
    PieceMoves::Initialize();
    MoveLogic::Initialize();
    KingSetup::Initialize();
    PassedPawnSetup::Initialize();
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

std::vector<Tuner::TunerPosition> Dataset(const std::string& fen, double result)
{
    std::vector<Tuner::TunerPosition> positions;
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    if (!board) throw std::runtime_error("Could not create tuner test board");
    positions.push_back({std::move(board), result});
    return positions;
}

Tuner::CoordinateDescentResult TuneOnePosition(const std::string& fen,
                                                Tuner::ParameterFamily family,
                                                Tuner::TunerEvaluationState& state,
                                                double result)
{
    Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
    state.LoadFromRegistry(registry);
    auto train = Dataset(fen, result);
    auto validation = Dataset(fen, result);
    return Tuner::TunerCoordinateDescent::Tune(
        train, validation, state, registry, {family}, 554.17, 1);
}

double ExpectedForState(const std::string& fen, const Tuner::TunerEvaluationState& state)
{
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    if (!board) throw std::runtime_error("Could not create tuner test board");
    const int score = Tuner::TunerEvaluator::Evaluate(*board, state);
    return Tuner::TunerLossEvaluator::CentipawnsToExpectedWhiteScore(score, 554.17);
}

int TestPieceValueSelection()
{
    Tuner::TunerEvaluationState state;
    Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
    state.LoadFromRegistry(registry);
    Tuner::TunerEvaluationState target = state;
    target.QueenValue += 25;
    target.Derive();
    constexpr const char* fen = "7k/8/8/8/8/8/Q7/K7 w - - 0 1";
    const double targetResult = ExpectedForState(fen, target);
    const auto result = TuneOnePosition(
        fen, Tuner::ParameterFamily::PieceValue, state, targetResult);
    if (result.parametersExamined != 5 || result.sweeps < 3 ||
        result.optimizerSteps != result.sweeps * 25 ||
        std::abs(state.QueenValue - Option::QueenValue) <= 10 ||
        result.terminationReason.empty())
        return 1;
    for (const auto& change : result.changedParameters)
        if (change.family != Tuner::ParameterFamily::PieceValue) return 1;
    return 0;
}

int TestOtherFamilyFreezesPieceValue()
{
    Tuner::TunerEvaluationState state;
    Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
    state.LoadFromRegistry(registry);
    Tuner::TunerEvaluationState target = state;
    target.DoubledPawnValue += 6;
    target.Derive();
    constexpr const char* fen = "7k/8/8/8/8/P7/P7/K7 w - - 0 1";
    const auto result = TuneOnePosition(
        fen, Tuner::ParameterFamily::PawnStructure, state, ExpectedForState(fen, target));
    if (state.PawnValue != Option::PawnValue || state.KnightValue != Option::KnightValue ||
        state.BishopValue != Option::BishopValue || state.RookValue != Option::RookValue ||
        state.QueenValue != Option::QueenValue)
        return 1;
    for (const auto& change : result.changedParameters)
        if (change.family == Tuner::ParameterFamily::PieceValue) return 1;
    return result.parametersExamined == 1 && !result.changedParameters.empty() &&
        result.optimizerSteps == result.sweeps * 5 ? 0 : 1;
}

int TestStopsAfterUnchangedSweep()
{
    Tuner::TunerEvaluationState state;
    Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
    state.LoadFromRegistry(registry);
    constexpr const char* fen = "7k/8/8/8/8/8/8/K7 w - - 0 1";
    const auto result = TuneOnePosition(
        fen, Tuner::ParameterFamily::PieceValue, state, ExpectedForState(fen, state));
    return result.sweeps == 1 && result.optimizerSteps == 25 &&
        result.parametersChanged == 0 && !result.terminationReason.empty() ? 0 : 1;
}

int TestRefine1SelectionUnchanged()
{
    const auto& families = Tuner::TunerCoordinateDescent::Refine1Families();
    if (families.size() != 5 ||
        Tuner::TunerCoordinateDescent::IsFamilyTunable(Tuner::ParameterFamily::PieceValue, families))
        return 1;

    Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
    int selected = 0;
    for (const auto& parameter : registry.GetParameters())
    {
        if (Tuner::TunerCoordinateDescent::IsFamilyTunable(parameter.family, families) &&
            Tuner::TunerCoordinateDescent::GetFamilyDeltaRefine1(parameter.family) > 0)
            selected++;
    }
    return selected == 1230 ? 0 : 1;
}

int TestParityRegression()
{
    constexpr const char* fen =
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
    Tuner::TunerEvaluationState state;
    state.LoadFromRegistry(registry);
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    if (!board) return 1;
    const int productionScore = EvaluationLogic::Evaluate(*board);
    const int tunerScore = Tuner::TunerEvaluator::Evaluate(*board, state);
    return productionScore == -215 && tunerScore == productionScore ? 0 : 1;
}

template <std::size_t N>
bool WithinRepresentationError(const int (&generated)[N],
                               const int (&production)[N],
                               int maximumError)
{
    for (std::size_t i = 0; i < N; ++i)
        if (std::abs(generated[i] - production[i]) > maximumError) return false;
    return true;
}

template <std::size_t N>
bool IsNondecreasing(const int (&values)[N])
{
    for (std::size_t i = 1; i < N; ++i)
        if (values[i] < values[i - 1]) return false;
    return true;
}

int TestMobilityV2Structure()
{
    static const int knightMg[9] = {-20, -12, -5, 0, 12, 25, 31, 38, 38};
    static const int knightEg[9] = {-18, -11, -5, -3, 7, 17, 22, 27, 27};
    static const int bishopMg[14] = {-30, -20, -12, -6, -2, 0, 3, 6, 10, 15, 20, 25, 29, 32};
    static const int bishopEg[14] = {-35, -24, -15, -8, -3, 0, 4, 8, 13, 19, 25, 30, 34, 37};
    static const int rookMg[15] = {-16, -11, -6, -1, 4, 9, 13, 17, 21, 24, 26, 27, 28, 29, 30};
    static const int rookEg[15] = {-25, -16, -7, 2, 11, 20, 29, 38, 47, 54, 59, 62, 64, 65, 66};
    static const int queenMg[28] = {-10, -8, -6, -3, -1, 1, 3, 5, 8, 10, 12, 15, 16, 17, 18, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20};
    static const int queenEg[28] = {-18, -13, -7, -2, 3, 8, 13, 19, 23, 27, 32, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35};

    if (!WithinRepresentationError(Option::KnightMoveCountValueMiddleGame, knightMg, 2) ||
        !WithinRepresentationError(Option::KnightMoveCountValueEndGame, knightEg, 3) ||
        !WithinRepresentationError(Option::BishopMoveCountValueMiddleGame, bishopMg, 2) ||
        !WithinRepresentationError(Option::BishopMoveCountValueEndGame, bishopEg, 2) ||
        !WithinRepresentationError(Option::RookMoveCountValueMiddleGame, rookMg, 1) ||
        !WithinRepresentationError(Option::RookMoveCountValueEndGame, rookEg, 2) ||
        !WithinRepresentationError(Option::QueenMoveCountValueMiddleGame, queenMg, 1) ||
        !WithinRepresentationError(Option::QueenMoveCountValueEndGame, queenEg, 2))
        return 1;

    if (!IsNondecreasing(Option::KnightMoveCountValueMiddleGame) ||
        !IsNondecreasing(Option::KnightMoveCountValueEndGame) ||
        !IsNondecreasing(Option::BishopMoveCountValueMiddleGame) ||
        !IsNondecreasing(Option::BishopMoveCountValueEndGame) ||
        !IsNondecreasing(Option::RookMoveCountValueMiddleGame) ||
        !IsNondecreasing(Option::RookMoveCountValueEndGame) ||
        !IsNondecreasing(Option::QueenMoveCountValueMiddleGame) ||
        !IsNondecreasing(Option::QueenMoveCountValueEndGame))
        return 1;

    if (Option::KnightMoveCountValueMiddleGame[8] != Option::KnightMoveCountValueMiddleGame[7] ||
        Option::KnightMoveCountValueEndGame[8] != Option::KnightMoveCountValueEndGame[7])
        return 1;
    for (int i = 16; i < 28; ++i)
        if (Option::QueenMoveCountValueMiddleGame[i] != Option::QueenMoveCountValueMiddleGame[15]) return 1;
    for (int i = 13; i < 28; ++i)
        if (Option::QueenMoveCountValueEndGame[i] != Option::QueenMoveCountValueEndGame[12]) return 1;
    for (int i = 0; i < 3; ++i)
        if (Option::PawnMoveCountValueMiddleGame[i] != 0 || Option::PawnMoveCountValueEndGame[i] != 0) return 1;
    for (int i = 0; i < 9; ++i)
        if (Option::KingMoveCountValueMiddleGame[i] != 0 || Option::KingMoveCountValueEndGame[i] != 0) return 1;

    Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
    const struct { Tuner::ParameterFamily family; int count; } groups[] = {
        {Tuner::ParameterFamily::KnightMobility, 8},
        {Tuner::ParameterFamily::BishopMobility, 10},
        {Tuner::ParameterFamily::RookMobility, 10},
        {Tuner::ParameterFamily::QueenMobility, 10}
    };
    for (const auto& group : groups)
    {
        int selected = 0;
        for (const auto& parameter : registry.GetParameters())
            if (Tuner::TunerCoordinateDescent::IsFamilyTunable(parameter.family, {group.family}))
            {
                if (parameter.family != group.family) return 1;
                selected++;
            }
        if (selected != group.count) return 1;
    }

    Tuner::TunerEvaluationState state;
    if (!state.LoadFromRegistry(registry)) return 1;
    state.KnightMobilityMiddleGameParameters[1] = -100;
    state.Derive();
    if (!IsNondecreasing(state.KnightMoveCountValueMiddleGame)) return 1;
    return 0;
}

int TestPassedPawnV2Structure()
{
    static const int productionMg[64] = {
        0,0,0,0,0,0,0,0, 5,8,10,12,12,10,8,5,
        8,10,12,15,15,12,10,8, 10,12,15,18,18,15,12,10,
        12,15,18,20,20,18,15,12, 15,18,20,22,22,20,18,15,
        18,20,22,25,25,22,20,18, 0,0,0,0,0,0,0,0
    };
    static const int productionEg[64] = {
        0,0,0,0,0,0,0,0, 15,15,15,15,15,15,15,15,
        25,25,25,25,25,25,25,25, 40,40,40,40,40,40,40,40,
        60,60,60,60,60,60,60,60, 95,95,95,95,95,95,95,95,
        150,150,150,150,150,150,150,150, 0,0,0,0,0,0,0,0
    };
    static const int expectedMgRanks[6] = {9, 11, 14, 16, 19, 21};
    static const int expectedEgRanks[6] = {15, 25, 40, 60, 95, 150};

    int mgRanks[6];
    int egRanks[6];
    PassedPawnV2::DecodeRanks(Option::PassedPawnMiddleGameParameters, mgRanks);
    PassedPawnV2::DecodeRanks(Option::PassedPawnEndGameParameters, egRanks);
    for (int rank = 0; rank < 6; ++rank)
        if (mgRanks[rank] != expectedMgRanks[rank] || egRanks[rank] != expectedEgRanks[rank])
            return 1;

    int maximumMgError = 0;
    int totalMgError = 0;
    for (int square = 0; square < 64; ++square)
    {
        const int mgError = std::abs(Option::WhitePassedPawnValueMiddleGam[square] - productionMg[square]);
        maximumMgError = std::max(maximumMgError, mgError);
        totalMgError += mgError;
        if (Option::WhitePassedPawnValueEndGame[square] != productionEg[square]) return 1;
    }
    if (maximumMgError > 4 || static_cast<double>(totalMgError) / 48.0 > 2.42) return 1;

    for (int rank = 0; rank < 8; ++rank)
        for (int file = 0; file < 4; ++file)
        {
            const int left = rank * 8 + file;
            const int right = rank * 8 + 7 - file;
            if (Option::WhitePassedPawnValueMiddleGam[left] != Option::WhitePassedPawnValueMiddleGam[right] ||
                Option::WhitePassedPawnValueEndGame[left] != Option::WhitePassedPawnValueEndGame[right])
                return 1;
        }

    for (int file = 0; file < 8; ++file)
    {
        if (Option::WhitePassedPawnValueMiddleGam[file] != 0 ||
            Option::WhitePassedPawnValueEndGame[file] != 0 ||
            Option::WhitePassedPawnValueMiddleGam[56 + file] != 0 ||
            Option::WhitePassedPawnValueEndGame[56 + file] != 0)
            return 1;
        for (int rank = 2; rank <= 6; ++rank)
            if (Option::WhitePassedPawnValueMiddleGam[rank * 8 + file] <
                    Option::WhitePassedPawnValueMiddleGam[(rank - 1) * 8 + file] ||
                Option::WhitePassedPawnValueEndGame[rank * 8 + file] <
                    Option::WhitePassedPawnValueEndGame[(rank - 1) * 8 + file])
                return 1;
    }

    Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
    int selected = 0;
    for (const auto& parameter : registry.GetParameters())
        if (Tuner::TunerCoordinateDescent::IsFamilyTunable(
                parameter.family, {Tuner::ParameterFamily::PassedPawnV2}))
        {
            if (parameter.family != Tuner::ParameterFamily::PassedPawnV2) return 1;
            selected++;
        }
    if (selected != 13) return 1;

    Tuner::TunerEvaluationState state;
    if (!state.LoadFromRegistry(registry)) return 1;
    state.PassedPawnMiddleGameParameters[1] = -100;
    state.PassedPawnEndGameParameters[1] = -100;
    state.Derive();
    for (int file = 0; file < 8; ++file)
        for (int rank = 2; rank <= 6; ++rank)
            if (state.WhitePassedPawnValueMiddleGam[rank * 8 + file] <
                    state.WhitePassedPawnValueMiddleGam[(rank - 1) * 8 + file] ||
                state.WhitePassedPawnValueEndGame[rank * 8 + file] <
                    state.WhitePassedPawnValueEndGame[(rank - 1) * 8 + file])
                return 1;
    return 0;
}

int TestPassedPawnV2ParallelStartup()
{
    Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
    Tuner::TunerEvaluationState state;
    if (!state.LoadFromRegistry(registry)) return 1;

    std::vector<Tuner::TunerPosition> positions;
    for (int i = 0; i < 64; ++i)
    {
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(
            "7k/8/8/3P4/8/5N2/8/K7 w - - 0 1"));
        if (!board) return 1;
        positions.push_back({std::move(board), 1.0});
    }

    const double loss = Tuner::TunerCoordinateDescent::ComputeLoss(
        positions, state, 554.17, 8);
    return std::isfinite(loss) ? 0 : 1;
}
}

int main(int argc, char* argv[])
{
    if (argc != 2) return 2;
    InitializeEngine();
    int result = 1;
    const std::string test = argv[1];
    if (test == "piece_value_only") result = TestPieceValueSelection();
    else if (test == "other_family_freezes_piece_value") result = TestOtherFamilyFreezesPieceValue();
    else if (test == "refine1_unchanged") result = TestRefine1SelectionUnchanged();
    else if (test == "parity_regression") result = TestParityRegression();
    else if (test == "stops_after_unchanged_sweep") result = TestStopsAfterUnchangedSweep();
    else if (test == "mobility_v2_structure") result = TestMobilityV2Structure();
    else if (test == "passed_pawn_v2_structure") result = TestPassedPawnV2Structure();
    else if (test == "passed_pawn_v2_parallel_startup") result = TestPassedPawnV2ParallelStartup();
    CleanupEngine();
    if (result != 0) std::cerr << "Tuner test failed: " << test << '\n';
    return result;
}
