#ifndef NNUEEVALUATOR_H
#define NNUEEVALUATOR_H

#include "Board.h"
#include <string>
#include <cstdint>
class Move;

class NNUEEvaluator
{
public:
    static constexpr int HiddenSize = 256;
    static constexpr int ColoredPieceClasses = 10;
    static constexpr int Squares = 64;
    static constexpr int KingAnchors = 64;
    static constexpr int FeatureCount = KingAnchors * ColoredPieceClasses * Squares;

    static int FeatureIndex(int anchorKingSquare, int coloredPieceClass, int pieceSquare);
    static int ColoredPieceClass(int pieceId);
    static int FeatureIndexForPerspective(int kingSquare, bool whitePerspective, int pieceId, int pieceSquare);
    static float Weight(int feature, int unit);
    static void Rebuild(Board& board);
    static void UpdateAfterMove(Board& board, const Move& move, const NNUEState& previous);
    static float Evaluate(const Board& board);
    static void SaveWeights(const std::string& path);
    static void LoadWeights(const std::string& path);
    struct Profile { std::uint64_t forwardCalls=0, updateCalls=0, rebuildCalls=0, snapshotSaveCalls=0, snapshotRestoreCalls=0, makeCalls=0, undoCalls=0; double forwardMs=0, updateMs=0, rebuildMs=0, snapshotSaveMs=0, snapshotRestoreMs=0, makeMs=0, undoMs=0; };
    static void ResetProfile();
    static Profile GetProfile();
    static void SaveSnapshot(Board& board); static NNUEState RestoreSnapshot(Board& board);
};

#endif
