#ifndef BOARDMAKER_H
#define BOARDMAKER_H

#include <string>
#include "Board.h"

class BoardMaker {
public:

    static Board* MakeInitialBoard(std::string position);
    static Board* MakeBoard(int boardArray[]);
};

#endif