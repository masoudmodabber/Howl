#pragma once
#include "Board.h"
#include "StructuredNNUEWeights.h"
#include <string>
#include <array>
class Move;
class StructuredNNUEEvaluator {
public:
    void Load(const std::string& path);
    float Evaluate(Board& board) const;
    void Rebuild(Board& board) const;
    void UpdateAfterMove(Board& board, const Move& move, const NNUEState& previous) const;
    std::size_t ParameterCount() const { return weights.ParameterCount(); }
private:
    StructuredNNUEWeights weights;
    static int cls(int piece, bool whitePerspective);
    float* tensor(const char* name) const;
    void AddPiece(Board& board, int piece, int square) const;
    void RemovePiece(Board& board, int piece, int square) const;
    void MovePiece(Board& board, int piece, int from, int to) const;
    void RebuildWhite(Board& board) const;
    void RebuildBlack(Board& board) const;
};
