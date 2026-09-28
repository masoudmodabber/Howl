#pragma once

#include "Board.h"
#include <string>

class Move;

class ExperimentalEvaluator
{
public:
    enum class Mode { Classical, NNUE, NNUEStaticLinear, StructuredNNUE };
    static void SetMode(Mode mode);
    static void SetStructuredNNUEWeightsPath(const std::string& path);
    static Mode GetMode();
    static void PrepareStructured(Board& board);
    static void UpdateStructuredAfterMove(Board& board, const Move& move, const NNUEState& previous);
    static int Evaluate(Board& board);
};
