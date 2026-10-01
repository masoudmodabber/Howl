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

    explicit MissingInfoAboutPrevStateFromMove(const Board& board4) noexcept
        : previousUnpassentPlace(board4.unpassentPlace)
        , previousWhiteBigCastle(board4.whiteBigCastle)
        , previousWhiteSmallCastle(board4.whiteSmallCastle)
        , previousBlackBigCastle(board4.blackBigCastle)
        , previousBlackSmallCastle(board4.blackSmallCastle)
        , movedPieceIndex(-1)
        , capturedPieceIndex(-1)
    {
    }

    MissingInfoAboutPrevStateFromMove(const Board& board4, const Move&) noexcept
        : MissingInfoAboutPrevStateFromMove(board4)
    {
    }
};

#endif
