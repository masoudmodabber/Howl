#include "BoardInitializer.h"
#include "BoardMaker.h"
#include "NNUEEvaluator.h"
#include "Option.h"
#include "GameLogic.h"
#include "ChessStringManipulation.h"
#include "PieceMoves.h"
#include "MoveLogic.h"
#include "AttackPlaces.h"
#include "KingSetup.h"
#include "PassedPawnSetup.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <cstdio>

namespace
{
void Require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

void RequireSame(const NNUEState& a, const NNUEState& b)
{
    Require(a.whiteKingSquare == b.whiteKingSquare && a.blackKingSquare == b.blackKingSquare, "anchor mismatch");
    for (int i = 0; i < NNUEEvaluator::HiddenSize; ++i)
    {
        if (std::abs(a.whiteAccumulator[i] - b.whiteAccumulator[i]) > 1e-6f) { std::cerr << "white unit " << i << " " << a.whiteAccumulator[i] << " " << b.whiteAccumulator[i] << "\n"; throw std::runtime_error("white accumulator mismatch"); }
        if (std::abs(a.blackAccumulator[i] - b.blackAccumulator[i]) > 1e-6f) { std::cerr << "black unit " << i << " " << a.blackAccumulator[i] << " " << b.blackAccumulator[i] << "\n"; throw std::runtime_error("black accumulator mismatch"); }
    }
}

std::unique_ptr<Board> Make(const char* fen)
{
    return std::unique_ptr<Board>(BoardMaker::MakeInitialBoard(fen));
}

void TestFeatures()
{
    Require(NNUEEvaluator::FeatureIndex(0, 0, 0) == 0, "first feature");
    Require(NNUEEvaluator::FeatureIndex(63, 9, 63) == 40959, "last feature");
    Require(NNUEEvaluator::ColoredPieceClass(1) == 0, "white pawn class");
    Require(NNUEEvaluator::ColoredPieceClass(5) == 4, "white queen class");
    Require(NNUEEvaluator::ColoredPieceClass(9) == 5, "black pawn class");
    Require(NNUEEvaluator::ColoredPieceClass(13) == 9, "black queen class");
    Require(NNUEEvaluator::ColoredPieceClass(6) == -1, "white king excluded");
    Require(NNUEEvaluator::ColoredPieceClass(14) == -1, "black king excluded");
    Require(NNUEEvaluator::FeatureIndexForPerspective(4, true, 1, 12) ==
            NNUEEvaluator::FeatureIndexForPerspective(59, false, 9, 51),
            "perspective mirror feature");
}

void TestPosition(const char* fen)
{
    auto board = Make(fen);
    NNUEEvaluator::Rebuild(*board);
    NNUEState first = board->nnueState;
    NNUEEvaluator::Rebuild(*board);
    RequireSame(first, board->nnueState);
    std::unique_ptr<Board> copy(board->MakeCopy());
    RequireSame(board->nnueState, copy->nnueState);
    Require(board->nnueState.whiteKingSquare == board->pieces[6].front(), "white anchor");
    Require(board->nnueState.blackKingSquare == board->pieces[14].front(), "black anchor");
}

void TestIncrementalMove(const char* fen, const char* uci)
{
    std::cerr << "testing " << uci << "\n";
    auto board = Make(fen);
    NNUEEvaluator::Rebuild(*board);
    NNUEState original = board->nnueState;
    Move* move = ChessStringManipulation::ConvertTextToMove(uci, *board);
    Require(move != nullptr, "move conversion failed");
    MissingInfoAboutPrevStateFromMove info(*board, *move);
    GameLogic::DoMove(*board, *move, info);
    NNUEState incremental = board->nnueState;
    NNUEEvaluator::Rebuild(*board);
    RequireSame(incremental, board->nnueState);
    GameLogic::UndoMove(*board, *move, info);
    RequireSame(original, board->nnueState);
    delete move;
}

void TestIncrementalSequence()
{
    auto board = Make("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    NNUEEvaluator::Rebuild(*board);
    NNUEState original = board->nnueState;
    const char* sequence[] = {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6", "b5a4", "g8f6", "e1g1"};
    std::vector<Move*> moves;
    std::vector<MissingInfoAboutPrevStateFromMove> infos;
    for (const char* uci : sequence)
    {
        Move* move = ChessStringManipulation::ConvertTextToMove(uci, *board);
        Require(move != nullptr, "sequence move conversion failed");
        moves.push_back(move);
        infos.emplace_back(*board, *move);
        GameLogic::DoMove(*board, *move, infos.back());
        NNUEState incremental = board->nnueState;
        NNUEEvaluator::Rebuild(*board);
        RequireSame(incremental, board->nnueState);
    }
    for (int i = static_cast<int>(moves.size()) - 1; i >= 0; --i)
    {
        GameLogic::UndoMove(*board, *moves[i], infos[i]);
        delete moves[i];
    }
    RequireSame(original, board->nnueState);
}

void TestForwardAndWeights()
{
    auto white = Make("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    auto black = Make("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    NNUEEvaluator::Rebuild(*white);
    NNUEEvaluator::Rebuild(*black);
    const float whiteValue = NNUEEvaluator::Evaluate(*white);
    const float blackValue = NNUEEvaluator::Evaluate(*black);
    Require(std::isfinite(whiteValue) && std::isfinite(blackValue), "non-finite forward output");
    Require(whiteValue != blackValue, "side-to-move ordering not applied");
    const std::string path = "/tmp/howl_nnue_prototype.weights";
    NNUEEvaluator::SaveWeights(path);
    const float before = NNUEEvaluator::Evaluate(*white);
    NNUEEvaluator::LoadWeights(path);
    Require(NNUEEvaluator::Evaluate(*white) == before, "save/load output mismatch");
    std::remove(path.c_str());
}
}

int main(int argc, char** argv)
{
    try
    {
        Option::Initialize();
        AttackPlaces::Initialize();
        BoardInitializer::Initialize();
        PieceMoves::Initialize();
        MoveLogic::Initialize();
        KingSetup::Initialize();
        PassedPawnSetup::Initialize();
        if (argc == 2)
        {
            auto board = Make("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            NNUEEvaluator::LoadWeights(argv[1]);
            NNUEEvaluator::Rebuild(*board);
            std::cout << "NNUE loaded output " << NNUEEvaluator::Evaluate(*board) << '\n';
            BoardInitializer::Cleanup();
            return 0;
        }
        TestFeatures();
        TestForwardAndWeights();
        TestPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        TestPosition("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");
        TestPosition("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1");
        TestPosition("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
        TestPosition("3qk3/4P3/8/8/8/8/8/4K3 w - - 0 1");
        TestIncrementalMove("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "e2e4");
        TestIncrementalMove("4k3/8/8/8/3pP3/8/8/4K3 w - - 0 1", "e4d5");
        TestIncrementalMove("4k3/P7/8/8/8/8/8/4K3 w - - 0 1", "a7a8q");
        TestIncrementalMove("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5d6");
    TestIncrementalMove("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "e1g1");
    TestIncrementalSequence();
        BoardInitializer::Cleanup();
        std::cout << "NNUE prototype tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
