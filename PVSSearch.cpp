#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "PVSSearch.h"
#include "BoardLogic.h"
#include "EvaluationLogic.h"
#include "QSearcher.h"
#include "MoveLogic.h"
#include "UCI.h"
#include "GameLogic.h"
#include "ChessStringManipulation.h"
#include "MissingInfoAboutPrevStateFromMove.h"
#include "RepetitionHistory.h"
#include "TranspositionTable.h"
#include <iostream>
#include <algorithm>

int PVSSearch::moveOrderingDepth[20] = {
    1,
    2,
    2,
    3,
    3,
    4,
    4,
    5,
    5,
    6,
    6,
    7,
    7,
    8,
    8,
    9,
    9,
    10};

PVSSearch::KillerMove PVSSearch::killers[PVSSearch::MaxKillerPly][2] = {};

void PVSSearch::ResetKillers()
{
    for (int i = 0; i < MaxKillerPly; ++i)
    {
        killers[i][0] = KillerMove{};
        killers[i][1] = KillerMove{};
    }
}

void PVSSearch::RecordKiller(int ply, const Move &move)
{
    if (ply < 0 || ply >= MaxKillerPly)
        return;
    if (move.endPiece > 0 || move.promotionPiece > 0)
        return;

    KillerMove km{move.beginPlace, move.endPlace, move.promotionPiece};
    if (killers[ply][0] != move)
    {
        killers[ply][1] = killers[ply][0];
        killers[ply][0] = km;
    }
}

#if HOWL_CORRECTNESS_TESTING
namespace
{
    int g_futilityPruningSkippedQuietMoves = 0;
    PVSSearch::MoveOrderingStats g_moveOrderingStats;

    void RecordMoveOrderingCutoff(int moveIndex, bool isTTMove, const Move &move, int depth, int ply)
    {
        g_moveOrderingStats.totalBetaCutoffs++;
        g_moveOrderingStats.allCutoffs.add(moveIndex);

        if (depth <= 2)
            g_moveOrderingStats.depth1To2.add(moveIndex);
        else if (depth <= 5)
            g_moveOrderingStats.depth3To5.add(moveIndex);
        else
            g_moveOrderingStats.depth6Plus.add(moveIndex);

        if (isTTMove)
        {
            g_moveOrderingStats.cutoffsTTMove++;
        }
        else if (move.endPiece > 0 || move.promotionPiece > 0)
        {
            g_moveOrderingStats.cutoffsCaptureOrPromotion++;
        }
        else
        {
            g_moveOrderingStats.cutoffsQuiet++;
            g_moveOrderingStats.quietCutoffs.add(moveIndex);
        }

        if (ply >= 0 && ply < PVSSearch::MaxKillerPly && move.endPiece == 0 && move.promotionPiece <= 0)
        {
            if (PVSSearch::killers[ply][0] == move)
            {
                g_moveOrderingStats.killerBetaCutoffs++;
                g_moveOrderingStats.killer1BetaCutoffs++;
            }
            else if (PVSSearch::killers[ply][1] == move)
            {
                g_moveOrderingStats.killerBetaCutoffs++;
                g_moveOrderingStats.killer2BetaCutoffs++;
            }
        }
    }

    void PrintBucketGroup(const char *title, const PVSSearch::IndexBuckets &b)
    {
        std::cout << "\n[" << title << "]\n";
        std::cout << "  Total cutoffs: " << b.total << "\n";
        auto printRow = [b](const char *label, uint64_t count) {
            double pct = (b.total > 0) ? (100.0 * count / static_cast<double>(b.total)) : 0.0;
            char buf[128];
            snprintf(buf, sizeof(buf), "  %-12s %8lu  (%6.2f%%)\n", label, (unsigned long)count, pct);
            std::cout << buf;
        };
        printRow("Index 0:", b.idx0);
        printRow("Index 1:", b.idx1);
        printRow("Index 2:", b.idx2);
        printRow("Index 3:", b.idx3);
        printRow("Indices 4-7:", b.idx4To7);
        printRow("Index 8+:", b.idx8Plus);
    }
}

int PVSSearch::FutilityPruningSkippedQuietMovesForTesting()
{
    return g_futilityPruningSkippedQuietMoves;
}

void PVSSearch::ResetFutilityPruningSkippedQuietMovesForTesting()
{
    g_futilityPruningSkippedQuietMoves = 0;
}

PVSSearch::MoveOrderingStats PVSSearch::GetMoveOrderingStatsForTesting()
{
    return g_moveOrderingStats;
}

void PVSSearch::ResetMoveOrderingStatsForTesting()
{
    g_moveOrderingStats = MoveOrderingStats{};
}

void PVSSearch::PrintMoveOrderingStatsForTesting()
{
    std::cout << "=== Move Ordering Cutoff Stats (Recursive PVS) ===\n";
    std::cout << "Overall total beta cutoffs: " << g_moveOrderingStats.totalBetaCutoffs << "\n";

    PrintBucketGroup("All Depths (Overall)", g_moveOrderingStats.allCutoffs);
    PrintBucketGroup("Depth 1 to 2", g_moveOrderingStats.depth1To2);
    PrintBucketGroup("Depth 3 to 5", g_moveOrderingStats.depth3To5);
    PrintBucketGroup("Depth 6+", g_moveOrderingStats.depth6Plus);
    PrintBucketGroup("Quiet Moves Only", g_moveOrderingStats.quietCutoffs);

    std::cout << "\n[Move Category Breakdown]\n";
    auto printCat = [total = g_moveOrderingStats.totalBetaCutoffs](const char *label, uint64_t count) {
        double pct = (total > 0) ? (100.0 * count / static_cast<double>(total)) : 0.0;
        char buf[128];
        snprintf(buf, sizeof(buf), "  %-24s %8lu  (%6.2f%%)\n", label, (unsigned long)count, pct);
        std::cout << buf;
    };
    printCat("TT move:", g_moveOrderingStats.cutoffsTTMove);
    printCat("Capture or promotion:", g_moveOrderingStats.cutoffsCaptureOrPromotion);
    printCat("Quiet:", g_moveOrderingStats.cutoffsQuiet);

    std::cout << "\n[Killer Move Stats]\n";
    std::cout << "  Killer candidates encountered: " << g_moveOrderingStats.killerCandidatesEncountered << "\n";
    printCat("Killer beta cutoffs:", g_moveOrderingStats.killerBetaCutoffs);
    printCat("Killer 1 cutoffs:", g_moveOrderingStats.killer1BetaCutoffs);
    printCat("Killer 2 cutoffs:", g_moveOrderingStats.killer2BetaCutoffs);
}

PVSSearch::LMRStats g_lmrStats;

