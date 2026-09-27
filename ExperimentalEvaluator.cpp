#include "ExperimentalEvaluator.h"
#include "EvaluationLogic.h"
#include "NNUEEvaluator.h"
#include "StructuredNNUEEvaluator.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>

namespace { ExperimentalEvaluator::Mode mode = ExperimentalEvaluator::Mode::Classical; StructuredNNUEEvaluator structured; bool structuredLoaded=false; std::string structuredWeightsPath; }

void ExperimentalEvaluator::SetMode(Mode value) { mode = value; }
void ExperimentalEvaluator::SetStructuredNNUEWeightsPath(const std::string& path) { structuredWeightsPath=path; structuredLoaded=false; }
ExperimentalEvaluator::Mode ExperimentalEvaluator::GetMode() { return mode; }

int ExperimentalEvaluator::Evaluate(Board& board)
{
    if (mode == Mode::Classical)
        return EvaluationLogic::Evaluate(board);
    if (mode == Mode::StructuredNNUE) {
        if (structuredWeightsPath.empty()) throw std::runtime_error("StructuredNNUE selected but StructuredNNUEWeights was not supplied");
        if (!structuredLoaded) {
            const std::string resolved=std::filesystem::absolute(structuredWeightsPath).string();
            structured.Load(resolved);
            structuredLoaded=true;
            std::cerr << "StructuredNNUE weights: " << resolved << "\n";
        }
        return static_cast<int>(std::lround(structured.Evaluate(board)));
    }
    if (!board.nnueState.initialized)
        NNUEEvaluator::Rebuild(board);
    const float raw = NNUEEvaluator::Evaluate(board);
    if (mode == Mode::NNUEStaticLinear)
        return static_cast<int>(std::lround(raw * 3000.0f));
    const float bounded = std::clamp(raw, -0.999999f, 0.999999f);
    return static_cast<int>(std::lround(std::atanh(bounded) * 600.0f));
}
