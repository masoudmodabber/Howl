#ifndef PVSSearch_H
#define PVSSearch_H

#include "MovePrintValue.h"
#include "Move.h"
#include "Board.h"
#include "MoveLogic.h"

class PVSSearch {
    static constexpr bool ProductionIGGEnabled = false;

public:
    static MovePrintValue* PVS(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, bool nullWindowVerification = false, bool isAggressivePreprobe = false);
    static void IGG(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool lastCheck, bool nullWindowSearch, MoveList moveList);
    static void deleteMoveList(MoveList moveList);
    struct KillerMove {
        int beginPlace = -1;
        int endPlace = -1;
        int promotionPiece = 0;

        bool operator==(const Move& m) const {
            return beginPlace == m.beginPlace && endPlace == m.endPlace &&
                   (promotionPiece == 0 ? (m.promotionPiece <= 0) : (m.promotionPiece == promotionPiece));
        }
        bool operator!=(const Move& m) const {
            return !(*this == m);
        }
        bool isValid() const {
            return beginPlace >= 0 && endPlace >= 0;
        }
    };

    static constexpr int MaxKillerPly = 128;
    static KillerMove killers[MaxKillerPly][2];

    static void ResetKillers();
    static void ResetCandidateMemory();
    static void RecordKiller(int ply, const Move& move);

#if HOWL_CORRECTNESS_TESTING
    static constexpr bool ProductionIGGEnabledForTesting()
    {
        return ProductionIGGEnabled;
    }
    static double NullMoveReductionForTesting(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool mateSearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch)
    {
        return NullMoveReduction(isPVNode, alpha, beta, depth, prevMove, move1, move2, move3, board4, mateSearch, isNullMoveAllowed, depthGone, previousMoveWasCheck, nullWindowSearch);
    }
    static int FutilityPruningSkippedQuietMovesForTesting();
    static void ResetFutilityPruningSkippedQuietMovesForTesting();

    struct IndexBuckets {
        uint64_t total = 0;
        uint64_t idx0 = 0;
        uint64_t idx1 = 0;
        uint64_t idx2 = 0;
        uint64_t idx3 = 0;
        uint64_t idx4To7 = 0;
        uint64_t idx8Plus = 0;

        void add(int moveIndex) {
            total++;
            if (moveIndex == 0) idx0++;
            else if (moveIndex == 1) idx1++;
            else if (moveIndex == 2) idx2++;
            else if (moveIndex == 3) idx3++;
            else if (moveIndex >= 4 && moveIndex <= 7) idx4To7++;
            else idx8Plus++;
        }
    };

    struct MoveOrderingStats {
        uint64_t totalBetaCutoffs = 0;
        IndexBuckets allCutoffs;
        IndexBuckets depth1To2;
        IndexBuckets depth3To5;
        IndexBuckets depth6Plus;
        IndexBuckets quietCutoffs;

        uint64_t cutoffsTTMove = 0;
        uint64_t cutoffsCaptureOrPromotion = 0;
        uint64_t cutoffsQuiet = 0;

        uint64_t killerCandidatesEncountered = 0;
        uint64_t killerBetaCutoffs = 0;
        uint64_t killer1BetaCutoffs = 0;
        uint64_t killer2BetaCutoffs = 0;
    };

    struct LMRBucket {
        uint64_t reducedSearches = 0;
        uint64_t failLow = 0;
        uint64_t reSearches = 0;

        void record(bool triggeredReSearch) {
            reducedSearches++;
            if (triggeredReSearch)
                reSearches++;
            else
                failLow++;
        }
    };

    struct LMRStats {
        uint64_t totalReducedSearches = 0;
        uint64_t reducedFailLow = 0;
        uint64_t reducedTriggeredReSearch = 0;

        uint64_t reSearchFailLow = 0;
        uint64_t reSearchPV = 0;
        uint64_t reSearchBetaCutoff = 0;

        uint64_t reduction1Attempts = 0;
        uint64_t reduction1FailLow = 0;
        uint64_t reduction1ReSearch = 0;

        uint64_t reduction2Attempts = 0;
        uint64_t reduction2FailLow = 0;
        uint64_t reduction2ReSearch = 0;

        LMRBucket idx1;
        LMRBucket idx2;
        LMRBucket idx3;
        LMRBucket idx4To7;
        LMRBucket idx8Plus;

        LMRBucket depth3;
        LMRBucket depth4To5;
        LMRBucket depth6To8;
        LMRBucket depth9Plus;

