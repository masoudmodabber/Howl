#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <algorithm>
#include "Move.h"

enum TTFlag : uint8_t
{
    TT_NONE = 0,
    TT_EXACT = 1,
    TT_LOWER_BOUND = 2, // Fail-high / Beta cutoff (rigorous)
    TT_UPPER_BOUND = 3, // Fail-low / Alpha bound (rigorous)
    TT_EXACT_HEURISTIC = 5,
    TT_LOWER_HEURISTIC = 6,
    TT_UPPER_HEURISTIC = 7
};

inline bool TTFlagIsRigorous(uint8_t flag)
{
    return flag >= TT_EXACT && flag <= TT_UPPER_BOUND;
}

inline uint8_t TTBaseFlag(uint8_t flag)
{
    if (flag >= TT_EXACT_HEURISTIC)
    {
        return static_cast<uint8_t>(flag - 4);
    }
    return flag;
}

#pragma pack(push, 1)
struct TTEntry
{
    uint64_t key = 0;       // 8 bytes
    int32_t score = 0;      // 4 bytes
    int8_t depth = 0;       // 1 byte
    uint8_t flag = TT_NONE; // 1 byte
    uint16_t bestMove = 0;  // 2 bytes
};
#pragma pack(pop)

static_assert(sizeof(TTEntry) == 16, "TTEntry must be exactly 16 bytes");

class TTMoveHelper
{
public:
    static constexpr uint16_t PackMove(int from, int to, int promotionPiece)
    {
        uint16_t promo = (promotionPiece > 0) ? static_cast<uint16_t>(promotionPiece & 0xF) : 0;
        return static_cast<uint16_t>((from & 0x3F) | ((to & 0x3F) << 6) | (promo << 12));
    }

    static constexpr uint16_t PackMove(const Move& move)
    {
        return PackMove(move.beginPlace, move.endPlace, move.promotionPiece);
    }

    static constexpr int UnpackFrom(uint16_t packed)
    {
        return packed & 0x3F;
    }

    static constexpr int UnpackTo(uint16_t packed)
    {
        return (packed >> 6) & 0x3F;
    }

    static constexpr int UnpackPromotion(uint16_t packed)
    {
        return (packed >> 12) & 0xF;
    }
};

struct TTStats
{
    long long probes = 0;
    long long hits = 0;
    long long usableBestMoveHits = 0;
    long long ttMoveAlreadyFirst = 0;
    long long cutoffs = 0;
};

#if HOWL_CORRECTNESS_TESTING
struct TTTelemetryBucket
{
    uint64_t probes = 0;
    uint64_t hits = 0;
    uint64_t cutoffs = 0;
};

struct TTTelemetryStats
{
    uint64_t eligibleProbes = 0;
    uint64_t hits = 0;
    uint64_t misses = 0;

    uint64_t hitsSufficientDepth = 0;
    uint64_t hitsInsufficientDepth = 0;

    uint64_t hitsExact = 0;
    uint64_t hitsLower = 0;
    uint64_t hitsUpper = 0;

    uint64_t cutoffsExact = 0;
    uint64_t cutoffsLower = 0;
    uint64_t cutoffsUpper = 0;
    uint64_t totalCutoffs = 0;

    uint64_t ttMoveFoundNoCutoff = 0;
    uint64_t ttMoveMatchedLegal = 0;
    uint64_t ttMoveMissingFromGenerated = 0;

    uint64_t stores = 0;
    uint64_t emptySlotStores = 0;
    uint64_t overwriteSameKey = 0;
    uint64_t replacementCollisions = 0;
    uint64_t replacementGreaterDepthOverwritten = 0;
    uint64_t hashKeyMismatchOnProbe = 0; // Slot occupied by different key

    TTTelemetryBucket depth1To2;
    TTTelemetryBucket depth3To5;
    TTTelemetryBucket depth6To8;
    TTTelemetryBucket depth9Plus;
};
#endif

class TranspositionTable
{
public:
    static bool Resize(std::size_t targetBytes);
    static void Clear();
    static std::size_t CapacityBytes();
    static std::size_t EntryCount();
    static bool IsActive();

    static bool Probe(uint64_t key, TTEntry& entry);
    static void Store(uint64_t key, int32_t score, int8_t depth, uint8_t flag, uint16_t bestMove);

    static TTStats Stats();
    static void ResetStats();
    static void RecordHitStats(bool usableBestMove, bool alreadyFirst);
    static void RecordCutoff();

    static void SetCutoffsEnabled(bool enabled);
    static bool CutoffsEnabled();

#if HOWL_CORRECTNESS_TESTING
    static void SetAllocationFailureThresholdForTesting(std::size_t thresholdBytes);
    static TTTelemetryStats& TelemetryStats();
    static void ResetTelemetryStats();
    static void PrintTelemetryStats();
#endif

private:
    static std::vector<TTEntry> entries;
    static std::size_t entryMask;
    static TTStats stats;
    static bool cutoffsEnabled;
#if HOWL_CORRECTNESS_TESTING
    static std::size_t failureThreshold;
#endif
};

#endif // TRANSPOSITIONTABLE_H
