#ifndef UCI_H
#define UCI_H

#include <thread>
#include <string>
#include <cstddef>
#include <iosfwd>
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
    static void ReplaceCurrentBoard(Board* replacementBoard);
    static void ReplaceMoveHistory(LastFourMoves* replacementHistory);
    static void ReleaseCurrentPosition();

#if HOWL_CORRECTNESS_TESTING
    static void ResetPositionOwnershipStatistics();
    static std::size_t ReleasedBoardCount();
    static std::size_t ReleasedHistoryMoveCount();
#endif

    static void MainAsync();
    static void MainSearchStart();
    static bool IsTest();
    static bool ApplyHashOptionCommand(const std::string& command,
                                       std::ostream& diagnostics);

private:
    static std::chrono::nanoseconds getAllowedTime(int TimeMill, int IncMill, int remainedMoves);
    static void ReleaseMoveHistory();
#if HOWL_CORRECTNESS_TESTING
    static std::size_t releasedBoards;
    static std::size_t releasedHistoryMoves;
#endif
};

#endif