void RecordLMRSearch(int moveIndex, int depth, int ply, const Move &move, int reductionAmount, bool triggeredReSearch)
{
    g_lmrStats.totalReducedSearches++;
    if (triggeredReSearch)
        g_lmrStats.reducedTriggeredReSearch++;
    else
        g_lmrStats.reducedFailLow++;

    if (reductionAmount == 2)
    {
        g_lmrStats.reduction2Attempts++;
        if (triggeredReSearch)
            g_lmrStats.reduction2ReSearch++;
        else
            g_lmrStats.reduction2FailLow++;
    }
    else
    {
        g_lmrStats.reduction1Attempts++;
        if (triggeredReSearch)
            g_lmrStats.reduction1ReSearch++;
        else
            g_lmrStats.reduction1FailLow++;
    }

    if (moveIndex == 1) g_lmrStats.idx1.record(triggeredReSearch);
    else if (moveIndex == 2) g_lmrStats.idx2.record(triggeredReSearch);
    else if (moveIndex == 3) g_lmrStats.idx3.record(triggeredReSearch);
    else if (moveIndex >= 4 && moveIndex <= 7) g_lmrStats.idx4To7.record(triggeredReSearch);
    else g_lmrStats.idx8Plus.record(triggeredReSearch);

    if (depth == 3) g_lmrStats.depth3.record(triggeredReSearch);
    else if (depth >= 4 && depth <= 5) g_lmrStats.depth4To5.record(triggeredReSearch);
    else if (depth >= 6 && depth <= 8) g_lmrStats.depth6To8.record(triggeredReSearch);
    else g_lmrStats.depth9Plus.record(triggeredReSearch);

    bool isKiller = (ply >= 0 && ply < PVSSearch::MaxKillerPly && (PVSSearch::killers[ply][0] == move || PVSSearch::killers[ply][1] == move));
    if (move.endPiece > 0)
    {
        g_lmrStats.moveLosingCapture.record(triggeredReSearch);
    }
    else if (isKiller)
    {
        g_lmrStats.moveKillerQuiet.record(triggeredReSearch);
    }
    else
    {
        g_lmrStats.moveQuiet.record(triggeredReSearch);
    }

    // Indices 4 to 7 detailed tracking
    if (moveIndex >= 4 && moveIndex <= 7)
    {
        if (depth >= 3 && depth <= 4) g_lmrStats.idx4To7_depth3To4.record(triggeredReSearch);
        else if (depth == 5) g_lmrStats.idx4To7_depth5.record(triggeredReSearch);
        else if (depth == 6) g_lmrStats.idx4To7_depth6.record(triggeredReSearch);
        else if (depth >= 7 && depth <= 8) g_lmrStats.idx4To7_depth7To8.record(triggeredReSearch);
        else if (depth >= 9) g_lmrStats.idx4To7_depth9Plus.record(triggeredReSearch);

        if (move.endPiece > 0)
            g_lmrStats.idx4To7_losingCapture.record(triggeredReSearch);
        else if (isKiller)
            g_lmrStats.idx4To7_killer.record(triggeredReSearch);
        else
            g_lmrStats.idx4To7_quiet.record(triggeredReSearch);
    }
}

void RecordLMRReSearchResult(int moveIndex, int finalVal, int origAlpha, int origBeta)
{
    if (finalVal <= origAlpha)
        g_lmrStats.reSearchFailLow++;
    else if (finalVal < origBeta)
        g_lmrStats.reSearchPV++;
    else
        g_lmrStats.reSearchBetaCutoff++;

    if (moveIndex >= 4 && moveIndex <= 7)
    {
        g_lmrStats.idx4To7_reSearchTotal++;
        if (finalVal <= origAlpha)
            g_lmrStats.idx4To7_reSearchFailLow++;
        else if (finalVal < origBeta)
            g_lmrStats.idx4To7_reSearchPV++;
        else
            g_lmrStats.idx4To7_reSearchBetaCutoff++;
    }
}

void PrintLMRBucketRow(const char *label, const PVSSearch::LMRBucket &b)
{
    double failLowPct = (b.reducedSearches > 0) ? (100.0 * b.failLow / static_cast<double>(b.reducedSearches)) : 0.0;
    double reSearchPct = (b.reducedSearches > 0) ? (100.0 * b.reSearches / static_cast<double>(b.reducedSearches)) : 0.0;
    char buf[256];
    snprintf(buf, sizeof(buf), "  %-20s Total: %7lu | Fail-low: %7lu (%6.2f%%) | Re-search: %7lu (%6.2f%%)\n",
             label, (unsigned long)b.reducedSearches, (unsigned long)b.failLow, failLowPct, (unsigned long)b.reSearches, reSearchPct);
    std::cout << buf;
}

PVSSearch::LMRStats PVSSearch::GetLMRStatsForTesting()
{
    return g_lmrStats;
}

void PVSSearch::ResetLMRStatsForTesting()
{
    g_lmrStats = LMRStats{};
}

void PVSSearch::PrintLMRStatsForTesting()
{
    std::cout << "=== Late Move Reduction (LMR) Stats (Recursive PVS) ===\n";
    std::cout << "Total LMR reduced searches: " << g_lmrStats.totalReducedSearches << "\n";

    double totalFailLowPct = (g_lmrStats.totalReducedSearches > 0) ? (100.0 * g_lmrStats.reducedFailLow / static_cast<double>(g_lmrStats.totalReducedSearches)) : 0.0;
    double totalReSearchPct = (g_lmrStats.totalReducedSearches > 0) ? (100.0 * g_lmrStats.reducedTriggeredReSearch / static_cast<double>(g_lmrStats.totalReducedSearches)) : 0.0;

    std::cout << "  Immediate fail-low (no re-search): " << g_lmrStats.reducedFailLow << " (" << totalFailLowPct << "%)\n";
    std::cout << "  Triggered full-depth re-search:    " << g_lmrStats.reducedTriggeredReSearch << " (" << totalReSearchPct << "%)\n";

    std::cout << "\n[Reduction Depth Breakdown]\n";
    auto printRed = [](const char *label, uint64_t attempts, uint64_t failLow, uint64_t reSearch) {
        double flPct = (attempts > 0) ? (100.0 * failLow / static_cast<double>(attempts)) : 0.0;
        double rsPct = (attempts > 0) ? (100.0 * reSearch / static_cast<double>(attempts)) : 0.0;
        char buf[256];
        snprintf(buf, sizeof(buf), "  %-16s Attempts: %7lu | Fail-low: %7lu (%6.2f%%) | Re-search: %7lu (%6.2f%%)\n",
                 label, (unsigned long)attempts, (unsigned long)failLow, flPct, (unsigned long)reSearch, rsPct);
        std::cout << buf;
    };
    printRed("Reduction = 1:", g_lmrStats.reduction1Attempts, g_lmrStats.reduction1FailLow, g_lmrStats.reduction1ReSearch);
    printRed("Reduction = 2:", g_lmrStats.reduction2Attempts, g_lmrStats.reduction2FailLow, g_lmrStats.reduction2ReSearch);

    std::cout << "\n[Full-Depth Re-Search Outcome Breakdown]\n";
    std::cout << "  Total full-depth re-searches: " << g_lmrStats.reducedTriggeredReSearch << "\n";
    auto printOutcome = [total = g_lmrStats.reducedTriggeredReSearch](const char *label, uint64_t count) {
        double pct = (total > 0) ? (100.0 * count / static_cast<double>(total)) : 0.0;
        char buf[128];
        snprintf(buf, sizeof(buf), "  %-32s %7lu  (%6.2f%%)\n", label, (unsigned long)count, pct);
        std::cout << buf;
    };
    printOutcome("Final value <= original alpha:", g_lmrStats.reSearchFailLow);
    printOutcome("Final value > alpha & < beta:", g_lmrStats.reSearchPV);
    printOutcome("Final value >= beta (cutoff):", g_lmrStats.reSearchBetaCutoff);

    std::cout << "\n[By Move Index]\n";
    PrintLMRBucketRow("Index 1:", g_lmrStats.idx1);
    PrintLMRBucketRow("Index 2:", g_lmrStats.idx2);
    PrintLMRBucketRow("Index 3:", g_lmrStats.idx3);
    PrintLMRBucketRow("Indices 4-7:", g_lmrStats.idx4To7);
    PrintLMRBucketRow("Index 8+:", g_lmrStats.idx8Plus);

    std::cout << "\n[By Remaining Depth]\n";
    PrintLMRBucketRow("Depth 3:", g_lmrStats.depth3);
    PrintLMRBucketRow("Depth 4-5:", g_lmrStats.depth4To5);
    PrintLMRBucketRow("Depth 6-8:", g_lmrStats.depth6To8);
    PrintLMRBucketRow("Depth 9+:", g_lmrStats.depth9Plus);

    std::cout << "\n[By Move Type]\n";
    PrintLMRBucketRow("Quiet moves:", g_lmrStats.moveQuiet);
    PrintLMRBucketRow("Losing captures:", g_lmrStats.moveLosingCapture);
    PrintLMRBucketRow("Killer quiet moves:", g_lmrStats.moveKillerQuiet);

    std::cout << "\n=======================================================\n";
    std::cout << "=== Detailed Investigation: Move Indices 4 to 7 ===\n";
    std::cout << "=======================================================\n";
    std::cout << "\n[Indices 4-7 By Depth]\n";
    PrintLMRBucketRow("Depth 3-4:", g_lmrStats.idx4To7_depth3To4);
    PrintLMRBucketRow("Depth 5:",   g_lmrStats.idx4To7_depth5);
    PrintLMRBucketRow("Depth 6:",   g_lmrStats.idx4To7_depth6);
    PrintLMRBucketRow("Depth 7-8:", g_lmrStats.idx4To7_depth7To8);
    PrintLMRBucketRow("Depth 9+:",  g_lmrStats.idx4To7_depth9Plus);

    std::cout << "\n[Indices 4-7 Re-Search Outcomes]\n";
    std::cout << "  Total full-depth re-searches: " << g_lmrStats.idx4To7_reSearchTotal << "\n";
    auto printIdx4To7Outcome = [total = g_lmrStats.idx4To7_reSearchTotal](const char *label, uint64_t count) {
        double pct = (total > 0) ? (100.0 * count / static_cast<double>(total)) : 0.0;
        char buf[128];
        snprintf(buf, sizeof(buf), "  %-32s %7lu  (%6.2f%%)\n", label, (unsigned long)count, pct);
        std::cout << buf;
    };
    printIdx4To7Outcome("Final value <= original alpha:", g_lmrStats.idx4To7_reSearchFailLow);
    printIdx4To7Outcome("Final value > alpha & < beta:", g_lmrStats.idx4To7_reSearchPV);
    printIdx4To7Outcome("Final value >= beta (cutoff):", g_lmrStats.idx4To7_reSearchBetaCutoff);

    std::cout << "\n[Indices 4-7 By Move Type]\n";
    PrintLMRBucketRow("Ordinary quiet:",   g_lmrStats.idx4To7_quiet);
    PrintLMRBucketRow("Killer quiet:",     g_lmrStats.idx4To7_killer);
    PrintLMRBucketRow("Losing capture:",   g_lmrStats.idx4To7_losingCapture);
}

