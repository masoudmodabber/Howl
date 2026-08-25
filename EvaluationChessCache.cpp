#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "EvaluationChessCache.h"

#include <algorithm>
#include <new>
#include <stdexcept>

#if HOWL_CORRECTNESS_TESTING
std::size_t EvaluationChessCache::allocationFailureThreshold = 0;
#endif

EvaluationChessCache::EvaluationChessCache(std::size_t capacityBytes)
{
    if (!resize(capacityBytes))
    {
        throw std::bad_alloc();
    }
}

bool EvaluationChessCache::resize(std::size_t capacityBytes)
{
    if (capacityBytes == 0)
    {
        clusters.reset();
        numberOfClusters = 0;
        clusterMask = 0;
        occupiedEntries = 0;
        replacementCursor = 0;
        resetStatistics();
        return true;
    }
    if (capacityBytes % sizeof(Cluster) != 0)
    {
        throw std::invalid_argument(
            "Evaluation cache capacity must be a multiple of 64 bytes");
    }

    const std::size_t requestedClusters = capacityBytes / sizeof(Cluster);
    if ((requestedClusters & (requestedClusters - 1)) != 0)
    {
        throw std::invalid_argument(
            "Evaluation cache cluster count must be a power of two");
    }

    clusters.reset();
    numberOfClusters = 0;
    clusterMask = 0;
    occupiedEntries = 0;
    replacementCursor = 0;
    resetStatistics();

#if HOWL_CORRECTNESS_TESTING
    if (allocationFailureThreshold != 0 && capacityBytes >= allocationFailureThreshold)
    {
        return false;
    }
#endif

    try
    {
        clusters = std::make_unique<Cluster[]>(requestedClusters);
    }
    catch (const std::bad_alloc&)
    {
        return false;
    }
    numberOfClusters = requestedClusters;
    clusterMask = numberOfClusters - 1;
    return true;
}

std::size_t EvaluationChessCache::size() const
{
    return occupiedEntries;
}

std::size_t EvaluationChessCache::capacityBytes() const
{
    return numberOfClusters * sizeof(Cluster);
}

std::size_t EvaluationChessCache::entryCapacity() const
{
    return numberOfClusters * 4;
}

std::size_t EvaluationChessCache::clusterCount() const
{
    return numberOfClusters;
}

std::optional<std::int32_t> EvaluationChessCache::getFromCache(std::uint64_t key)
{
#if HOWL_EVAL_CACHE_STATS
    cacheStatistics.probes++;
#endif
    if (numberOfClusters == 0)
    {
#if HOWL_EVAL_CACHE_STATS
        cacheStatistics.misses++;
#endif
        return std::nullopt;
    }

    const Cluster& cluster = clusters[key & clusterMask];
    for (const Entry& entry : cluster.entries)
    {
        if (entry.valid != 0 && entry.key == key)
        {
#if HOWL_EVAL_CACHE_STATS
            cacheStatistics.hits++;
#endif
            return entry.score;
        }
    }

#if HOWL_EVAL_CACHE_STATS
    cacheStatistics.misses++;
#endif
    return std::nullopt;
}

void EvaluationChessCache::addToCache(std::uint64_t key, std::int32_t value)
{
#if HOWL_EVAL_CACHE_STATS
    cacheStatistics.stores++;
#endif
    if (numberOfClusters == 0)
    {
        return;
    }

    Cluster& cluster = clusters[key & clusterMask];
    Entry* destination = nullptr;
    for (Entry& entry : cluster.entries)
    {
        if (entry.valid != 0 && entry.key == key)
        {
            destination = &entry;
            break;
        }
        if (destination == nullptr && entry.valid == 0)
        {
            destination = &entry;
        }
    }

    if (destination == nullptr)
    {
        destination = &cluster.entries[replacementCursor & 3];
        replacementCursor++;
#if HOWL_EVAL_CACHE_STATS
        cacheStatistics.replacements++;
#endif
    }
    else if (destination->valid == 0)
    {
        occupiedEntries++;
    }

    destination->key = key;
    destination->score = value;
    destination->valid = 1;
}

void EvaluationChessCache::clear()
{
    if (numberOfClusters != 0)
    {
        std::fill_n(clusters.get(), numberOfClusters, Cluster{});
    }
    occupiedEntries = 0;
    replacementCursor = 0;
    resetStatistics();
}

EvaluationCacheStatistics EvaluationChessCache::statistics() const
{
#if HOWL_EVAL_CACHE_STATS
    EvaluationCacheStatistics result = cacheStatistics;
#else
    EvaluationCacheStatistics result;
#endif
    result.uniqueEntries = occupiedEntries;
    return result;
}

void EvaluationChessCache::resetStatistics()
{
#if HOWL_EVAL_CACHE_STATS
    cacheStatistics = {};
#endif
}

#if HOWL_CORRECTNESS_TESTING
void EvaluationChessCache::SetAllocationFailureThresholdForTesting(
    std::size_t capacityBytes)
{
    allocationFailureThreshold = capacityBytes;
}
#endif
