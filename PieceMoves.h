#ifndef PIECEMOVES_H
#define PIECEMOVES_H

#include "Move.h"
#include <vector>

class PieceMoves
{
public:
    static void Initialize();
    static void Cleanup();
    static Move *WhitePawnMoves[64][18];
    static Move *BlackPawnMoves[64][18];
    static Move *WhiteKingMoves[64][18];
    static Move *BlackKingMoves[64][18];
    static Move *KnightMoves[64][16];
    static std::vector<Move *> BishopMoves[64][8];
    static std::vector<Move *> RookMoves[64][8];
    static std::vector<Move *> QueenMoves[64][16];
    static long long pawnTwoMove[64];

private:
    static bool initialized;
};

#endif