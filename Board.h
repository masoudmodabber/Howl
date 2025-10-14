#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include "MyList.h"

class Board
{
public:
    long long whitePieces;
    long long whitePawns;
    long long blackPieces;
    long long blackPawns;
    long long ZobristHashCode;
    int moveNumber;
    int fiftyMoveRule;
    bool sideToMove;
    bool whiteSmallCastle;
    bool whiteBigCastle;
    bool blackSmallCastle;
    bool blackBigCastle;
    int unpassentPlace;
    int mainBoard[64];
    MyList pieces[15];

    Board *MakeCopy();
    static bool AreBoardsEqual(Board &board1, Board &board2);
};

#endif