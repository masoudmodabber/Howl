#ifndef SEARCH_PARAMETERS_H
#define SEARCH_PARAMETERS_H

struct SearchParameters
{
    struct NullMove
    {
        static constexpr int Base = 854;
        static constexpr int DepthSlope = 68;
        static constexpr int Divisor = 258;
        static constexpr int EvalDivisor = 192;
        static constexpr int MaxEvalBonus = 3;
        static constexpr int StaticEvalDepthSlope = 32;
        static constexpr int StaticEvalBase = 292;
        static constexpr int ImprovingMargin = 30;
        static constexpr int StatScoreLimit = 23397;
        static constexpr int VerificationDepth = 13;
        static constexpr int SuppressionNumerator = 3;
        static constexpr int SuppressionDenominator = 4;
    };

    struct History
    {
        static constexpr int MainLimit = 16384;
        static constexpr int CaptureLimit = 16384;
        static constexpr int ContinuationLimit = 29952;
        static constexpr int BonusDeepDepth = 15;
        static constexpr int BonusDeepValue = -8;
        static constexpr int BonusQuadratic = 19;
        static constexpr int BonusLinear = 155;
        static constexpr int BonusConstant = -132;
        static constexpr int LmrStatOffset = 4926;
        static constexpr int ParentFutilityLimit = 25000;
        static constexpr int TtCutoffPreviousMoveCount = 2;
        static constexpr int LmrKillerBonusDivisor = 4;
    };

    struct LMR
    {
        static constexpr double LogarithmicScale = 24.8;
        static constexpr int RoundingTerm = 511;
        static constexpr int Divisor = 1024;
        static constexpr int NonImprovingThreshold = 1007;
        static constexpr int TtHitLowThreshold = 375;
        static constexpr int TtHitHighThreshold = 500;
        static constexpr int TtPvAdjustment = 2;
        static constexpr int PreviousMoveCountThreshold = 14;
        static constexpr int SingularAdjustment = 2;
        static constexpr int TtCaptureAdjustment = 1;
        static constexpr int CutNodeAdjustment = 2;
        static constexpr int EscapeCaptureAdjustment = 2;
        static constexpr int GoodStatScore = -102;
        static constexpr int BadPreviousStatScore = -114;
        static constexpr int GoodPreviousStatScore = -116;
        static constexpr int BadStatScore = -154;
        static constexpr int StatScoreDivisor = 16384;
        static constexpr int ShallowCaptureDepth = 8;
        static constexpr int LateCaptureMoveCount = 2;
    };

    struct MoveCountPruning
    {
        static constexpr int Base = 5;
        static constexpr int DepthQuadratic = 1;
        static constexpr int ImprovingBase = 1;
        static constexpr int Divisor = 2;
        static constexpr int Offset = 1;
    };

    struct Razoring
    {
        static constexpr int MaxDepth = 2;
        static constexpr int Margin = 531;
    };

    struct ReverseFutility
    {
        static constexpr int MaxDepth = 6;
        static constexpr int DepthMargin = 217;
    };

    struct ParentFutility
    {
        static constexpr int MaxReducedDepth = 6;
        static constexpr int BaseMargin = 235;
        static constexpr int DepthMargin = 172;
    };

    struct CounterMovePruning
    {
        static constexpr int ReducedDepthBase = 4;
        static constexpr int ContinuationThreshold = 0;
    };

    struct SEE
    {
        static constexpr int TacticalDepthMargin = 194;
        static constexpr int QuietBase = 32;
        static constexpr int QuietDepthCap = 18;
        static constexpr int GoodCaptureCoefficient = 55;
    };

    struct ProbCut
    {
        static constexpr int MinDepth = 5;
        static constexpr int BetaMargin = 189;
        static constexpr int ImprovingMargin = 45;
    };

    struct QSearch
    {
        static constexpr int FutilityMargin = 154;
    };

    struct MoveOrdering
    {
        static constexpr int StrongQuietDepthCoefficient = -3000;
        static constexpr int CaptureVictimMultiplier = 6;
        static constexpr int GoodCaptureSeeDivisor = 1024;
    };

    struct StaticEvaluation
    {
        static constexpr int StatScoreDivisor = 512;
    };
};

#endif
