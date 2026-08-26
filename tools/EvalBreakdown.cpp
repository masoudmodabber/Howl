#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <sstream>
#include "BoardMaker.h"
#include "EvaluationLogic.h"
#include "BoardInitializer.h"
#include "Option.h"
#include "AttackPlaces.h"
#include "PieceMoves.h"
#include "MoveLogic.h"
#include "KingSetup.h"
#include "PassedPawnSetup.h"
#include "HashMemoryBudget.h"
#include "UCI.h"

void InitializeEngine()
{
    UCI::IsRelease = true;
    Option::Initialize();
    AttackPlaces::Initialize();
    BoardInitializer::Initialize();
    PieceMoves::Initialize();
    MoveLogic::Initialize();
    KingSetup::Initialize();
    PassedPawnSetup::Initialize();
    std::ostringstream diagnostics;
    HashMemoryBudget::EnsureDefaultConfigured(diagnostics);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: howl_eval_breakdown \"<FEN>\"\n";
        return 1;
    }

    std::string fen;
    for (int i = 1; i < argc; ++i)
    {
        if (i > 1) fen += " ";
        fen += argv[i];
    }

    InitializeEngine();
    EvaluationLogic::ClearEvalCacheForTesting();

    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    if (!board)
    {
        std::cerr << "Error: invalid FEN\n";
        return 1;
    }

    EvaluationBreakdown bd = EvaluationLogic::EvaluateDetailed(*board);

    std::cout << "FEN: " << fen << "\n";
    std::cout << "Side to move: " << (board->sideToMove ? "Black" : "White")
              << " | Phase: " << bd.phase << "/24 | State: " << bd.state << "\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << std::left << std::setw(22) << "Term" 
              << std::right << std::setw(10) << "White" 
              << std::setw(10) << "Black" 
              << std::setw(10) << "Net" << "\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << std::left << std::setw(22) << "Material" 
              << std::right << std::setw(10) << bd.whiteMaterial 
              << std::setw(10) << bd.blackMaterial 
              << std::setw(10) << bd.materialNet << "\n";
    std::cout << std::left << std::setw(22) << "Piece Eval (Balance)" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.pieceEvaluation << "\n";
    std::cout << std::left << std::setw(22) << "Bishop Pair" 
              << std::right << std::setw(10) << bd.whiteBishopPair 
              << std::setw(10) << bd.blackBishopPair 
              << std::setw(10) << bd.bishopPairNet << "\n";
    std::cout << std::left << std::setw(22) << "Mobility" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.mobilityNet << "\n";
    std::cout << std::left << std::setw(22) << "Center" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.centerNet << "\n";
    std::cout << std::left << std::setw(22) << "King Attack" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.kingAttackNet << "\n";
    std::cout << std::left << std::setw(22) << "King Placement" 
              << std::right << std::setw(10) << bd.whiteKingPlacement 
              << std::setw(10) << bd.blackKingPlacement 
              << std::setw(10) << bd.kingPlacementNet << "\n";
    std::cout << std::left << std::setw(22) << "Pawn Shield" 
              << std::right << std::setw(10) << bd.whitePawnShield 
              << std::setw(10) << bd.blackPawnShield 
              << std::setw(10) << bd.pawnShieldNet << "\n";
    std::cout << std::left << std::setw(22) << "Central King Exposure" 
              << std::right << std::setw(10) << bd.whiteCentralKingExposure 
              << std::setw(10) << bd.blackCentralKingExposure 
              << std::setw(10) << bd.centralKingExposureNet << "\n";
    std::cout << std::left << std::setw(22) << "King Safety Total" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.kingSafetyTotal << "\n";
    std::cout << std::left << std::setw(22) << "Pawn Structure" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.pawnStructureNet << "\n";
    std::cout << std::left << std::setw(22) << "Rook Connectivity" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.rookConnectionNet << "\n";
    std::cout << std::left << std::setw(22) << "Tempo" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.tempoNet << "\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << std::left << std::setw(22) << "Unscaled Total" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.unscaledTotal << "\n";
    if (bd.oppositeColorBishopScale != 1.0)
    {
        std::cout << std::left << std::setw(22) << "OCB Scale" 
                  << std::right << std::setw(10) << "-" 
                  << std::setw(10) << "-" 
                  << std::setw(10) << bd.oppositeColorBishopScale << "\n";
    }
    std::cout << std::left << std::setw(22) << "White Perspective" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.whitePerspectiveTotal << "\n";
    std::cout << std::left << std::setw(22) << "Side-to-move Total" 
              << std::right << std::setw(10) << "-" 
              << std::setw(10) << "-" 
              << std::setw(10) << bd.sideToMoveTotal << "\n";
    std::cout << "--------------------------------------------------------\n";

    return 0;
}
