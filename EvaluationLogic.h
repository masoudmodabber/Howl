#ifndef EVALUATIONLOGIC_H
#define EVALUATIONLOGIC_H

#include "ChessCache.h"
#include "Board.h"
#include "PawnCache.h"
class EvaluationLogic
{
public:

    static int Evaluate(Board& thisBoard);
    static int GetPawnStructureValue(Board& thisBoard, int state);
    static int* PieceMoveCount(Board& thisBoard, int state);

private:
    static ChessCache EvalCache;
    static PawnCache PawnEvalCache;
};

#endif