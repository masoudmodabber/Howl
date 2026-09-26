#pragma once
#include "Board.h"
#include "StructuredNNUEWeights.h"
#include <string>
#include <array>
class StructuredNNUEEvaluator {
public:
    void Load(const std::string& path);
    float Evaluate(Board& board) const;
    void Rebuild(Board& board) const;
    std::size_t ParameterCount() const { return weights.ParameterCount(); }
private:
    StructuredNNUEWeights weights;
    static int cls(int piece, bool whitePerspective);
    float* tensor(const char* name) const;
};
