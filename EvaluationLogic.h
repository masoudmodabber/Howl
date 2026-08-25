#ifndef EVALUATIONLOGIC_H
#define EVALUATIONLOGIC_H

#include "EvaluationChessCache.h"
#include "Board.h"
#include "PawnCache.h"
#include <cstddef>
class EvaluationLogic
{
public:

    static int Evaluate(Board& thisBoard);
    static int GetPawnStructureValue(Board& thisBoard, int state);
    static int* PieceMoveCount(Board& thisBoard, int state);
    static std::size_t EvalCacheSize();
    static std::size_t EvalCacheCapacityBytes();
    static std::size_t EvalCacheEntryCapacity();
    static std::size_t EvalCacheClusterCount();
    static EvaluationCacheStatistics EvalCacheStats();
    static void ResetEvalCacheStats();
    static bool ResizeEvalCache(std::size_t capacityBytes);
    static std::size_t PawnEvalCacheSize();
#if HOWL_CORRECTNESS_TESTING
    static void ClearEvalCacheForTesting();
    static int RookConnectionValueForTesting(Board& board);
    static void SetEvalCacheAllocationFailureThresholdForTesting(
        std::size_t capacityBytes);
#endif

private:
    static EvaluationChessCache EvalCache;
    static PawnCache PawnEvalCache;
};

#endif
