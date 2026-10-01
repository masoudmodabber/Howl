#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <array>
#include "MyList.h"

struct NNUEState
{
    std::array<float, 256> whiteAccumulator{};
    std::array<float, 256> blackAccumulator{};
    int whiteKingSquare = -1;
    int blackKingSquare = -1;
    bool initialized = false;
};

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
    NNUEState nnueState;
    static constexpr int MaxNNUESnapshots = 128;
    int nnueSnapshotCount = 0;
    std::array<NNUEState, MaxNNUESnapshots> nnueSnapshots;

    Board *MakeCopy();
    static bool AreBoardsEqual(Board &board1, Board &board2, bool requireExactPieceOrder = false);
};

#endif
