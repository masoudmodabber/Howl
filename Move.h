#ifndef MOVE_H
#define MOVE_H
#include <cstddef>

class Move
{
public:
    static void* operator new(std::size_t size);
    static void operator delete(void* pointer) noexcept;
    static void SetPoolEnabled(bool enabled);

    int beginPlace;
    int endPlace;
    int endPiece;
    int promotionPiece;
    char CastleFlag;
    char PublicFlag;
    int unpassentPlace;
    int value;
    bool givesCheck = false;
    bool isRefuteWithoutNullMove = false;
    int depth;
    int depthGone;
    int moveCount = -5;
};

#endif
