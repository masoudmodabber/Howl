#ifndef QSEARCHER_H
#define QSEARCHER_H

#include "MovePrintValue.h"
#include "Move.h"
#include "Board.h"
#include "MoveLogic.h"

#ifdef HOWL_CORRECTNESS_TESTING
struct QSearchTestStatistics {
    int rootGeneratedMoves = 0;
    int rootLegalMoves = 0;
    int rootAvailableMoves = 0;
    int rootIllegalMovesBeforeFirstSearch = 0;
    int rootFullWindowResearches = 0;
    bool firstLegalSearchedMoveUsedFullWindow = false;
};
#endif

class QSearcher {
public:

#ifdef HOWL_CORRECTNESS_TESTING
    static void ResetTestStatistics();
    static QSearchTestStatistics TestStatistics();
#endif

    static MovePrintValue* QSearch(bool isPVNode, int alpha, int beta, Move& prevMove, int depthGone, int lastCheck, bool kick, int depth, Move& move1, Move& move2, Move& move3, Board& board4, bool MAtESearch, int depthQuisStarted, bool nullWindowSearch);

private:
    static int pieceValue100[15];
    static void deleteMoveList(MoveList moveList);
};

#endif
