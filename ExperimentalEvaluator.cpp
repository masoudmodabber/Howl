#include "ExperimentalEvaluator.h"
#include "EvaluationLogic.h"
#include "NNUEEvaluator.h"

#include <algorithm>
#include <cmath>

namespace { ExperimentalEvaluator::Mode mode = ExperimentalEvaluator::Mode::Classical; }

void ExperimentalEvaluator::SetMode(Mode value) { mode = value; }
ExperimentalEvaluator::Mode ExperimentalEvaluator::GetMode() { return mode; }

int ExperimentalEvaluator::Evaluate(Board& board)
{
    if (mode == Mode::Classical)
        return EvaluationLogic::Evaluate(board);
    if (!board.nnueState.initialized)
        NNUEEvaluator::Rebuild(board);
    const float raw = NNUEEvaluator::Evaluate(board);
    if (mode == Mode::NNUEStaticLinear)
        return static_cast<int>(std::lround(raw * 3000.0f));
    const float bounded = std::clamp(raw, -0.999999f, 0.999999f);
    return static_cast<int>(std::lround(std::atanh(bounded) * 600.0f));
}
