#ifndef POSITION_CORE_H
#define POSITION_CORE_H

#include <cstdint>
#include <cstddef>

using Bitboard = std::uint64_t;

struct PositionCore
{
    Bitboard colourOccupancy[2] = {};
    Bitboard pieceOccupancy[6] = {};
    std::uint8_t pieceAt[64] = {};
    // Preserves the legacy per-piece iteration order for consumers that must
    // retain exact generated move ordering during the migration.
    std::uint8_t pieceOrder[15][16] = {};
    std::uint8_t pieceOrderCount[15] = {};
    std::uint64_t zobristHash = 0;
    std::uint32_t fullmoveNumber = 0;
    std::uint16_t halfmoveClock = 0;
    std::uint8_t kingSquare[2] = {255, 255};
    std::uint8_t sideToMove = 0;
    std::uint8_t castlingRights = 0;
    std::int8_t enPassantSquare = 0;
};

struct PositionCorePieceListView
{
    const std::uint8_t* first;
    std::size_t count;

    const std::uint8_t* begin() const { return first; }
    const std::uint8_t* end() const { return first + count; }
    std::size_t size() const { return count; }
    bool empty() const { return count == 0; }
    int front() const { return first[0]; }
    int operator[](std::size_t index) const { return first[index]; }
};

struct PositionCorePieceListsView
{
    const PositionCore& core;

    PositionCorePieceListView operator[](int piece) const
    {
        return {core.pieceOrder[piece], core.pieceOrderCount[piece]};
    }
};

namespace PositionCoreLogic
{

inline long long WhiteOccupancy(const PositionCore& core)
{
    return static_cast<long long>(core.colourOccupancy[0]);
}

inline long long BlackOccupancy(const PositionCore& core)
{
    return static_cast<long long>(core.colourOccupancy[1]);
}

inline long long PawnOccupancy(const PositionCore& core, bool white)
{
    return static_cast<long long>(core.pieceOccupancy[0] &
                                  core.colourOccupancy[white ? 0 : 1]);
}

}

class Board;
class Move;
class MissingInfoAboutPrevStateFromMove;

namespace PositionCoreLogic
{
void Initialize(Board& board);
void PrepareMove(const Board& board, const Move& move,
                 MissingInfoAboutPrevStateFromMove& missingInfo);
void UpdateAfterMove(Board& board, const Move& move, bool movingWhite,
                     const MissingInfoAboutPrevStateFromMove* missingInfo);
void UpdateAfterUndo(Board& board, const Move& move, bool movingWhite,
                     const MissingInfoAboutPrevStateFromMove& missingInfo);
void Verify(const Board& board, const Move* move, const char* operation);
}

#endif
