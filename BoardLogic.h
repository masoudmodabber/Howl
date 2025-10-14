#ifndef BOARDLOGIC_H
#define BOARDLOGIC_H

#include <vector>
#include "Board.h"
#include "AttackPlaces.h"
#include "Option.h"

class BoardLogic {
public:

    static bool UnderAttack(Board& thisBoard, int position, bool attackerSide);
};

#endif