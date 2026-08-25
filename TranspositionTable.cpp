#include "TranspositionTable.h"

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
