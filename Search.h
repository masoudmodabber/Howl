#ifndef SEARCH_H
#define SEARCH_H

#include <string>
#include <vector>
#include <ctime>
#include "MoveLogic.h" // Use MoveLogic.h for MoveList definition
#include "Move.h"
#include "Board.h"
#include "MovePrintValue.h"

class Search
{
public:

    static bool active;
    static time_t beginTime;
    static double allowedTime;
    static std::string bestMove;
    static std::string ponderMove;
    static bool finiteSearch;

    static int overAllIteration;
    static int moveCount;
    static std::string Score;
    static bool mated;

    static void MainSearch(Move& move1, Move& move2, Move& move3, Move& move4, Board& board4);
    static void SearchDepthZero(MoveList& moveList, bool& firstAssign, int& recDepth, int& alpha, int& beta, bool& previousMoveWasCheck, Move& move1, Move& move2, Move& move3, Move& move4, Board& board4);
    static void SearchForCheckUpdate();
    static void CalculateAndDisplayScore(int value);
    static std::string Parse(std::string p, int place);
    static int PrintKBest(std::vector<MovePrintValue*>& movesPrintValue, int KBest, bool finiteSearch);
    
private:
    static void deleteMoveList(std::vector<Move*>* moveList);
    static void deleteMovesPrintValue(std::vector<MovePrintValue*>& movesPrintValue);
};

#endif