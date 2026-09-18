#include "TranspositionTable.h"
#include <iostream>
#include <cstdio>

std::vector<TTEntry> TranspositionTable::entries{};
std::size_t TranspositionTable::entryMask = 0;
uint8_t TranspositionTable::generation = 0;
TTStats TranspositionTable::stats{};
QSearchTTStats TranspositionTable::qSearchStats{};
bool TranspositionTable::cutoffsEnabled = true;
#if HOWL_CORRECTNESS_TESTING
std::size_t TranspositionTable::failureThreshold = 0;
#endif

bool TranspositionTable::Resize(std::size_t targetBytes)
{
    entries.clear();
    entries.shrink_to_fit();
    entryMask = 0;

    if (targetBytes < 3 * sizeof(TTEntry))
    {
        return true;
    }

#if HOWL_CORRECTNESS_TESTING
    if (failureThreshold != 0 && targetBytes >= failureThreshold)
    {
        return false;
    }
#endif

    std::size_t count = 1;
    while (count * 2 * 3 * sizeof(TTEntry) <= targetBytes)
    {
        count *= 2;
    }

    try
    {
        entries.resize(count * 3);
        entryMask = count - 1;
        return true;
    }
    catch (...)
    {
        entries.clear();
        entries.shrink_to_fit();
        entryMask = 0;
        return false;
    }
}

void TranspositionTable::Clear()
{
    std::fill(entries.begin(), entries.end(), TTEntry{});
    ResetStats();
}

std::size_t TranspositionTable::CapacityBytes()
{
    return entries.capacity() * sizeof(TTEntry);
}

std::size_t TranspositionTable::EntryCount()
{
    return entries.size();
}

void TranspositionTable::NewSearch()
{
    ++generation;
}

bool TranspositionTable::IsActive()
{
    return !entries.empty();
}

#if HOWL_CORRECTNESS_TESTING
TTTelemetryStats g_ttTelemetryStats;

// Shadow table tracking deeper entries that were overwritten by shallow same-key stores
struct ShadowEntryInfo {
    TTEntry entry;
    bool wasOverwritten = false;
    bool producedCutoff = false;
    bool producedMoveOnly = false;
};
std::vector<ShadowEntryInfo> g_shadowEntries;

TTTelemetryStats& TranspositionTable::TelemetryStats()
{
    return g_ttTelemetryStats;
}

void TranspositionTable::ResetTelemetryStats()
{
    g_ttTelemetryStats = TTTelemetryStats{};
    g_shadowEntries.clear();
    if (!entries.empty()) {
        g_shadowEntries.resize(entries.size());
    }
}

