#ifndef MOVE_H
#define MOVE_H
#include <cstddef>
#include <cstdint>

class Move
{
public:
    static void* operator new(std::size_t size);
    static void operator delete(void* pointer) noexcept;
    static void SetPoolEnabled(bool enabled);

    int8_t beginPlace;
    int8_t endPlace;
    int8_t endPiece;
    int8_t promotionPiece;
    char CastleFlag;
    char PublicFlag;
    int8_t unpassentPlace;
    bool givesCheck = false;
    bool givesCheckComputed = false;
    bool isRefuteWithoutNullMove = false;
    int16_t pad = 0;
    int value;
};

static_assert(sizeof(Move) == 16, "Move layout must be exactly 16 bytes");

#endif
