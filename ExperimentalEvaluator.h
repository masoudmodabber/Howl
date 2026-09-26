#pragma once

#include "Board.h"

class ExperimentalEvaluator
{
public:
    enum class Mode { Classical, NNUE, NNUEStaticLinear };
    static void SetMode(Mode mode);
    static Mode GetMode();
    static int Evaluate(Board& board);
};
