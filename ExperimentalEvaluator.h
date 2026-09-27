#pragma once

#include "Board.h"
#include <string>

class ExperimentalEvaluator
{
public:
    enum class Mode { Classical, NNUE, NNUEStaticLinear, StructuredNNUE };
    static void SetMode(Mode mode);
    static void SetStructuredNNUEWeightsPath(const std::string& path);
    static Mode GetMode();
    static int Evaluate(Board& board);
};
