#ifndef EXCHANGECHESSCACHE_H
#define EXCHANGECHESSCACHE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#ifndef HOWL_EXCHANGE_CACHE_STATS
#define HOWL_EXCHANGE_CACHE_STATS 0
#endif

struct ExchangeCacheStatistics {
    std::uint64_t probes = 0;
    std::uint64_t hits = 0;
    std::uint64_t misses = 0;
    std::uint64_t stores = 0;
    std::uint64_t replacements = 0;
    std::size_t uniqueEntries = 0;
};

class ExchangeChessCache {
public:
    explicit ExchangeChessCache(std::size_t capacityBytes = 0);
    bool resize(std::size_t capacityBytes);
    std::size_t size() const;
    std::size_t capacityBytes() const;
    void addToCache(std::uint64_t key, int value);
    std::optional<int> getFromCache(std::uint64_t key);
    void clear();
    ExchangeCacheStatistics statistics() const;
    void resetStatistics();

#if HOWL_CORRECTNESS_TESTING
    static void SetAllocationFailureThresholdForTesting(std::size_t capacityBytes);
#endif

private:
    struct Entry {
        std::uint64_t packedValue = 0;
    };

    struct Set {
        Entry ways[2];
    };

    static_assert(sizeof(Entry) == 8, "Exchange cache entries must be exactly 8 bytes");
    static_assert(sizeof(Set) == 16, "Exchange cache sets must contain two adjacent entries");

    std::unique_ptr<Set[]> sets;
    std::size_t setCount = 0;
    std::size_t setMask = 0;
    std::size_t occupiedEntries = 0;
#if HOWL_EXCHANGE_CACHE_STATS
    ExchangeCacheStatistics cacheStatistics;
#endif
#if HOWL_CORRECTNESS_TESTING
    static std::size_t allocationFailureThreshold;
#endif

    static std::uint64_t MixKey(std::uint64_t key);
    static std::uint64_t Pack(std::uint64_t key, int value);
    static int UnpackScore(std::uint64_t packedValue);
    static bool IsValid(std::uint64_t packedValue);
    static bool KeyMatches(std::uint64_t packedValue, std::uint64_t key);
};

#endif