PVSSearch::FutilityStats g_futilityStats;

void RecordFutilityCandidate(int moveIndex, int depth, int ply, const Move &move, int staticEval, int origAlpha, int origBeta, int actualValue, bool givesCheck)
{
    g_futilityStats.totalCandidates++;
    if (actualValue <= origAlpha)
        g_futilityStats.candidatesFailLow++;
    else if (actualValue < origBeta)
        g_futilityStats.candidatesPV++;
    else
        g_futilityStats.candidatesBetaCutoff++;

    if (depth == 1) g_futilityStats.depth1.record(actualValue, origAlpha, origBeta);
    else if (depth == 2) g_futilityStats.depth2.record(actualValue, origAlpha, origBeta);

    if (moveIndex == 1) g_futilityStats.idx1.record(actualValue, origAlpha, origBeta);
    else if (moveIndex == 2) g_futilityStats.idx2.record(actualValue, origAlpha, origBeta);
    else if (moveIndex == 3) g_futilityStats.idx3.record(actualValue, origAlpha, origBeta);
    else if (moveIndex >= 4 && moveIndex <= 7) g_futilityStats.idx4To7.record(actualValue, origAlpha, origBeta);
    else g_futilityStats.idx8Plus.record(actualValue, origAlpha, origBeta);

    bool isKiller = (ply >= 0 && ply < PVSSearch::MaxKillerPly && (PVSSearch::killers[ply][0] == move || PVSSearch::killers[ply][1] == move));
    if (givesCheck)
        g_futilityStats.quietGivingCheck.record(actualValue, origAlpha, origBeta);
    else if (isKiller)
        g_futilityStats.killerQuiet.record(actualValue, origAlpha, origBeta);
    else
        g_futilityStats.ordinaryQuiet.record(actualValue, origAlpha, origBeta);

    int staticGap = origAlpha - staticEval;
    if (staticGap < 150) g_futilityStats.gap0To149.record(actualValue, origAlpha, origBeta);
    else if (staticGap < 300) g_futilityStats.gap150To299.record(actualValue, origAlpha, origBeta);
    else if (staticGap < 500) g_futilityStats.gap300To499.record(actualValue, origAlpha, origBeta);
    else g_futilityStats.gap500Plus.record(actualValue, origAlpha, origBeta);
}

void PrintFutilityBucketRow(const char *label, const PVSSearch::FutilityBucket &b)
{
    double flPct = (b.total > 0) ? (100.0 * b.failLow / static_cast<double>(b.total)) : 0.0;
    double pvPct = (b.total > 0) ? (100.0 * b.pv / static_cast<double>(b.total)) : 0.0;
    double cutPct = (b.total > 0) ? (100.0 * b.cutoff / static_cast<double>(b.total)) : 0.0;
    char buf[256];
    snprintf(buf, sizeof(buf), "  %-24s Total: %7lu | <=alpha: %7lu (%5.1f%%) | PV: %5lu (%4.1f%%) | >=beta: %5lu (%4.1f%%)\n",
             label, (unsigned long)b.total, (unsigned long)b.failLow, flPct, (unsigned long)b.pv, pvPct, (unsigned long)b.cutoff, cutPct);
    std::cout << buf;
}

PVSSearch::FutilityStats PVSSearch::GetFutilityStatsForTesting()
{
    return g_futilityStats;
}

void PVSSearch::ResetFutilityStatsForTesting()
{
    g_futilityStats = FutilityStats{};
}

void PVSSearch::PrintFutilityStatsForTesting()
{
    std::cout << "=== Futility Pruning Safety Stats (Recursive PVS) ===\n";
    std::cout << "Total futility candidates evaluated: " << g_futilityStats.totalCandidates << "\n";

    double flPct = (g_futilityStats.totalCandidates > 0) ? (100.0 * g_futilityStats.candidatesFailLow / static_cast<double>(g_futilityStats.totalCandidates)) : 0.0;
    double pvPct = (g_futilityStats.totalCandidates > 0) ? (100.0 * g_futilityStats.candidatesPV / static_cast<double>(g_futilityStats.totalCandidates)) : 0.0;
    double cutPct = (g_futilityStats.totalCandidates > 0) ? (100.0 * g_futilityStats.candidatesBetaCutoff / static_cast<double>(g_futilityStats.totalCandidates)) : 0.0;

    char buf[256];
    snprintf(buf, sizeof(buf), "  Overall <= original alpha (safe to prune): %7lu (%5.1f%%)\n  Overall > alpha and < beta (would improve PV): %7lu (%5.1f%%)\n  Overall >= beta (would cause cutoff):       %7lu (%5.1f%%)\n",
             (unsigned long)g_futilityStats.candidatesFailLow, flPct, (unsigned long)g_futilityStats.candidatesPV, pvPct, (unsigned long)g_futilityStats.candidatesBetaCutoff, cutPct);
    std::cout << buf;

    std::cout << "\n[By Remaining Depth]\n";
    PrintFutilityBucketRow("Depth 1:", g_futilityStats.depth1);
    PrintFutilityBucketRow("Depth 2:", g_futilityStats.depth2);

    std::cout << "\n[By Move Index]\n";
    PrintFutilityBucketRow("Index 1:", g_futilityStats.idx1);
    PrintFutilityBucketRow("Index 2:", g_futilityStats.idx2);
    PrintFutilityBucketRow("Index 3:", g_futilityStats.idx3);
    PrintFutilityBucketRow("Indices 4-7:", g_futilityStats.idx4To7);
    PrintFutilityBucketRow("Index 8+:", g_futilityStats.idx8Plus);

    std::cout << "\n[By Move Type]\n";
    PrintFutilityBucketRow("Ordinary quiet:", g_futilityStats.ordinaryQuiet);
    PrintFutilityBucketRow("Killer quiet:", g_futilityStats.killerQuiet);
    PrintFutilityBucketRow("Quiet giving check:", g_futilityStats.quietGivingCheck);

    std::cout << "\n[By Static Evaluation Gap (alpha - staticEval)]\n";
    PrintFutilityBucketRow("Gap 0 to 149:", g_futilityStats.gap0To149);
    PrintFutilityBucketRow("Gap 150 to 299:", g_futilityStats.gap150To299);
    PrintFutilityBucketRow("Gap 300 to 499:", g_futilityStats.gap300To499);
    PrintFutilityBucketRow("Gap 500+:", g_futilityStats.gap500Plus);
}

