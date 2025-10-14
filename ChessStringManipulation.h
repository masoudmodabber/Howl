#ifndef CHESSSTRINGMANIPULATION_H
#define CHESSSTRINGMANIPULATION_H

#include "Move.h"
#include "Board.h"
#include <string>

class ChessStringManipulation
{
public:

    static Move* ConvertTextToMove(const std::string& move, Board& thisBoard);
    static std::string PVToString(const Move& move, int type, bool mated, const Board& thisBoard);

private:
    static Move* ConvertTextToMoveWithoutFlags(const std::string& move);
};

#endif