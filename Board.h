#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <array>
#include <memory>
#include "MyList.h"
#include "PositionCore.h"

struct NNUEState
{
    std::array<float, 256> whiteAccumulator{};
    std::array<float, 256> blackAccumulator{};
    int whiteKingSquare = -1;
    int blackKingSquare = -1;
    bool initialized = false;
};

struct NNUEHistory
{
    static constexpr int MaxSnapshots = 128;
    int snapshotCount = 0;
    std::array<NNUEState, MaxSnapshots> snapshots;
};

class Board
{
public:
    Board();
    Board(const Board& other);
    Board& operator=(const Board& other);
    Board(Board&& other) noexcept = default;
    Board& operator=(Board&& other) noexcept = default;
    ~Board() = default;

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
    PositionCore positionCore;
    NNUEState nnueState;
    std::unique_ptr<NNUEHistory> nnueHistory;

    Board *MakeCopy();
    static bool AreBoardsEqual(Board &board1, Board &board2, bool requireExactPieceOrder = false);
};

#endif
