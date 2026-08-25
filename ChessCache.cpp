#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "ChessCache.h"

long ChessCache::getMaxMemoryUsage() { 
    return maxMemoryUsage; 
}

void ChessCache::setMaxMemoryUsage(long value) { 
    maxMemoryUsage = value; 
}

std::size_t ChessCache::size() const
{
    return evalCache.size();
}

void ChessCache::addToCache(long key, int value)
{
    long itemSize = sizeof(long) + sizeof(int); // Estimate the size of the item

    if (currentMemoryUsage + itemSize > maxMemoryUsage)
    {
        evictLeastRecentlyUsedItems();
    }

    evalCache[key] = {std::chrono::system_clock::now(), value};
    currentMemoryUsage += itemSize;
}

void ChessCache::evictLeastRecentlyUsedItems()
{
    // Find the least recently used 30% of items
    int itemsToRemoveCount = evalCache.size() * 0.3;
    auto it = evalCache.begin();
    for (int i = 0; i < itemsToRemoveCount; ++i)
    {
        currentMemoryUsage -= sizeof(long) + sizeof(int); // Estimate the size of the item
        it = evalCache.erase(it);
    }
}

std::optional<int> ChessCache::getFromCache(long key)
{
    auto it = evalCache.find(key);
    return it != evalCache.end() ? std::optional<int>(it->second.second) : std::nullopt;
}
