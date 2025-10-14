#ifndef UCI_H
#define UCI_H

#include <thread>
#include <string>
#include "Move.h"
#include "Board.h"
#include "Search.h"
#include "LastFourMoves.h"

// UCI class definition
class UCI {
public:

    // TRUE = Real Mode
    static bool IsRelease;
    static bool TestOrderAvailable;
    static bool StartPosForTest;

    static Move* move1;
    static Move* move2;
    static Move* move3;
    static Move* move4;
    static Board* thisBoard;
    static std::string order;

    static void SetAutomaticOrders(int savedOrderToProcessNo);
    static LastFourMoves* MakeMoves(std::string moves, Board& thisBoard);

    static void MainAsync();
    static void MainSearchStart();
    static bool IsTest();

private:
    static std::chrono::nanoseconds getAllowedTime(int TimeMill, int IncMill, int remainedMoves);
};

#endif