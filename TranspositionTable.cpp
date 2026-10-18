#include "TranspositionTable.h"
#include <iostream>
#include <cstdio>
#include <unordered_map>
#include <array>
#include <string>
#include "RepetitionHistory.h"

std::vector<TTEntry> TranspositionTable::entries{};
static bool verificationIsolation = false;
static std::unordered_map<std::string, std::array<TTEntry, 3>> verificationEntries{};
std::size_t TranspositionTable::entryMask = 0;
uint8_t TranspositionTable::generation = 0;
TTStats TranspositionTable::stats{};
QSearchTTStats TranspositionTable::qSearchStats{};
bool TranspositionTable::cutoffsEnabled = true;

bool TranspositionTable::Resize(std::size_t targetBytes)
{
    entries.clear();
    entries.shrink_to_fit();
    verificationEntries.clear();
    entryMask = 0;

    if (targetBytes < 3 * sizeof(TTEntry))
    {
        return true;
    }


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

static std::string VerificationKey(uint64_t key)
{
    std::string result(reinterpret_cast<const char*>(&key), sizeof(key));
    const auto& history = RepetitionHistory::GetHistory();
    result.append(reinterpret_cast<const char*>(history.data()), history.size() * sizeof(history[0]));
    return result;
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


bool TranspositionTable::Probe(uint64_t key, TTEntry& entry)
{
    if (verificationIsolation)
    {
        auto it = verificationEntries.find(VerificationKey(key));
        if (it == verificationEntries.end()) return false;
        for (const TTEntry& candidate : it->second)
            if (candidate.key == key && candidate.flag != TT_NONE)
            {
                entry = candidate;
                return true;
            }
        return false;
    }
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
    if (verificationIsolation)
    {
        if (key == 0 || flag == TT_NONE) return;
        auto& slots = verificationEntries[VerificationKey(key)];
        TTEntry* replacement = &slots[0];
        for (int i = 0; i < 3; ++i)
        {
            TTEntry& candidate = slots[i];
            if (candidate.key == 0 || candidate.flag == TT_NONE || candidate.key == key)
            { replacement = &candidate; break; }
            const int candidateValue = candidate.depth - 8 * int(uint8_t(generation - candidate.generation));
            const int replacementValue = replacement->depth - 8 * int(uint8_t(generation - replacement->generation));
            if (candidateValue < replacementValue) replacement = &candidate;
        }
        if (replacement->key == key && depth + 4 < replacement->depth && flag != TT_EXACT)
        {
            if (bestMove) replacement->bestMove = bestMove;
            replacement->generation = generation;
            return;
        }
        *replacement = TTEntry{key, score, depth, flag, bestMove,
            staticEval == TT_NO_STATIC_EVAL ? TT_NO_STATIC_EVAL
                : static_cast<int16_t>(std::clamp(staticEval, -32767, 32766)),
            static_cast<uint8_t>(pv ? TT_META_PV : 0), generation};
        return;
    }
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

void TranspositionTable::SetVerificationIsolation(bool enabled)
{
    verificationIsolation = enabled;
    if (enabled) verificationEntries.clear();
}