void TranspositionTable::PrintTelemetryStats()
{
    // Aggregate shadow entry outcomes
    for (const auto& s : g_shadowEntries) {
        if (s.wasOverwritten) {
            if (s.producedCutoff) g_ttTelemetryStats.shallowSameKey.shadowProducedCutoff++;
            else if (s.producedMoveOnly) g_ttTelemetryStats.shallowSameKey.shadowProducedUsableMoveOnly++;
            else g_ttTelemetryStats.shallowSameKey.shadowNoLaterUse++;
        }
    }

    std::cout << "=== Transposition Table Telemetry (Recursive PVS) ===\n";
    std::cout << "  Eligible PVS Probes: " << g_ttTelemetryStats.eligibleProbes << "\n";
    double hitRate = (g_ttTelemetryStats.eligibleProbes > 0) ? (100.0 * g_ttTelemetryStats.hits / static_cast<double>(g_ttTelemetryStats.eligibleProbes)) : 0.0;
    std::cout << "  TT Probe Hits:       " << g_ttTelemetryStats.hits << " (" << hitRate << "%)\n";
    std::cout << "  TT Probe Misses:     " << g_ttTelemetryStats.misses << " (Slot occupied key mismatch: " << g_ttTelemetryStats.hashKeyMismatchOnProbe << ")\n";
    std::cout << "  Hits Stored Depth >= Requested: " << g_ttTelemetryStats.hitsSufficientDepth << "\n";
    std::cout << "  Hits Stored Depth <  Requested: " << g_ttTelemetryStats.hitsInsufficientDepth << "\n";

    std::cout << "\n[Hits by Bound Type]\n";
    std::cout << "  Exact Bound: " << g_ttTelemetryStats.hitsExact << "\n";
    std::cout << "  Lower Bound: " << g_ttTelemetryStats.hitsLower << "\n";
    std::cout << "  Upper Bound: " << g_ttTelemetryStats.hitsUpper << "\n";

    std::cout << "\n[Cutoffs by Bound Type]\n";
    std::cout << "  Exact Cutoffs: " << g_ttTelemetryStats.cutoffsExact << "\n";
    std::cout << "  Lower Cutoffs: " << g_ttTelemetryStats.cutoffsLower << "\n";
    std::cout << "  Upper Cutoffs: " << g_ttTelemetryStats.cutoffsUpper << "\n";
    std::cout << "  Total Cutoffs: " << g_ttTelemetryStats.totalCutoffs << "\n";

    std::cout << "\n[TT Move Ordering]\n";
    std::cout << "  TT Move Found (No Cutoff): " << g_ttTelemetryStats.ttMoveFoundNoCutoff << "\n";
    std::cout << "  TT Move Matched Generated: " << g_ttTelemetryStats.ttMoveMatchedLegal << "\n";
    std::cout << "  TT Move Missing Generated: " << g_ttTelemetryStats.ttMoveMissingFromGenerated << "\n";

    std::cout << "\n[TT Store & Replacement Details]\n";
    std::cout << "  Total Stores: " << g_ttTelemetryStats.stores << "\n";
    std::cout << "  Empty Slot Stores: " << g_ttTelemetryStats.emptySlotStores << "\n";
    std::cout << "  Overwrite Same Key: " << g_ttTelemetryStats.overwriteSameKey << "\n";
    std::cout << "  Replacement Collisions: " << g_ttTelemetryStats.replacementCollisions << "\n";
    std::cout << "  Replaced Entry Had Greater Depth: " << g_ttTelemetryStats.replacementGreaterDepthOverwritten << "\n";

    std::cout << "\n=== Shallow Same-Key Overwrites (incoming depth < existing depth) ===\n";
    const auto& s = g_ttTelemetryStats.shallowSameKey;
    std::cout << "  Total Shallow Same-Key Attempts: " << s.totalAttempts << "\n";
    std::cout << "  Depth Delta = 1:   " << s.depthDiff1 << "\n";
    std::cout << "  Depth Delta = 2:   " << s.depthDiff2 << "\n";
    std::cout << "  Depth Delta = 3-4: " << s.depthDiff3To4 << "\n";
    std::cout << "  Depth Delta = 5+:  " << s.depthDiff5Plus << "\n";

    std::cout << "  Flags (Exist -> Inc):\n";
    std::cout << "    Exact -> Exact: " << s.exactToExact << " | Exact -> Lower: " << s.exactToLower << " | Exact -> Upper: " << s.exactToUpper << "\n";
    std::cout << "    Lower -> Exact: " << s.lowerToExact << " | Lower -> Lower: " << s.lowerToLower << " | Lower -> Upper: " << s.lowerToUpper << "\n";
    std::cout << "    Upper -> Exact: " << s.upperToExact << " | Upper -> Lower: " << s.upperToLower << " | Upper -> Upper: " << s.upperToUpper << "\n";

    std::cout << "  Incoming BestMove Differs: " << s.bestMoveDiffers << "\n";
    std::cout << "  Existing BestMove Empty & Incoming Provides: " << s.existingBestMoveEmptyIncomingProvides << "\n";
    std::cout << "  Incoming Score Is Mate: " << s.incomingScoreIsMate << "\n";
    std::cout << "  Existing Score Is Mate: " << s.existingScoreIsMate << "\n";

    std::cout << "  Overwritten Deeper Entry Shadow Outcome:\n";
    std::cout << "    Would have produced TT cutoff: " << s.shadowProducedCutoff << "\n";
    std::cout << "    Would have produced usable move only: " << s.shadowProducedUsableMoveOnly << "\n";
    std::cout << "    No later use: " << s.shadowNoLaterUse << "\n";

    auto printBucket = [](const char* label, const TTTelemetryBucket& b) {
        double hr = (b.probes > 0) ? (100.0 * b.hits / static_cast<double>(b.probes)) : 0.0;
        double cr = (b.probes > 0) ? (100.0 * b.cutoffs / static_cast<double>(b.probes)) : 0.0;
        char buf[256];
        snprintf(buf, sizeof(buf), "  %-12s Probes: %8lu | Hits: %8lu (%5.1f%%) | Cutoffs: %8lu (%5.1f%%)\n",
                 label, (unsigned long)b.probes, (unsigned long)b.hits, hr, (unsigned long)b.cutoffs, cr);
        std::cout << buf;
    };
    std::cout << "\n[Breakdown by Remaining Depth]\n";
    printBucket("Depth 1-2:", g_ttTelemetryStats.depth1To2);
    printBucket("Depth 3-5:", g_ttTelemetryStats.depth3To5);
    printBucket("Depth 6-8:", g_ttTelemetryStats.depth6To8);
    printBucket("Depth 9+:", g_ttTelemetryStats.depth9Plus);
}

