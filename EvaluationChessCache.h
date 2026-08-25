#ifndef EVALUATIONCHESSCACHE_H
#define EVALUATIONCHESSCACHE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#ifndef HOWL_EVAL_CACHE_STATS
#define HOWL_EVAL_CACHE_STATS 0
#endif

struct EvaluationCacheStatistics {
    std::uint64_t probes = 0;
    std::uint64_t hits = 0;
    std::uint64_t misses = 0;
    std::uint64_t stores = 0;
    std::uint64_t replacements = 0;
    std::size_t uniqueEntries = 0;
};

class EvaluationChessCache {
public:
    explicit EvaluationChessCache(std::size_t capacityBytes = 0);
    bool resize(std::size_t capacityBytes);
    std::size_t size() const;
    std::size_t capacityBytes() const;
    std::size_t entryCapacity() const;
    std::size_t clusterCount() const;
    void addToCache(std::uint64_t key, std::int32_t value);
    std::optional<std::int32_t> getFromCache(std::uint64_t key);
    void clear();
    EvaluationCacheStatistics statistics() const;
    void resetStatistics();

#if HOWL_CORRECTNESS_TESTING
    static void SetAllocationFailureThresholdForTesting(std::size_t capacityBytes);
#endif

private:
    struct Entry {
        std::uint64_t key = 0;
        std::int32_t score = 0;
        std::uint32_t valid = 0;
    };

    struct alignas(64) Cluster {
        Entry entries[4];
    };

    static_assert(sizeof(Entry) == 16,
                  "Evaluation cache entries must be exactly 16 bytes");
    static_assert(sizeof(Cluster) == 64,
                  "Evaluation cache clusters must occupy one cache line");
    static_assert(alignof(Cluster) == 64,
                  "Evaluation cache clusters must be cache-line aligned");

    std::unique_ptr<Cluster[]> clusters;
    std::size_t numberOfClusters = 0;
    std::size_t clusterMask = 0;
    std::size_t occupiedEntries = 0;
    std::uint32_t replacementCursor = 0;
#if HOWL_EVAL_CACHE_STATS
    EvaluationCacheStatistics cacheStatistics;
#endif
#if HOWL_CORRECTNESS_TESTING
    static std::size_t allocationFailureThreshold;
#endif
};

#endif
