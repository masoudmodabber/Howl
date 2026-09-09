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
    return selected == 1345 ? 0 : 1;
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
    return productionScore == -224 && tunerScore == productionScore ? 0 : 1;
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
    CleanupEngine();
    if (result != 0) std::cerr << "Tuner test failed: " << test << '\n';
    return result;
}