PVSSearch::NullMoveStats g_nullMoveStats;

void RecordNullMoveAttempt(int depth, int R, int staticMargin, const Board &board, int turn, bool isCutoff, bool isVerified)
{
    g_nullMoveStats.totalAttempts++;
    if (isCutoff) {
        g_nullMoveStats.totalCutoffs++;
        if (isVerified) g_nullMoveStats.totalVerifiedCutoffs++;
        else g_nullMoveStats.totalFalseCutoffs++;
    } else {
        g_nullMoveStats.totalFailLow++;
    }

    // Depth
    if (depth == 4) g_nullMoveStats.depth4.recordAttempt(isCutoff, isVerified);
    else if (depth >= 5 && depth <= 6) g_nullMoveStats.depth5To6.recordAttempt(isCutoff, isVerified);
    else if (depth >= 7 && depth <= 8) g_nullMoveStats.depth7To8.recordAttempt(isCutoff, isVerified);
    else if (depth >= 9) g_nullMoveStats.depth9Plus.recordAttempt(isCutoff, isVerified);

    // Reduction R
    if (R == 3) g_nullMoveStats.r3.recordAttempt(isCutoff, isVerified);
    else if (R == 4) g_nullMoveStats.r4.recordAttempt(isCutoff, isVerified);

    // Margin
    if (staticMargin < 50) g_nullMoveStats.margin0To49.recordAttempt(isCutoff, isVerified);
    else if (staticMargin < 150) g_nullMoveStats.margin50To149.recordAttempt(isCutoff, isVerified);
    else if (staticMargin < 300) g_nullMoveStats.margin150To299.recordAttempt(isCutoff, isVerified);
    else g_nullMoveStats.margin300Plus.recordAttempt(isCutoff, isVerified);

    // Material Class
    int knights = board.pieces[turn * 8 + 2].size();
    int bishops = board.pieces[turn * 8 + 3].size();
    int rooks = board.pieces[turn * 8 + 4].size();
    int queens = board.pieces[turn * 8 + 5].size();
    int totalNonPawnCount = knights + bishops + rooks + queens;

    if (queens > 0) g_nullMoveStats.matQueen.recordAttempt(isCutoff, isVerified);
    else if (rooks > 0) g_nullMoveStats.matRookNoQueen.recordAttempt(isCutoff, isVerified);
    else g_nullMoveStats.matMinorsOnly.recordAttempt(isCutoff, isVerified);

    if (totalNonPawnCount == 1) g_nullMoveStats.matSinglePiece.recordAttempt(isCutoff, isVerified);
    else if (totalNonPawnCount >= 2) g_nullMoveStats.matMultiPiece.recordAttempt(isCutoff, isVerified);
}

void PrintNullBucketRow(const char *label, const PVSSearch::NullMoveBucket &b)
{
    double flPct = (b.attempts > 0) ? (100.0 * b.failLow / static_cast<double>(b.attempts)) : 0.0;
    double cutPct = (b.attempts > 0) ? (100.0 * b.cutoffs / static_cast<double>(b.attempts)) : 0.0;
    double verPct = (b.cutoffs > 0) ? (100.0 * b.verifiedCutoffs / static_cast<double>(b.cutoffs)) : 0.0;
    double falsePct = (b.cutoffs > 0) ? (100.0 * b.falseCutoffs / static_cast<double>(b.cutoffs)) : 0.0;

    char buf[256];
    snprintf(buf, sizeof(buf), "  %-24s Attempts: %6lu | Fail-low: %6lu (%5.1f%%) | Cutoffs: %6lu (%5.1f%%) | Confirmed: %6lu (%5.1f%%) | False: %4lu (%4.1f%%)\n",
             label, (unsigned long)b.attempts, (unsigned long)b.failLow, flPct, (unsigned long)b.cutoffs, cutPct, (unsigned long)b.verifiedCutoffs, verPct, (unsigned long)b.falseCutoffs, falsePct);
    std::cout << buf;
}

PVSSearch::NullMoveStats PVSSearch::GetNullMoveStatsForTesting()
{
    return g_nullMoveStats;
}

void PVSSearch::ResetNullMoveStatsForTesting()
{
    g_nullMoveStats = NullMoveStats{};
}

void PVSSearch::PrintNullMoveStatsForTesting()
{
    std::cout << "=== Null Move Pruning (NMP) Safety Stats (Recursive PVS) ===\n";
    std::cout << "Total null move attempts: " << g_nullMoveStats.totalAttempts << "\n";

    double flPct = (g_nullMoveStats.totalAttempts > 0) ? (100.0 * g_nullMoveStats.totalFailLow / static_cast<double>(g_nullMoveStats.totalAttempts)) : 0.0;
    double cutPct = (g_nullMoveStats.totalAttempts > 0) ? (100.0 * g_nullMoveStats.totalCutoffs / static_cast<double>(g_nullMoveStats.totalAttempts)) : 0.0;
    double verPct = (g_nullMoveStats.totalCutoffs > 0) ? (100.0 * g_nullMoveStats.totalVerifiedCutoffs / static_cast<double>(g_nullMoveStats.totalCutoffs)) : 0.0;
    double falsePct = (g_nullMoveStats.totalCutoffs > 0) ? (100.0 * g_nullMoveStats.totalFalseCutoffs / static_cast<double>(g_nullMoveStats.totalCutoffs)) : 0.0;

    char buf[256];
    snprintf(buf, sizeof(buf), "  Null move fail-lows:           %6lu (%5.1f%%)\n  Null move cutoffs:             %6lu (%5.1f%%)\n  Confirmed genuine cutoffs:     %6lu (%5.1f%% of cutoffs)\n  False cutoffs (failed verify): %6lu (%5.1f%% of cutoffs)\n",
             (unsigned long)g_nullMoveStats.totalFailLow, flPct, (unsigned long)g_nullMoveStats.totalCutoffs, cutPct, (unsigned long)g_nullMoveStats.totalVerifiedCutoffs, verPct, (unsigned long)g_nullMoveStats.totalFalseCutoffs, falsePct);
    std::cout << buf;

    std::cout << "\n[By Remaining Depth]\n";
    PrintNullBucketRow("Depth 4:", g_nullMoveStats.depth4);
    PrintNullBucketRow("Depth 5-6:", g_nullMoveStats.depth5To6);
    PrintNullBucketRow("Depth 7-8:", g_nullMoveStats.depth7To8);
    PrintNullBucketRow("Depth 9+:", g_nullMoveStats.depth9Plus);

    std::cout << "\n[By Reduction R]\n";
    PrintNullBucketRow("R = 3:", g_nullMoveStats.r3);
    PrintNullBucketRow("R = 4:", g_nullMoveStats.r4);

    std::cout << "\n[By Side-to-Move Material Class]\n";
    PrintNullBucketRow("Queen present:", g_nullMoveStats.matQueen);
    PrintNullBucketRow("Rook (no queen):", g_nullMoveStats.matRookNoQueen);
    PrintNullBucketRow("Minor pieces only:", g_nullMoveStats.matMinorsOnly);
    PrintNullBucketRow("Single non-pawn piece:", g_nullMoveStats.matSinglePiece);
    PrintNullBucketRow(">=2 non-pawn pieces:", g_nullMoveStats.matMultiPiece);

    std::cout << "\n[By Static Margin (staticEval - beta)]\n";
    PrintNullBucketRow("Margin 0 to 49:", g_nullMoveStats.margin0To49);
    PrintNullBucketRow("Margin 50 to 149:", g_nullMoveStats.margin50To149);
    PrintNullBucketRow("Margin 150 to 299:", g_nullMoveStats.margin150To299);
    PrintNullBucketRow("Margin 300+:", g_nullMoveStats.margin300Plus);
}
#endif

