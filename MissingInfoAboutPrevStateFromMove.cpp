#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "MissingInfoAboutPrevStateFromMove.h"

#include "Option.h"

MissingInfoAboutPrevStateFromMove::MissingInfoAboutPrevStateFromMove(Board& board4)
    : previousUnpassentPlace(board4.unpassentPlace)
    , previousWhiteBigCastle(board4.whiteBigCastle)
    , previousWhiteSmallCastle(board4.whiteSmallCastle)
    , previousBlackBigCastle(board4.blackBigCastle)
    , previousBlackSmallCastle(board4.blackSmallCastle)
    , movedPieceIndex(-1)
    , capturedPieceIndex(-1)
{
}

MissingInfoAboutPrevStateFromMove::MissingInfoAboutPrevStateFromMove(Board& board4, const Move& move)
    : MissingInfoAboutPrevStateFromMove(board4)
{
    if (move.promotionPiece > 0)
    {
        int pawnPiece = board4.mainBoard[move.beginPlace];
        if (pawnPiece == 0) pawnPiece = board4.sideToMove ? 9 : 1;
        movedPieceIndex = board4.pieces[pawnPiece].find(move.beginPlace);
        if ((move.PublicFlag & Option::PowerTwo[7]) != 0)
        {
            int capPiece = move.endPiece > 0 ? move.endPiece : board4.mainBoard[move.endPlace];
            if (capPiece > 0)
            {
                capturedPieceIndex = board4.pieces[capPiece].find(move.endPlace);
            }
        }
    }
    else if ((move.PublicFlag & Option::PowerTwo[6]) != 0)
    {
        int capPawn = board4.sideToMove ? 1 : 9;
        int capPlace = board4.sideToMove ? (move.endPlace + 8) : (move.endPlace - 8);
        capturedPieceIndex = board4.pieces[capPawn].find(capPlace);
    }
    else if ((move.PublicFlag & Option::PowerTwo[7]) != 0)
    {
        int capPiece = move.endPiece > 0 ? move.endPiece : board4.mainBoard[move.endPlace];
        if (capPiece > 0)
        {
            capturedPieceIndex = board4.pieces[capPiece].find(move.endPlace);
        }
    }
}