#ifndef MISSINGINFOABOUTPREVSTATEFROMMOVE_H
#define MISSINGINFOABOUTPREVSTATEFROMMOVE_H

#include "Board.h"
class MissingInfoAboutPrevStateFromMove
{
public:

    int previousUnpassentPlace;
    bool previousWhiteBigCastle;
    bool previousWhiteSmallCastle;
    bool previousBlackBigCastle;
    bool previousBlackSmallCastle;

    MissingInfoAboutPrevStateFromMove(Board& board4);
};

#endif