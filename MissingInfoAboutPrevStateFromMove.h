#ifndef MISSINGINFOABOUTPREVSTATEFROMMOVE_H
#define MISSINGINFOABOUTPREVSTATEFROMMOVE_H

#include "Board.h"
#include "Move.h"

class MissingInfoAboutPrevStateFromMove
{
public:

    int previousUnpassentPlace;
    bool previousWhiteBigCastle;
    bool previousWhiteSmallCastle;
    bool previousBlackBigCastle;
    bool previousBlackSmallCastle;
    int movedPieceIndex;
    int capturedPieceIndex;

    MissingInfoAboutPrevStateFromMove(Board& board4);
    MissingInfoAboutPrevStateFromMove(Board& board4, const Move& move);
};

#endif
