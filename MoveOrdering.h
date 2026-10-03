#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include "Move.h"
#include <cstdint>

struct MoveOrderingTieKey
{
    int value = 0;
    int givesCheck = 0;
    int promotion = 0;
    int movingPiece = 0;
    int capturedPiece = 0;
    std::uint64_t identity = 0;
};

inline MoveOrderingTieKey MakeMoveOrderingTieKey(const Move& move,
                                                 int movingPiece,
                                                 bool includeValue,
                                                 bool includeCheck)
{
    const auto field = [](int value) -> std::uint64_t {
        return static_cast<std::uint64_t>(static_cast<std::uint8_t>(value));
    };
    std::uint64_t identity = field(move.beginPlace);
    identity |= field(move.endPlace) << 8;
    identity |= field(move.endPiece) << 16;
    identity |= field(move.promotionPiece) << 24;
    identity |= field(move.CastleFlag) << 32;
    identity |= field(move.PublicFlag) << 40;
    identity |= field(move.unpassentPlace) << 48;

    return {
        includeValue ? move.value : 0,
        includeCheck && move.givesCheck ? 1 : 0,
        move.promotionPiece > 0 ? move.promotionPiece : 0,
        movingPiece > 0 ? movingPiece : 0,
        move.endPiece > 0 ? move.endPiece % 8 : 0,
        identity
    };
}

inline bool MoveOrderingTieKeyGreater(const MoveOrderingTieKey& left,
                                      const MoveOrderingTieKey& right)
{
    if (left.value != right.value) return left.value > right.value;
    if (left.givesCheck != right.givesCheck) return left.givesCheck > right.givesCheck;
    if (left.promotion != right.promotion) return left.promotion > right.promotion;
    if (left.movingPiece != right.movingPiece) return left.movingPiece > right.movingPiece;
    if (left.capturedPiece != right.capturedPiece) return left.capturedPiece > right.capturedPiece;
    return left.identity > right.identity;
}

#endif
