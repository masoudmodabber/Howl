#ifndef MOVE_H
#define MOVE_H

class Move
{
public:

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
