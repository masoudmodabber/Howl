#include "TranspositionTable.h"
#include <iostream>
#include <cstdio>

std::vector<TTEntry> TranspositionTable::entries{};
std::size_t TranspositionTable::entryMask = 0;
TTStats TranspositionTable::stats{};
bool TranspositionTable::cutoffsEnabled = true;
#if HOWL_CORRECTNESS_TESTING
std::size_t TranspositionTable::failureThreshold = 0;
#endif

bool TranspositionTable::Resize(std::size_t targetBytes)
{
    entries.clear();
    entries.shrink_to_fit();
    entryMask = 0;

    if (targetBytes < sizeof(TTEntry))
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
    while (count * 2 * sizeof(TTEntry) <= targetBytes)
    {
        count *= 2;
    }

    try
    {
        entries.resize(count);
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

bool TranspositionTable::IsActive()
{
    return !entries.empty();
}

#if HOWL_CORRECTNESS_TESTING
TTTelemetryStats g_ttTelemetryStats;

TTTelemetryStats& TranspositionTable::TelemetryStats()
{
    return g_ttTelemetryStats;
}

void TranspositionTable::ResetTelemetryStats()
{
    g_ttTelemetryStats = TTTelemetryStats{};
}

void TranspositionTable::PrintTelemetryStats()
{
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
#endif

bool TranspositionTable::Probe(uint64_t key, TTEntry& entry)
{
    if (entries.empty())
    {
        return false;
    }
    stats.probes++;
    std::size_t idx = key & entryMask;
    if (entries[idx].key == key && entries[idx].flag != TT_NONE)
    {
        stats.hits++;
        entry = entries[idx];
        return true;
    }
    return false;
}

void TranspositionTable::Store(uint64_t key, int32_t score, int8_t depth, uint8_t flag, uint16_t bestMove)
{
    if (entries.empty() || key == 0)
    {
        return;
    }
    std::size_t idx = key & entryMask;
#if HOWL_CORRECTNESS_TESTING
    g_ttTelemetryStats.stores++;
    if (entries[idx].key == 0 || entries[idx].flag == TT_NONE)
    {
        g_ttTelemetryStats.emptySlotStores++;
    }
    else if (entries[idx].key == key)
    {
        g_ttTelemetryStats.overwriteSameKey++;
    }
    else
    {
        g_ttTelemetryStats.replacementCollisions++;
        if (entries[idx].depth > depth)
        {
            g_ttTelemetryStats.replacementGreaterDepthOverwritten++;
        }
    }
#endif
    if (entries[idx].key == 0 || entries[idx].key == key || depth >= entries[idx].depth)
    {
        entries[idx].key = key;
        entries[idx].score = score;
        entries[idx].depth = depth;
        entries[idx].flag = flag;
        if (bestMove != 0)
        {
            entries[idx].bestMove = bestMove;
        }
    }
    else if (entries[idx].key == key && bestMove != 0 && entries[idx].bestMove == 0)
    {
        entries[idx].bestMove = bestMove;
    }
}

TTStats TranspositionTable::Stats()
{
    return stats;
}

void TranspositionTable::ResetStats()
{
    stats = TTStats{};
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
