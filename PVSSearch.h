#ifndef PVSSearch_H
#define PVSSearch_H

#include "MovePrintValue.h"
#include "Move.h"
#include "Board.h"
#include "MoveLogic.h"

class PVSSearch {
    static constexpr bool ProductionIGGEnabled = false;

public:
    static MovePrintValue* PVS(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch);
    static void IGG(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool lastCheck, bool nullWindowSearch, MoveList moveList);
    static void deleteMoveList(MoveList moveList);
#if HOWL_CORRECTNESS_TESTING
    static constexpr bool ProductionIGGEnabledForTesting()
    {
        return ProductionIGGEnabled;
    }
    static double NullMoveReductionForTesting(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool mateSearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch)
    {
        return NullMoveReduction(isPVNode, alpha, beta, depth, prevMove, move1, move2, move3, board4, mateSearch, isNullMoveAllowed, depthGone, previousMoveWasCheck, nullWindowSearch);
    }
    static int FutilityPruningSkippedQuietMovesForTesting();
    static void ResetFutilityPruningSkippedQuietMovesForTesting();
#endif

private:
    static int moveOrderingDepth[20];
    static void NullMovePruning(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool mAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, MovePrintValue mPValue);
    static double NullMoveReduction(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool mateSearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch);
    static MovePrintValue* StartQSearch(bool isPVNode, int alpha, int beta, Move& prevMove, int depthGone, Move& move1, Move& move2, Move& move3, Board& board4, bool nullWindowSearch, bool previousMoveWasCheck);
};

#endif
