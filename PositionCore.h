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
    Bitboard mask;

    struct Iterator
    {
        Bitboard remaining;
        int operator*() const { return __builtin_ctzll(remaining); }
        Iterator& operator++() { remaining &= remaining - 1; return *this; }
        bool operator!=(const Iterator& other) const { return remaining != other.remaining; }
    };

    Iterator begin() const { return {mask}; }
    Iterator end() const { return {0}; }
    std::size_t size() const { return static_cast<std::size_t>(__builtin_popcountll(mask)); }
    bool empty() const { return mask == 0; }
    int front() const { return __builtin_ctzll(mask); }
    int operator[](std::size_t index) const
    {
        Bitboard remaining = mask;
        while (index-- != 0)
            remaining &= remaining - 1;
        return __builtin_ctzll(remaining);
    }
};

struct PositionCorePieceListsView
{
    const PositionCore& core;

    PositionCorePieceListView operator[](int piece) const
    {
        if (piece < 1 || (piece > 6 && piece < 9) || piece > 14)
            return {0};
        const int type = (piece > 8 ? piece - 8 : piece) - 1;
        const int colour = piece > 8 ? 1 : 0;
        return {core.pieceOccupancy[type] & core.colourOccupancy[colour]};
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