void TranspositionTable::CheckShadowEntryOnProbe(uint64_t key, int depth, int alpha, int beta, bool isPVNode, const MoveList& moveList, bool actualCutoffOccurred)
{
    if (entries.empty() || key == 0) return;
    std::size_t idx = key & entryMask;
    if (g_shadowEntries.size() > idx && g_shadowEntries[idx].wasOverwritten && g_shadowEntries[idx].entry.key == key)
    {
        const TTEntry& shadow = g_shadowEntries[idx].entry;
        // Check if shadow entry would have produced a cutoff that didn't happen
        if (!actualCutoffOccurred && !isPVNode && shadow.depth >= depth && TTFlagIsRigorous(shadow.flag))
        {
            if (TTBaseFlag(shadow.flag) == TT_EXACT ||
                (TTBaseFlag(shadow.flag) == TT_LOWER_BOUND && shadow.score >= beta) ||
                (TTBaseFlag(shadow.flag) == TT_UPPER_BOUND && shadow.score <= alpha))
            {
                g_shadowEntries[idx].producedCutoff = true;
                return;
            }
        }

        // Check if shadow entry had a valid move that could be matched
        if (shadow.bestMove != 0)
        {
            int ttFrom = TTMoveHelper::UnpackFrom(shadow.bestMove);
            int ttTo = TTMoveHelper::UnpackTo(shadow.bestMove);
            int ttPromo = TTMoveHelper::UnpackPromotion(shadow.bestMove);
            for (int i = 0; i < moveList.count; ++i)
            {
                Move* m = moveList.moves[i];
                if (m->beginPlace == ttFrom && m->endPlace == ttTo &&
                    (ttPromo == 0 ? (m->promotionPiece <= 0) : (m->promotionPiece == ttPromo)))
                {
                    g_shadowEntries[idx].producedMoveOnly = true;
                    break;
                }
            }
        }
    }
}
#endif

bool TranspositionTable::Probe(uint64_t key, TTEntry& entry)
{
    if (entries.empty()) return false;
    stats.probes++;
    const std::size_t base = (key & entryMask) * 3;
    for (int i = 0; i < 3; ++i)
    {
        TTEntry& candidate = entries[base + i];
        if (candidate.key == key && candidate.flag != TT_NONE)
        {
            stats.hits++;
            candidate.generation = generation;
            entry = candidate;
            return true;
        }
    }
    return false;
}

