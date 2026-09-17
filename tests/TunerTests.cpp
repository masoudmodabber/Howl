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
#include <unordered_map>
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
    Tuner::TunerPosition position;
    position.gameResult = board->sideToMove ? 1.0 - result : result;
    position.positionKey = fen;
    position.baselineHowlScoreCp = EvaluationLogic::Evaluate(*board);
    position.baselineHowlProbability = Tuner::TunerLossEvaluator::ScoreToProbability(
        position.baselineHowlScoreCp, 554.17);
    position.board = std::move(board);
    positions.push_back(std::move(position));
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
    return selected == 133 ? 0 : 1;
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
    if (tunerScore != productionScore)
        std::cerr << "Parity mismatch: production=" << productionScore << " tuner=" << tunerScore << '\n';
    const auto verifierResult = Tuner::TunerEvaluationVerifier::RunVerification();
    if (!verifierResult.allMatched)
    {
        std::cerr << "Multi-position parity mismatch on: " << verifierResult.firstMismatchFen
                  << " prod=" << verifierResult.productionScore
                  << " tuner=" << verifierResult.tunerScore << '\n';
        return 1;
    }
    return tunerScore == productionScore ? 0 : 1;
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
    int mgRanks[6];
    int egRanks[6];
    PassedPawnV2::DecodeRanks(Option::PassedPawnMiddleGameParameters, mgRanks);
    PassedPawnV2::DecodeRanks(Option::PassedPawnEndGameParameters, egRanks);
    for (int rank = 1; rank < 6; ++rank)
        if (mgRanks[rank] < mgRanks[rank - 1] || egRanks[rank] < egRanks[rank - 1]) return 1;

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
        Tuner::TunerPosition position;
        position.positionKey = "parallel-" + std::to_string(i);
        position.gameResult = 1.0;
        position.baselineHowlScoreCp = EvaluationLogic::Evaluate(*board);
        position.baselineHowlProbability = Tuner::TunerLossEvaluator::ScoreToProbability(
            position.baselineHowlScoreCp, 554.17);
        position.board = std::move(board);
        positions.push_back(std::move(position));
    }

    const double loss = Tuner::TunerCoordinateDescent::ComputeLoss(
        positions, state, 554.17, 8);
    return std::isfinite(loss) ? 0 : 1;
}

int TestHybridLossWeighting()
{
    const auto withoutTeacher = Tuner::TunerLossEvaluator::PositionLoss(
        0.8, 1.0, 0.6, false, 0.0);
    const auto withTeacher = Tuner::TunerLossEvaluator::PositionLoss(
        0.8, 1.0, 0.6, true, 0.7);
    const double resultLoss = 0.04;
    const double anchorLoss = 0.04;
    const double teacherLoss = 0.01;
    const double expectedWithout = (0.50 / 0.65) * resultLoss + (0.15 / 0.65) * anchorLoss;
    const double expectedWith = 0.50 * resultLoss + 0.35 * teacherLoss + 0.15 * anchorLoss;
    return std::abs(withoutTeacher.combined - expectedWithout) < 1e-12 &&
           std::abs(withTeacher.combined - expectedWith) < 1e-12 ? 0 : 1;
}

int TestExternalAnchorAndAttackBounds()
{
    const std::string fen = "8/8/8/8/8/8/4K3/6k1 w - - 0 1";
    auto positions = Dataset(fen, 0.5);
    std::unordered_map<std::string, int> anchors{{fen, 1000}};
    Tuner::TunerCoordinateDescent::PrepareTargets(positions, {}, &anchors, 554.17);
    if (positions[0].baselineHowlScoreCp != 1000 ||
        std::abs(positions[0].baselineHowlProbability -
                 Tuner::TunerLossEvaluator::ScoreToProbability(1000, 554.17)) > 1e-12)
        return 1;

    const Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
    int attackParameters = 0;
    for (const auto& parameter : registry.GetParameters())
        if (parameter.family == Tuner::ParameterFamily::Attack)
        {
            ++attackParameters;
            if (parameter.minValue != 0 || parameter.currentValue < 0) return 1;
        }
    return attackParameters == 60 ? 0 : 1;
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
    else if (test == "hybrid_loss_weighting") result = TestHybridLossWeighting();
    else if (test == "external_anchor_and_attack_bounds") result = TestExternalAnchorAndAttackBounds();
    CleanupEngine();
    if (result != 0) std::cerr << "Tuner test failed: " << test << '\n';
    return result;
}