        LMRBucket moveQuiet;
        LMRBucket moveLosingCapture;
        LMRBucket moveKillerQuiet;

        // Dedicated tracking for move indices 4 to 7
        LMRBucket idx4To7_depth3To4;
        LMRBucket idx4To7_depth5;
        LMRBucket idx4To7_depth6;
        LMRBucket idx4To7_depth7To8;
        LMRBucket idx4To7_depth9Plus;

        uint64_t idx4To7_reSearchTotal = 0;
        uint64_t idx4To7_reSearchFailLow = 0;
        uint64_t idx4To7_reSearchPV = 0;
        uint64_t idx4To7_reSearchBetaCutoff = 0;

        LMRBucket idx4To7_quiet;
        LMRBucket idx4To7_killer;
        LMRBucket idx4To7_losingCapture;
    };

    struct FutilityBucket {
        uint64_t total = 0;
        uint64_t failLow = 0;
        uint64_t pv = 0;
        uint64_t cutoff = 0;

        void record(int val, int alpha, int beta) {
            total++;
            if (val <= alpha) failLow++;
            else if (val < beta) pv++;
            else cutoff++;
        }
    };

    struct FutilityStats {
        uint64_t totalCandidates = 0;
        uint64_t candidatesFailLow = 0;
        uint64_t candidatesPV = 0;
        uint64_t candidatesBetaCutoff = 0;

        FutilityBucket depth1;
        FutilityBucket depth2;

        FutilityBucket idx1;
        FutilityBucket idx2;
        FutilityBucket idx3;
        FutilityBucket idx4To7;
        FutilityBucket idx8Plus;

        FutilityBucket ordinaryQuiet;
        FutilityBucket killerQuiet;
        FutilityBucket quietGivingCheck;

        FutilityBucket gap0To149;
        FutilityBucket gap150To299;
        FutilityBucket gap300To499;
        FutilityBucket gap500Plus;
    };

    static MoveOrderingStats GetMoveOrderingStatsForTesting();
    static void ResetMoveOrderingStatsForTesting();
    static void PrintMoveOrderingStatsForTesting();

    static LMRStats GetLMRStatsForTesting();
    static void ResetLMRStatsForTesting();
    static void PrintLMRStatsForTesting();

    static FutilityStats GetFutilityStatsForTesting();
    static void ResetFutilityStatsForTesting();
    static void PrintFutilityStatsForTesting();

    struct NullMoveBucket {
        uint64_t attempts = 0;
        uint64_t failLow = 0;
        uint64_t cutoffs = 0;
        uint64_t verifiedCutoffs = 0;
        uint64_t falseCutoffs = 0;

        void recordAttempt(bool isCutoff, bool isVerified) {
            attempts++;
            if (isCutoff) {
                cutoffs++;
                if (isVerified) verifiedCutoffs++;
                else falseCutoffs++;
            } else {
                failLow++;
            }
        }
    };

    struct NullMoveStats {
        uint64_t totalAttempts = 0;
        uint64_t totalFailLow = 0;
        uint64_t totalCutoffs = 0;
        uint64_t totalVerifiedCutoffs = 0;
        uint64_t totalFalseCutoffs = 0;

        NullMoveBucket depth4;
        NullMoveBucket depth5To6;
        NullMoveBucket depth7To8;
        NullMoveBucket depth9Plus;

        NullMoveBucket matQueen;
        NullMoveBucket matRookNoQueen;
        NullMoveBucket matMinorsOnly;
        NullMoveBucket matSinglePiece;
        NullMoveBucket matMultiPiece;

        NullMoveBucket margin0To49;
        NullMoveBucket margin50To149;
        NullMoveBucket margin150To299;
        NullMoveBucket margin300Plus;

        NullMoveBucket r3;
        NullMoveBucket r4;
    };

    static NullMoveStats GetNullMoveStatsForTesting();
    static void ResetNullMoveStatsForTesting();
    static void PrintNullMoveStatsForTesting();
#endif

private:
    static int moveOrderingDepth[20];
    static void NullMovePruning(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool mAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, MovePrintValue mPValue);
    static double NullMoveReduction(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool mateSearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch);
    static MovePrintValue* StartQSearch(bool isPVNode, int alpha, int beta, Move& prevMove, int depthGone, Move& move1, Move& move2, Move& move3, Board& board4, bool nullWindowSearch, bool previousMoveWasCheck);
};

#endif
