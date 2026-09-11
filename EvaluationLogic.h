#ifndef EVALUATIONLOGIC_H
#define EVALUATIONLOGIC_H

#include "EvaluationChessCache.h"
#include "Board.h"
#include "PawnCache.h"
#include <cstddef>
struct EvaluationBreakdown
{
    int phase = 0;

    int whiteMaterial = 0;
    int blackMaterial = 0;
    int materialNet = 0;
    double pieceBalance = 1.0;
    int pieceEvaluation = 0;

    int whiteBishopPair = 0;
    int blackBishopPair = 0;
    int bishopPairNet = 0;

    int mobilityNet = 0;
    int pieceAttacksNet = 0;
    int whiteRookFileBonus = 0;
    int blackRookFileBonus = 0;
    int rookFileBonusNet = 0;
    int centerNet = 0;

    int kingAttackNet = 0;
    int whiteKingPlacement = 0;
    int blackKingPlacement = 0;
    int kingPlacementNet = 0;
    int whitePawnShield = 0;
    int blackPawnShield = 0;
    int pawnShieldNet = 0;
    int whiteCentralKingExposure = 0;
    int blackCentralKingExposure = 0;
    int centralKingExposureNet = 0;
    int whiteCentralKingAttackPressure = 0;
    int blackCentralKingAttackPressure = 0;
    int centralKingAttackPressureNet = 0;
    int centralGeneralOpenness = 0;
    int centralEffectiveOpenness = 0;
    int centralDFileExposure = 0;
    int centralEFileExposure = 0;
    bool centralCentreLocked = false;
    bool centralCentreOpen = false;
    bool whiteCentralKingActive = false;
    bool blackCentralKingActive = false;
    int whiteHeavyLinePressure = 0;
    int blackHeavyLinePressure = 0;
    int whiteBishopDiagonalPressure = 0;
    int blackBishopDiagonalPressure = 0;
    int whiteDirectHeavyLines = 0;
    int blackDirectHeavyLines = 0;
    int whiteOneBlockerHeavyLines = 0;
    int blackOneBlockerHeavyLines = 0;
    int whiteMultiBlockerHeavyLines = 0;
    int blackMultiBlockerHeavyLines = 0;
    int whiteDirectBishopLines = 0;
    int blackDirectBishopLines = 0;
    int whiteOneBlockerBishopLines = 0;
    int blackOneBlockerBishopLines = 0;
    int whiteMultiBlockerBishopLines = 0;
    int blackMultiBlockerBishopLines = 0;
    int whiteInnerAttackers = 0;
    int blackInnerAttackers = 0;
    int whiteOuterAttackers = 0;
    int blackOuterAttackers = 0;
    int whiteInnerAttackContribution = 0;
    int blackInnerAttackContribution = 0;
    int whiteOuterAttackContribution = 0;
    int blackOuterAttackContribution = 0;
    int whiteNonlinearEscalation = 0;
    int blackNonlinearEscalation = 0;
    int whiteCastlingMitigation = 0;
    int blackCastlingMitigation = 0;
    bool whiteImmediateCastling = false;
    bool blackImmediateCastling = false;
    int kingSafetyTotal = 0;
    int whiteKingDanger = 0;
    int blackKingDanger = 0;
    int whiteAttackerWeight = 0;
    int blackAttackerWeight = 0;
    int whiteDefenderWeight = 0;
    int blackDefenderWeight = 0;
    int whiteEscapeSafety = 0;
    int blackEscapeSafety = 0;
    int whiteFilePressure = 0;
    int blackFilePressure = 0;
    int whiteDiagonalPressure = 0;
    int blackDiagonalPressure = 0;
    int whitePawnShelter = 0;
    int blackPawnShelter = 0;
    int whitePhaseScale = 0;
    int blackPhaseScale = 0;

    int pawnStructureNet = 0;
    int rookConnectionNet = 0;
    int tempoNet = 0;

    double oppositeColorBishopScale = 1.0;
    int unscaledTotal = 0;
    int scaledTotal = 0;
    bool drawAdjustmentApplied = false;

    int whitePerspectiveTotal = 0;
    int sideToMoveTotal = 0;
};

class EvaluationLogic
{
public:
    static int CalculatePhase(const Board& thisBoard);
    static int Evaluate(Board& thisBoard);
    static EvaluationBreakdown EvaluateDetailed(Board& thisBoard);
    static int GetPawnStructureValue(Board& thisBoard, int phase);
    static int* PieceMoveCount(Board& thisBoard, int phase);
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
    static int TaperEvaluationValueForTesting(int middleGameValue,
                                              int endGameValue, int phase);
    static int TaperGroup1ValueForTesting(int middleGameValue,
                                          int endGameValue, int phase);
    static int TaperGroup2ValueForTesting(int middleGameValue,
                                          int endGameValue, int phase);
    static int TaperGroup3ValueForTesting(int middleGameValue,
                                          int endGameValue, int phase);
    static int KnightOutpostValueForTesting(Board& board, int phase);
    static int PassedPawnMinorAccessibilityValueForTesting(Board& board);
    static int IsolatedPawnValueForTesting(Board& board, int phase);
    static int RookBehindPassedPawnValueForTesting(Board& board, int phase);
    static int UndefendedKingZoneDangerForTesting(Board& board, bool whiteKing);
    static void SetEvalCacheAllocationFailureThresholdForTesting(
        std::size_t capacityBytes);
#endif

private:
    static EvaluationChessCache EvalCache;
    static PawnCache PawnEvalCache;
};

#endif