MovePrintValue *PVSSearch::PVS(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch)
{
    Move *SelectedMove = nullptr;
    Board *boardCopy = nullptr;

    MovePrintValue *retValue = new MovePrintValue();
    retValue->printString = "";

    MovePrintValue *MPValue = new MovePrintValue();
    MPValue->printString = "";

    int turn;
    if (!board4.sideToMove)
    {
        turn = 0;
    }
    else
    {
        turn = 1;
    }

    const int origAlpha = alpha;
    if (depth == 0)
    {
        delete retValue;
        retValue = nullptr;
        delete MPValue;
        MPValue = nullptr;
        return StartQSearch(isPVNode, alpha, beta, prevMove, depthGone, move1, move2, move3, board4, nullWindowSearch, previousMoveWasCheck);
    }

    if (BoardLogic::UnderAttack(board4, board4.pieces[(1 - turn) * 8 + 6].front(), board4.sideToMove))
    {
        retValue->value = 160000;
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    }
    if (depth == 1 && BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
    {
        previousMoveWasCheck = true;
    }
    if (isNullMoveAllowed && !isPVNode && depth >= 4)
    {
        bool inCheck = BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove);
        if (!inCheck)
        {
            bool hasNonPawn = (board4.pieces[turn * 8 + 2].size() > 0 ||
                               board4.pieces[turn * 8 + 3].size() > 0 ||
                               board4.pieces[turn * 8 + 4].size() > 0 ||
                               board4.pieces[turn * 8 + 5].size() > 0);
            if (hasNonPawn)
            {
                int staticEval = EvaluationLogic::Evaluate(board4);
                if (staticEval >= beta)
                {
                    int R = (depth > 6) ? 4 : 3;
                    Move nullMove{};
                    nullMove.promotionPiece = -1;
                    MissingInfoAboutPrevStateFromMove undoInfo(board4);
                    GameLogic::DoMove(board4, nullMove, prevMove, depthGone, depthGone);

                    std::unique_ptr<MovePrintValue> nullRes(PVS(false, -beta, -beta + 1, depth - 1 - R, nullMove, move2, move3, prevMove, board4, MAtESearch, false, depthGone + 1, previousMoveWasCheck, true));
                    int nullScore = -nullRes->value;
                    if (nullScore > 159800 && nullScore != 160000)
                    {
                        nullScore--;
                    }
                    else if (nullScore < -159800 && nullScore != -160000)
                    {
                        nullScore++;
                    }

                    GameLogic::UndoMove(board4, nullMove, undoInfo);

#if HOWL_CORRECTNESS_TESTING
                    bool isCutoff = (nullScore >= beta);
                    bool isVerified = false;
                    if (isCutoff)
                    {
                        // In instrumentation mode only: perform normal verification search
                        // from the original position without null move to check if value >= beta
                        MovePrintValue *verMP = PVS(false, beta - 1, beta, depth, prevMove, move1, move2, move3, board4, MAtESearch, false, depthGone, previousMoveWasCheck, true);
                        int verVal = verMP->value;
                        if (verVal >= beta)
                        {
                            isVerified = true;
                        }
                        delete verMP;
                    }
                    RecordNullMoveAttempt(depth, R, staticEval - beta, board4, turn, isCutoff, isVerified);
#endif

                    if (nullScore >= beta)
                    {
                        retValue->value = (nullScore >= 159800) ? beta : nullScore;
                        retValue->printString = "null";
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                }
            }
        }
    }
    MoveList moveList = MoveLogic::MoveGenerator(board4, depth, depthGone);
    if constexpr (ProductionIGGEnabled)
    {
        IGG(isPVNode, alpha, beta, depth, prevMove, move1, move2, move3, board4, MAtESearch, isNullMoveAllowed, depthGone, previousMoveWasCheck, nullWindowSearch, moveList);
    }
    TTEntry ttEntry{};
    bool ttHit = TranspositionTable::Probe(board4.ZobristHashCode, ttEntry);
#if HOWL_CORRECTNESS_TESTING
    TTTelemetryStats &ttStats = TranspositionTable::TelemetryStats();
    ttStats.eligibleProbes++;
    TTTelemetryBucket *depthBucket = nullptr;
    if (depth <= 2) depthBucket = &ttStats.depth1To2;
    else if (depth <= 5) depthBucket = &ttStats.depth3To5;
    else if (depth <= 8) depthBucket = &ttStats.depth6To8;
    else depthBucket = &ttStats.depth9Plus;

    if (depthBucket) depthBucket->probes++;

    if (ttHit)
    {
        ttStats.hits++;
        if (depthBucket) depthBucket->hits++;

        if (ttEntry.depth >= depth) ttStats.hitsSufficientDepth++;
        else ttStats.hitsInsufficientDepth++;

        uint8_t baseFlag = TTBaseFlag(ttEntry.flag);
        if (baseFlag == TT_EXACT) ttStats.hitsExact++;
        else if (baseFlag == TT_LOWER_BOUND) ttStats.hitsLower++;
        else if (baseFlag == TT_UPPER_BOUND) ttStats.hitsUpper++;
    }
    else
    {
        ttStats.misses++;
    }
#endif
    if (TranspositionTable::CutoffsEnabled() && !isPVNode && ttHit && ttEntry.depth >= depth && TTFlagIsRigorous(ttEntry.flag))
    {
        if (ttEntry.flag == TT_EXACT)
        {
            TranspositionTable::RecordCutoff();
#if HOWL_CORRECTNESS_TESTING
            ttStats.cutoffsExact++;
            ttStats.totalCutoffs++;
            if (depthBucket) depthBucket->cutoffs++;
#endif
            retValue->value = ttEntry.score;
            deleteMoveList(moveList);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else if (ttEntry.flag == TT_LOWER_BOUND && ttEntry.score >= beta)
        {
            TranspositionTable::RecordCutoff();
#if HOWL_CORRECTNESS_TESTING
            ttStats.cutoffsLower++;
            ttStats.totalCutoffs++;
            if (depthBucket) depthBucket->cutoffs++;
#endif
            retValue->value = ttEntry.score;
            deleteMoveList(moveList);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else if (ttEntry.flag == TT_UPPER_BOUND && ttEntry.score <= alpha)
        {
            TranspositionTable::RecordCutoff();
#if HOWL_CORRECTNESS_TESTING
            ttStats.cutoffsUpper++;
            ttStats.totalCutoffs++;
            if (depthBucket) depthBucket->cutoffs++;
#endif
            retValue->value = ttEntry.score;
            deleteMoveList(moveList);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
    }
    if (depthGone >= 0 && depthGone < MaxKillerPly)
    {
        bool boosted = false;
        for (int i = 0; i < moveList.count; ++i)
        {
            Move *m = moveList.moves[i];
            if (m->endPiece == 0 && m->promotionPiece <= 0)
            {
                if (killers[depthGone][0] == *m || killers[depthGone][1] == *m)
                {
                    m->value = std::max(m->value, 30);
                    boosted = true;
#if HOWL_CORRECTNESS_TESTING
                    g_moveOrderingStats.killerCandidatesEncountered++;
#endif
                }
            }
        }
        if (boosted)
        {
            std::sort(moveList.moves, moveList.moves + moveList.count, [](const Move *a, const Move *b)
                      { return b->value < a->value; });
        }
    }
    bool hasTTMove = false;
    if (ttHit && ttEntry.bestMove != 0)
    {
        int ttFrom = TTMoveHelper::UnpackFrom(ttEntry.bestMove);
        int ttTo = TTMoveHelper::UnpackTo(ttEntry.bestMove);
        int ttPromo = TTMoveHelper::UnpackPromotion(ttEntry.bestMove);
#if HOWL_CORRECTNESS_TESTING
        ttStats.ttMoveFoundNoCutoff++;
        bool matchedGenerated = false;
#endif
        for (int i = 0; i < moveList.count; ++i)
        {
            Move *m = moveList.moves[i];
            if (m->beginPlace == ttFrom && m->endPlace == ttTo &&
                (ttPromo == 0 ? (m->promotionPiece <= 0) : (m->promotionPiece == ttPromo)))
            {
                hasTTMove = true;
#if HOWL_CORRECTNESS_TESTING
                matchedGenerated = true;
#endif
                TranspositionTable::RecordHitStats(true, i == 0);
                if (i != 0)
                {
                    std::swap(moveList.moves[0], moveList.moves[i]);
                }
                break;
            }
        }
#if HOWL_CORRECTNESS_TESTING
        if (matchedGenerated) ttStats.ttMoveMatchedLegal++;
        else ttStats.ttMoveMissingFromGenerated++;
#endif
    }
    int inCheck = -1;
    int staticEval = -200000;
    int bestMoveValue = -200000;
    std::string SelectedPV = "";
    int availMoves = 0;
    int futilityPrunedCount = 0;
    int unverifiedLMRCount = 0;
    {
        bool firstMove = true;
        for (int i = 0; i < moveList.count; ++i)
        {
            Move *move = moveList.moves[i];
            int LMRDepth = 0;
            bool wasResearchedAtFullDepth = false;
            if (Search::overAllIteration == 1 && move->beginPlace == 60 && move->endPlace == 53)
            {
                //std::cout << "here" << std::endl;
                // std::cout << "Object is still in use after 15 seconds." << std::endl;
                //  throw std::runtime_error("Object is still in use after 15 seconds.");
            }
            if (move->beginPlace == 60 && move->endPlace == 53 && depth == 1 && depthGone == 1)
            {
                std::cout << "here" << std::endl;
                // std::cout << "Object is still in use after 15 seconds." << std::endl;
                //  throw std::runtime_error("Object is still in use after 15 seconds.");
            }
            if (firstMove)
            {
                boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
                GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone);
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                {
                    bestMoveValue = 0;
                    SelectedMove = move;
                    move->value = 0;
                }
                else
                {
                    bool tempPVNode = false;
                    if (isPVNode || move->isRefuteWithoutNullMove)
                    {
                        tempPVNode = true;
                    }
                    delete MPValue;
                    MPValue = PVS(tempPVNode, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, true, depthGone + 1, previousMoveWasCheck, nullWindowSearch);
                    bestMoveValue = -MPValue->value;
                    SelectedMove = move;
                    SelectedPV = MPValue->printString;
                    if (bestMoveValue != -160000)
                    {
                        availMoves++;
                    }
                    if (bestMoveValue > 159800 && bestMoveValue != 160000)
                    {
                        bestMoveValue--;
                    }
                    else if (bestMoveValue < -159800 && bestMoveValue != -160000)
                    {
                        bestMoveValue++;
                    }
                    move->value = bestMoveValue;
                }
                GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                delete missingInfoAboutPrevStateFromMove;
                missingInfoAboutPrevStateFromMove = nullptr;
                if (UCI::IsTest())
                {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                firstMove = false;
                if (bestMoveValue > alpha)
                {
                    if (bestMoveValue >= beta && ((bestMoveValue < 159800 && bestMoveValue > -159800) || !MAtESearch))
                    {
#if HOWL_CORRECTNESS_TESTING
                        RecordMoveOrderingCutoff(i, hasTTMove, *move, depth, depthGone);
#endif
                        RecordKiller(depthGone, *move);
                        if (isPVNode)
                        {
                            move->isRefuteWithoutNullMove = true;
                        }
                        // TODO: remove rest of list from movelist for memory management
                        if (bestMoveValue != 0)
                        {
                            uint16_t packed = TTMoveHelper::PackMove(*move);
                            TranspositionTable::Store(board4.ZobristHashCode, move->value, depth, TT_LOWER_BOUND, packed);
                        }
                        retValue->value = move->value;
                        retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + MPValue->printString;
                        deleteMoveList(moveList);
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                    alpha = bestMoveValue;
                }
                move->isRefuteWithoutNullMove = false;
                // AddToTable(board4, move, depth, false, nullWindowSearch, depthGone);
            }
            else
            {
                if (!isPVNode && depth <= 2 && availMoves > 0 && move->endPiece == 0 && move->promotionPiece <= 0 &&
                    (move->CastleFlag & 15) == 0 &&
                    alpha > -159800 && beta < 159800)
                {
                    if (staticEval == -200000)
                    {
                        if (inCheck < 0)
                        {
                            inCheck = BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove) ? 1 : 0;
                        }
                        if (inCheck == 0)
                        {
                            staticEval = EvaluationLogic::Evaluate(board4);
                        }
                    }
                    if (inCheck == 0 && staticEval != -200000)
                    {
                        int futilityMargin = (depth == 1) ? 150 : 300;
                        if (staticEval + futilityMargin <= alpha)
                        {
                            futilityPrunedCount++;
#if HOWL_CORRECTNESS_TESTING
                            g_futilityPruningSkippedQuietMoves++;

                            // Measure futility pruning safety in instrumentation mode:
                            MissingInfoAboutPrevStateFromMove *diagMissing = new MissingInfoAboutPrevStateFromMove(board4);
                            GameLogic::DoMove(board4, *move, prevMove, depth, depthGone);
                            int diagValue = 0;
                            if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                            {
                                diagValue = 0;
                            }
                            else
                            {
                                bool oppInCheck = BoardLogic::UnderAttack(board4, board4.pieces[(!board4.sideToMove) * 8 + 6].front(), board4.sideToMove);
                                MovePrintValue *diagMP = PVS(false, -alpha - Option::nullWindowSize, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, true, depthGone + 1, oppInCheck, true);
                                diagValue = -diagMP->value;
                                if (diagValue > 159800 && diagValue != 160000) diagValue--;
                                else if (diagValue < -159800 && diagValue != -160000) diagValue++;
                                std::string pvStr = diagMP->printString;
                                if (diagValue > alpha)
                                {
                                    delete diagMP;
                                    diagMP = PVS(false, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, true, depthGone + 1, oppInCheck, nullWindowSearch);
                                    diagValue = -diagMP->value;
                                    if (diagValue > 159800 && diagValue != 160000) diagValue--;
                                    else if (diagValue < -159800 && diagValue != -160000) diagValue++;
                                    pvStr = diagMP->printString;
                                }
                                delete diagMP;
                            }
                            bool givesCheck = BoardLogic::UnderAttack(board4, board4.pieces[(!board4.sideToMove) * 8 + 6].front(), board4.sideToMove);
                            GameLogic::UndoMove(board4, *move, *diagMissing);
                            delete diagMissing;

                            RecordFutilityCandidate(i, depth, depthGone, *move, staticEval, alpha, beta, diagValue, givesCheck);
#endif
                            continue;
                        }
                    }
                }

                bool tempRepeat = false;
                boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
                GameLogic::DoMove(board4, *move, prevMove, depth, depthGone);
                int value;
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                {
                    tempRepeat = true;
                    value = 0;
                    move->value = 0;
                }
                else
                {
                    bool tempPVNode = false;
                    if (move->isRefuteWithoutNullMove || (isPVNode && availMoves < 2))
                    {
                        tempPVNode = true;
                    }
                    bool exempt = false;
                    if (move->promotionPiece > 0)
                    {
                        exempt = true;
                    }
                    else if (move->endPiece > 0)
                    {
                        static const int pieceValLookup[8] = {0, 100, 320, 330, 500, 900, 20000, 0};
                        int movingPieceType = board4.mainBoard[move->endPlace] % 8;
                        int capturedPieceType = move->endPiece % 8;
                        if (pieceValLookup[capturedPieceType] >= pieceValLookup[movingPieceType])
                        {
                            exempt = true;
                        }
                    }
                    if (!tempPVNode && !exempt && depth >= 3)
                    {
                        bool isKiller = (depthGone >= 0 && depthGone < MaxKillerPly &&
                                         (killers[depthGone][0] == *move || killers[depthGone][1] == *move));
                        if (i >= 8 && depth >= 5 && move->endPiece == 0 && !isKiller)
                        {
                            LMRDepth = 2;
                        }
                        else
                        {
                            LMRDepth = 1;
                        }
                    }
                    delete MPValue;
                    MPValue = PVS(tempPVNode, -alpha - Option::nullWindowSize, -alpha, depth - 1 - LMRDepth, *move, move2, move3, prevMove, board4, MAtESearch, true, depthGone + 1, previousMoveWasCheck, true);
                    value = -MPValue->value;
                    if (value != -160000)
                    {
                        availMoves++;
                    }
                    if (value > 159800 && value != 160000)
                    {
                        value--;
                    }
                    else if (value < -159800 && value != -160000)
                    {
                        value++;
                    }
                    move->value = value;

                    if (LMRDepth > 0 && value <= alpha)
                    {
                        unverifiedLMRCount++;
                    }
#if HOWL_CORRECTNESS_TESTING
                    if (LMRDepth > 0 && !tempRepeat)
                    {
                        RecordLMRSearch(i, depth, depthGone, *move, LMRDepth, (value > alpha));
                    }
#endif
                }
                if (value > alpha /* && value < beta */)
                {
                    if (!tempRepeat)
                    {
                        bool tempPVNode = false;
                        if (isPVNode || move->isRefuteWithoutNullMove)
                        {
                            tempPVNode = true;
                        }
#if HOWL_CORRECTNESS_TESTING
                        int origAlphaForLMR = alpha;
                        int origBetaForLMR = beta;
#endif
                        delete MPValue;
                        MPValue = PVS(tempPVNode, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, true, depthGone + 1, previousMoveWasCheck, nullWindowSearch);
                        wasResearchedAtFullDepth = true;
                        value = -MPValue->value;
                        if (value > 159800 && value != 160000)
                        {
                            value--;
                        }
                        else if (value < -159800 && value != -160000)
                        {
                            value++;
                        }
                        move->value = value;
#if HOWL_CORRECTNESS_TESTING
                        if (LMRDepth > 0)
                        {
                            RecordLMRReSearchResult(i, value, origAlphaForLMR, origBetaForLMR);
                        }
#endif
                    }
                    if (value > alpha)
                    {
                        alpha = value;
                    }
                }
                GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                delete missingInfoAboutPrevStateFromMove;
                missingInfoAboutPrevStateFromMove = nullptr;
                if (UCI::IsTest())
                {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                if (value > bestMoveValue)
                {
                    if (value >= beta && ((value < 159800 && value > -159800) || !MAtESearch))
                    {
#if HOWL_CORRECTNESS_TESTING
                        RecordMoveOrderingCutoff(i, false, *move, depth, depthGone);
#endif
                        RecordKiller(depthGone, *move);
                        if (isPVNode)
                        {
                            move->isRefuteWithoutNullMove = true;
                        }
                        // TODO: delete movelist except first and second move
                        if (value != 0)
                        {
                            uint16_t packed = TTMoveHelper::PackMove(*move);
                            int storedDepth = (LMRDepth > 0 && !wasResearchedAtFullDepth) ? (depth - LMRDepth) : depth;
                            TranspositionTable::Store(board4.ZobristHashCode, move->value, storedDepth, TT_LOWER_BOUND, packed);
                        }
                        retValue->value = move->value;
                        retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + MPValue->printString;
                        deleteMoveList(moveList);
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                    move->isRefuteWithoutNullMove = false;
                    bestMoveValue = value;
                    SelectedMove = move;
                    SelectedPV = MPValue->printString;
                }
            }
        }
        if (availMoves == 0 && !BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
        {
            Move stalemateMove;
            stalemateMove.value = 0;
            stalemateMove.promotionPiece = -2;
            retValue->value = 0;
            deleteMoveList(moveList);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else if (availMoves == 0 && BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
        {
            Move mateMove;
            mateMove.value = -159999;
            retValue->value = -159999;
            deleteMoveList(moveList);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else
        {
            if (SelectedMove != nullptr && bestMoveValue != 0)
            {
                uint8_t flag = (bestMoveValue > origAlpha) ? TT_EXACT : TT_UPPER_BOUND;
                if (futilityPrunedCount > 0 || unverifiedLMRCount > 0)
                {
                    flag = static_cast<uint8_t>(flag + 4);
                }
                uint16_t packed = TTMoveHelper::PackMove(*SelectedMove);
                TranspositionTable::Store(board4.ZobristHashCode, bestMoveValue, depth, flag, packed);
            }
            retValue->value = bestMoveValue;
            retValue->printString = ChessStringManipulation::PVToString(*SelectedMove, 0, false, board4) + ' ' + SelectedPV;
            deleteMoveList(moveList);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
    }
}

void PVSSearch::deleteMoveList(MoveList moveList)
{
    for (int i = 0; i < moveList.count; ++i)
    {
        delete moveList.moves[i];
    }
}

void PVSSearch::NullMovePruning(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool mAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, MovePrintValue mPValue)
{
    /*if (depth == 1)
    {
        if (Evaluate(board4) + Option.futilityMargin <= alpha)
        {
            double valueRazored = qSearch(alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone);
            if (Evaluate(board4) + Option.futilityMargin > valueRazored)
            {
                return Evaluate(board4) + Option.futilityMargin;
            }
            else
            {
                return valueRazored;
            }
        }
    }
    if (depth == 2)
    {
        if (Evaluate(board4) + Option.extendedFutilityMargin <= alpha)
        {
            double valueRazored = qSearch(alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone);
            if (Evaluate(board4) + Option.extendedFutilityMargin > valueRazored)
            {
                return Evaluate(board4) + Option.extendedFutilityMargin;
            }
            else
            {
                return valueRazored;
            }
        }
    }
    if (depth == 3)
    {
        if (Evaluate(board4) + Option.superExtendedFutilityMargin <= alpha)
        {
            double valueRazored = qSearch(alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone);
            if (Evaluate(board4) + Option.superExtendedFutilityMargin > valueRazored)
            {
                return Evaluate(board4) + Option.superExtendedFutilityMargin;
            }
            else
            {
                return valueRazored;
            }
        }
    }
    int expectedValue = Evaluate(board4) + depth * 450;
    if (expectedValue <= alpha)
    {
        retValue.value = expectedValue;
        return retValue;
        //return expectedValue;
        //int valueRazored = qSearch(isPVNode, alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone, nullWindowSearch).value;
        //if (expectedValue > valueRazored)
        //{
        //    return expectedValue;
        //}
        //else
        //{
        //    return valueRazored;
        //}
    }*/
}

double PVSSearch::NullMoveReduction(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool mateSearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch)
{
    Move nullMove = Move();
    nullMove.promotionPiece = -1;

    Board *boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
    MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
    GameLogic::DoMove(board4, nullMove, prevMove, depthGone, depthGone);
    double valueReduced;
    std::unique_ptr<MovePrintValue> childResult;
    if (depth > 6)
    {
        childResult.reset(PVS(false, -beta, -beta + 1, depth - 4 - 1, nullMove, move2, move3, prevMove, board4, mateSearch, false, depthGone + 1, previousMoveWasCheck, true));
        valueReduced = -childResult->value;
    }
    else if (depth >= 4)
    {
        childResult.reset(PVS(false, -beta, -beta + 1, depth - 3 - 1, nullMove, move2, move3, prevMove, board4, mateSearch, false, depthGone + 1, previousMoveWasCheck, true));
        valueReduced = -childResult->value;
    }
    else
    {
        childResult.reset(QSearcher::QSearch(false, -beta, -beta + 1, nullMove, depthGone + 1, 0, true, 0, move2, move3, prevMove, board4, false, depthGone + 1, nullWindowSearch));
        valueReduced = -childResult->value;
    }
    GameLogic::UndoMove(board4, nullMove, *missingInfoAboutPrevStateFromMove);
    if (UCI::IsTest())
    {
        Board::AreBoardsEqual(board4, *boardCopy);
        delete boardCopy;
        boardCopy = nullptr;
    }
    delete missingInfoAboutPrevStateFromMove;
    missingInfoAboutPrevStateFromMove = nullptr;
    return valueReduced;
}

MovePrintValue *PVSSearch::StartQSearch(bool isPVNode, int alpha, int beta, Move &prevMove, int depthGone, Move &move1, Move &move2, Move &move3, Board &board4, bool nullWindowSearch, bool previousMoveWasCheck)
{
    MovePrintValue *res = nullptr;
    if (previousMoveWasCheck)
    {
        res = QSearcher::QSearch(isPVNode, alpha, beta, prevMove, depthGone, 1, false, 1, move1, move2, move3, board4, false, depthGone, nullWindowSearch);
    }
    // else if (evaluate(recDepth) > beta && isNullMoveAllowed)
    //{
    //     return evaluate(recDepth);
    // }
    else if (prevMove.endPiece != 0 || prevMove.promotionPiece > 0)
    {
        res = QSearcher::QSearch(isPVNode, alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone, nullWindowSearch);
    }
    else
    {
        res = QSearcher::QSearch(isPVNode, alpha, beta, prevMove, depthGone, 0, false, 0, move1, move2, move3, board4, false, depthGone, nullWindowSearch);
    }
    return res;
}

void PVSSearch::IGG(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool lastCheck, bool nullWindowSearch, MoveList moveList)
{
    Board* boardCopy = nullptr; // <-- ADD THIS LINE
    MovePrintValue *MPValue = new MovePrintValue();
    int value = -200000;
    int tempDepth = depth;
    int bestMoveValue = -200000;
    depth = moveOrderingDepth[depth];
    {
        if (depth >= 3)
        {
            int availMovesIr = 0;
            bool firstMoveIr = true;
            {
                firstMoveIr = true;
                for (int i = 0; i < moveList.count; ++i)
                {
                    Move *move = moveList.moves[i];
                    if (firstMoveIr)
                    {
                        boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                        MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
                        GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone);
                        if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                        {
                            bestMoveValue = 0;
                            move->value = 0;
                        }
                        else
                        {
                            bool tempPVNode = false;
                            if (isPVNode || move->isRefuteWithoutNullMove)
                            {
                                tempPVNode = true;
                            }
                            delete MPValue;
                            MPValue = PVSSearch::PVS(tempPVNode, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, isNullMoveAllowed, depthGone + 1, lastCheck, nullWindowSearch);
                            bestMoveValue = -MPValue->value;
                            if (bestMoveValue != -160000)
                            {
                                availMovesIr++;
                            }
                            if (bestMoveValue > 159800 && bestMoveValue != 160000)
                            {
                                bestMoveValue--;
                            }
                            else if (bestMoveValue < -159800 && bestMoveValue != -160000)
                            {
                                bestMoveValue++;
                            }
                            move->value = bestMoveValue;
                        }
                        GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                        delete missingInfoAboutPrevStateFromMove;
                        missingInfoAboutPrevStateFromMove = nullptr;
                        if (UCI::IsTest())
                        {
                            Board::AreBoardsEqual(board4, *boardCopy);
                            delete boardCopy;
                            boardCopy = nullptr;
                        }
                        firstMoveIr = false;
                        if (bestMoveValue > alpha)
                        {
                            if (bestMoveValue >= beta && ((bestMoveValue < 159800 && bestMoveValue > -159800) || !MAtESearch))
                            {
                                if (isPVNode)
                                {
                                    move->isRefuteWithoutNullMove = true;
                                }
                            }
                            alpha = bestMoveValue;
                        }
                        move->isRefuteWithoutNullMove = false;
                        // AddToTable(board4, move, depth, false, nullWindowSearch, depthGone);
                    }
                    else
                    {
                        bool tempRepeat = false;
                        boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                        MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
                        GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone);
                        if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                        {
                            tempRepeat = true;
                            value = 0;
                            move->value = 0;
                        }
                        else
                        {
                            bool tempPVNode = false;
                            if (move->isRefuteWithoutNullMove || (isPVNode && availMovesIr < 2))
                            {
                                tempPVNode = true;
                            }
                            int LMRDepth = 0;
                            bool exempt = false;
                            if (move->promotionPiece > 0)
                            {
                                exempt = true;
                            }
                            else if (move->endPiece > 0)
                            {
                                static const int pieceValLookup[8] = {0, 100, 320, 330, 500, 900, 20000, 0};
                                int movingPieceType = board4.mainBoard[move->endPlace] % 8;
                                int capturedPieceType = move->endPiece % 8;
                                if (pieceValLookup[capturedPieceType] >= pieceValLookup[movingPieceType])
                                {
                                    exempt = true;
                                }
                            }
                            if (!tempPVNode && !exempt && depth >= 3)
                            {
                                LMRDepth = 1;
                            }
                            delete MPValue;
                            MPValue = PVSSearch::PVS(tempPVNode, -alpha - Option::nullWindowSize, -alpha, depth - 1 - LMRDepth, *move, move2, move3, prevMove, board4, MAtESearch, isNullMoveAllowed, depthGone + 1, lastCheck, true);
                            value = -MPValue->value;
                            if (value != -160000)
                            {
                                availMovesIr++;
                            }
                            if (value > 159800 && value != 160000)
                            {
                                value--;
                            }
                            else if (value < -159800 && value != -160000)
                            {
                                value++;
                            }
                            move->value = value;
                        }
                        if (value > alpha /* && value < beta */)
                        {
                            if (!tempRepeat)
                            {
                                bool tempPVNode = false;
                                if (isPVNode || move->isRefuteWithoutNullMove)
                                {
                                    tempPVNode = true;
                                }
                                delete MPValue;
                                MPValue = PVSSearch::PVS(tempPVNode, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, isNullMoveAllowed, depthGone + 1, lastCheck, nullWindowSearch);
                                value = -MPValue->value;
                                if (value > 159800 && value != 160000)
                                {
                                    value--;
                                }
                                else if (value < -159800 && value != -160000)
                                {
                                    value++;
                                }
                                move->value = value;
                            }
                            if (value > alpha)
                            {
                                alpha = value;
                            }
                        }

                        GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                        delete missingInfoAboutPrevStateFromMove;
                        missingInfoAboutPrevStateFromMove = nullptr;
                        if (UCI::IsTest())
                        {
                            Board::AreBoardsEqual(board4, *boardCopy);
                            delete boardCopy;
                            boardCopy = nullptr;
                        }
                        if (value > bestMoveValue)
                        {
                            if (value >= beta && ((value < 159800 && value > -159800) || !MAtESearch))
                            {
                                if (isPVNode)
                                {
                                    move->isRefuteWithoutNullMove = true;
                                }
                            }
                            move->isRefuteWithoutNullMove = false;
                            // AddToTable(board4, move, depth, false, nullWindowSearch, depthGone);
                            bestMoveValue = value;
                        }
                    }
                }
                std::sort(moveList.moves, moveList.moves + moveList.count, [](Move *a, Move *b)
                          { return b->value > a->value; });
            }
        }
    }
    delete MPValue;
    MPValue = nullptr;
}
