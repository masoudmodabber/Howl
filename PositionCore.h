#ifndef POSITION_CORE_H
#define POSITION_CORE_H

#include <cstdint>

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

class Board;
class Move;

namespace PositionCoreLogic
{
void Initialize(Board& board);
void UpdateAfterMove(Board& board, const Move& move, bool movingWhite);
void UpdateAfterUndo(Board& board, const Move& move, bool movingWhite);
void Verify(const Board& board, const Move* move, const char* operation);
}

#endif
