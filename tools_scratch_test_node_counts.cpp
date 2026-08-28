#include <iostream>
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
#include "Search.h"
#include "PVSSearch.h"
#include "QSearcher.h"
#include "BoardLogic.h"
#include "GameLogic.h"
#include "ChessStringManipulation.h"
#include "TranspositionTable.h"
#include "RepetitionHistory.h"

int main() {
    UCI::IsRelease = true;
    Option::Initialize();
    AttackPlaces::Initialize();
    BoardInitializer::Initialize();
    PieceMoves::Initialize();
    MoveLogic::Initialize();
    KingSetup::Initialize();
    PassedPawnSetup::Initialize();
    std::ostringstream diag;
    HashMemoryBudget::EnsureDefaultConfigured(diag);

    const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    for (int d = 1; d <= 3; ++d) {
        std::unique_ptr<Board> b(BoardMaker::MakeInitialBoard(fen));
        TranspositionTable::Clear();
        RepetitionHistory::ResetWithRoot(b->ZobristHashCode);
        PVSSearch::ResetKillers();

        Move m1{}, m2{}, m3{}, m4{};
        Search::active = true;
        Search::maxDepth = d;
        Search::maxNodes = -1;
        Search::finiteSearch = true;
        Search::startTime = std::chrono::high_resolution_clock::now();
        Search::MainSearch(m1, m2, m3, m4, *b);
        std::cout << "Depth " << d << " completed with nodes = " << Search::moveCount << ", bestMove = " << Search::completedBestMove << "\n";
    }
    return 0;
}
