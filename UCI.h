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
    static Move* move1;
    static Move* move2;
    static Move* move3;
    static Move* move4;
    static Board* thisBoard;
    static std::string order;

    static LastFourMoves* MakeMoves(std::string moves, Board& thisBoard);
    static void ReplaceCurrentBoard(Board* replacementBoard);
    static void ReplaceMoveHistory(LastFourMoves* replacementHistory);
    static void ReleaseCurrentPosition();

    static std::thread searchThread;

    static void MainAsync();
    static void Run(std::istream& in, std::ostream& out);
    static void MainSearchStart();
    static bool ApplyHashOptionCommand(const std::string& command,
                                       std::ostream& diagnostics);
    static std::chrono::nanoseconds getAllowedTime(int TimeMill, int IncMill, int remainedMoves);

private:
    static void ReleaseMoveHistory();
};

#endif