void TranspositionTable::Store(uint64_t key, int32_t score, int8_t depth,
    uint8_t flag, uint16_t bestMove, int32_t staticEval, bool pv)
{
    if (entries.empty() || key == 0 || flag == TT_NONE) return;
    const std::size_t base = (key & entryMask) * 3;
    TTEntry* replacement = &entries[base];
    for (int i = 0; i < 3; ++i)
    {
        TTEntry& candidate = entries[base + i];
        if (candidate.key == 0 || candidate.flag == TT_NONE)
        {
            replacement = &candidate;
            break;
        }
        if (candidate.key == key)
        {
            replacement = &candidate;
            break;
        }
        const int candidateValue = candidate.depth - 8 * int(uint8_t(generation - candidate.generation));
        const int replacementValue = replacement->depth -
            8 * int(uint8_t(generation - replacement->generation));
        if (candidateValue < replacementValue)
            replacement = &candidate;
    }
#if HOWL_CORRECTNESS_TESTING
    g_ttTelemetryStats.stores++;
#endif
    if (replacement->key == key && depth + 4 < replacement->depth && flag != TT_EXACT)
    {
        if (bestMove) replacement->bestMove = bestMove;
        if (staticEval != TT_NO_STATIC_EVAL)
            replacement->staticEval = static_cast<int16_t>(std::clamp(staticEval, -32767, 32766));
        replacement->generation = generation;
        return;
    }
    *replacement = TTEntry{key, score, depth, flag, bestMove,
        staticEval == TT_NO_STATIC_EVAL ? TT_NO_STATIC_EVAL
            : static_cast<int16_t>(std::clamp(staticEval, -32767, 32766)),
        static_cast<uint8_t>(pv ? TT_META_PV : 0), generation};
}

bool TranspositionTable::ProbeQSearch(uint64_t key, QSearchTTEntry& entry)
{
    qSearchStats.probes++;
    TTEntry candidate{};
    if (!Probe(key, candidate)) return false;
    entry.key = candidate.key;
    entry.score = candidate.score;
    entry.staticEval = candidate.staticEval;
    entry.depth = candidate.depth;
    entry.flag = candidate.flag;
    entry.bestMove = candidate.bestMove;
    entry.state = QTT_NONE;
    entry.staticEvalValid = candidate.staticEval != TT_NO_STATIC_EVAL;
    entry.pv = (candidate.metadata & TT_META_PV) != 0;
    return true;
}

void TranspositionTable::StoreQSearch(uint64_t key, int32_t score, int8_t depth,
    uint8_t state, uint8_t flag, uint16_t bestMove,
    int32_t staticEval, bool staticEvalValid, bool pv)
{
    (void)state;
    Store(key, score, depth, flag, bestMove,
          staticEvalValid ? staticEval : TT_NO_STATIC_EVAL, pv);
    qSearchStats.stores++;
}

void TranspositionTable::RecordQSearchCutoff()
{
    qSearchStats.usableCutoffs++;
}

QSearchTTStats TranspositionTable::QSearchStats()
{
    return qSearchStats;
}

TTStats TranspositionTable::Stats()
{
    return stats;
}

void TranspositionTable::ResetStats()
{
    stats = TTStats{};
    qSearchStats = QSearchTTStats{};
}

void TranspositionTable::RecordHitStats(bool usableBestMove, bool alreadyFirst)
{
    if (usableBestMove)
    {
        stats.usableBestMoveHits++;
        if (alreadyFirst)
        {
            stats.ttMoveAlreadyFirst++;
        }
    }
}

void TranspositionTable::RecordCutoff()
{
    stats.cutoffs++;
}

void TranspositionTable::SetCutoffsEnabled(bool enabled)
{
    cutoffsEnabled = enabled;
}

bool TranspositionTable::CutoffsEnabled()
{
    return cutoffsEnabled;
}

#if HOWL_CORRECTNESS_TESTING
void TranspositionTable::SetAllocationFailureThresholdForTesting(std::size_t thresholdBytes)
{
    failureThreshold = thresholdBytes;
}
#endif
