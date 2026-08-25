#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "ExchangeChessCache.h"

#include <algorithm>
#include <limits>
#include <new>
#include <stdexcept>

namespace
{
constexpr std::uint64_t ExchangeKeyMask = (std::uint64_t{1} << 43) - 1;
constexpr int ExchangeScoreShift = 43;
constexpr std::uint64_t ExchangeScoreMask = std::uint64_t{0xffff} << ExchangeScoreShift;
constexpr std::uint64_t ExchangeValidBit = std::uint64_t{1} << 63;
constexpr std::uint64_t ExchangeMixMultiplier = 0x9e3779b97f4a7c15ULL;
}

#if HOWL_CORRECTNESS_TESTING
std::size_t ExchangeChessCache::allocationFailureThreshold = 0;
#endif

ExchangeChessCache::ExchangeChessCache(std::size_t capacityBytes)
{
    if (!resize(capacityBytes))
    {
        throw std::bad_alloc();
    }
}

bool ExchangeChessCache::resize(std::size_t capacityBytes)
{
    if (capacityBytes == 0)
    {
        sets.reset();
        setCount = 0;
        setMask = 0;
        occupiedEntries = 0;
        resetStatistics();
        return true;
    }
    if (capacityBytes % sizeof(Set) != 0)
    {
        throw std::invalid_argument("Exchange cache capacity must be a multiple of 16 bytes");
    }

    const std::size_t requestedSetCount = capacityBytes / sizeof(Set);
    if ((requestedSetCount & (requestedSetCount - 1)) != 0)
    {
        throw std::invalid_argument("Exchange cache set count must be a power of two");
    }

    sets.reset();
    setCount = 0;
    setMask = 0;
    occupiedEntries = 0;
    resetStatistics();

#if HOWL_CORRECTNESS_TESTING
    if (allocationFailureThreshold != 0 && capacityBytes >= allocationFailureThreshold)
    {
        return false;
    }
#endif

    try
    {
        sets = std::make_unique<Set[]>(requestedSetCount);
    }
    catch (const std::bad_alloc&)
    {
        return false;
    }
    setCount = requestedSetCount;
    setMask = setCount - 1;
    return true;
}

std::size_t ExchangeChessCache::size() const
{
    return occupiedEntries;
}

std::size_t ExchangeChessCache::capacityBytes() const
{
    return setCount * sizeof(Set);
}

std::uint64_t ExchangeChessCache::MixKey(std::uint64_t key)
{
    std::uint64_t mixed = key * ExchangeMixMultiplier;
    return mixed ^ (mixed >> 33);
}

std::uint64_t ExchangeChessCache::Pack(std::uint64_t key, int value)
{
    if (key > ExchangeKeyMask ||
        value < std::numeric_limits<std::int16_t>::min() ||
        value > std::numeric_limits<std::int16_t>::max())
    {
        throw std::out_of_range("Exchange cache key or score is outside the packed range");
    }

    const std::uint16_t scoreBits = static_cast<std::uint16_t>(value);
    return ExchangeValidBit
        | (static_cast<std::uint64_t>(scoreBits) << ExchangeScoreShift)
        | key;
}

int ExchangeChessCache::UnpackScore(std::uint64_t packedValue)
{
    const std::uint16_t scoreBits = static_cast<std::uint16_t>(
        (packedValue & ExchangeScoreMask) >> ExchangeScoreShift);
    if ((scoreBits & std::uint16_t{0x8000}) != 0)
    {
        return static_cast<int>(scoreBits) - 0x10000;
    }
    return static_cast<int>(scoreBits);
}

bool ExchangeChessCache::IsValid(std::uint64_t packedValue)
{
    return (packedValue & ExchangeValidBit) != 0;
}

bool ExchangeChessCache::KeyMatches(std::uint64_t packedValue, std::uint64_t key)
{
    return IsValid(packedValue) && (packedValue & ExchangeKeyMask) == key;
}

std::optional<int> ExchangeChessCache::getFromCache(std::uint64_t key)
{
#if HOWL_EXCHANGE_CACHE_STATS
    cacheStatistics.probes++;
#endif
    if (setCount == 0)
    {
#if HOWL_EXCHANGE_CACHE_STATS
        cacheStatistics.misses++;
#endif
        return std::nullopt;
    }

    const std::uint64_t mixed = MixKey(key);
    const Set& set = sets[mixed & setMask];
    if (KeyMatches(set.ways[0].packedValue, key))
    {
#if HOWL_EXCHANGE_CACHE_STATS
        cacheStatistics.hits++;
#endif
        return UnpackScore(set.ways[0].packedValue);
    }
    if (KeyMatches(set.ways[1].packedValue, key))
    {
#if HOWL_EXCHANGE_CACHE_STATS
        cacheStatistics.hits++;
#endif
        return UnpackScore(set.ways[1].packedValue);
    }

#if HOWL_EXCHANGE_CACHE_STATS
    cacheStatistics.misses++;
#endif
    return std::nullopt;
}

void ExchangeChessCache::addToCache(std::uint64_t key, int value)
{
#if HOWL_EXCHANGE_CACHE_STATS
    cacheStatistics.stores++;
#endif
    if (setCount == 0)
    {
        return;
    }

    const std::uint64_t mixed = MixKey(key);
    Set& set = sets[mixed & setMask];
    Entry* destination = nullptr;

    if (KeyMatches(set.ways[0].packedValue, key))
    {
        destination = &set.ways[0];
    }
    else if (KeyMatches(set.ways[1].packedValue, key))
    {
        destination = &set.ways[1];
    }
    else if (!IsValid(set.ways[0].packedValue))
    {
        destination = &set.ways[0];
        occupiedEntries++;
    }
    else if (!IsValid(set.ways[1].packedValue))
    {
        destination = &set.ways[1];
        occupiedEntries++;
    }
    else
    {
        destination = &set.ways[(mixed >> 32) & 1];
#if HOWL_EXCHANGE_CACHE_STATS
        cacheStatistics.replacements++;
#endif
    }

    destination->packedValue = Pack(key, value);
}

void ExchangeChessCache::clear()
{
    if (setCount != 0)
    {
        std::fill_n(sets.get(), setCount, Set{});
    }
    occupiedEntries = 0;
    resetStatistics();
}

ExchangeCacheStatistics ExchangeChessCache::statistics() const
{
#if HOWL_EXCHANGE_CACHE_STATS
    ExchangeCacheStatistics result = cacheStatistics;
#else
    ExchangeCacheStatistics result;
#endif
    result.uniqueEntries = occupiedEntries;
    return result;
}

void ExchangeChessCache::resetStatistics()
{
#if HOWL_EXCHANGE_CACHE_STATS
    cacheStatistics = {};
#endif
}

#if HOWL_CORRECTNESS_TESTING
void ExchangeChessCache::SetAllocationFailureThresholdForTesting(
    std::size_t capacityBytes)
{
    allocationFailureThreshold = capacityBytes;
}
#endif
