#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "PawnCache.h"
#include <algorithm>

PawnCache::PawnCache() : caches(2, std::vector<std::unordered_map<long, std::pair<std::chrono::system_clock::time_point, int>>>(3)) { }

void PawnCache::Add(long key, int value, int i, int j) {
    return;
    long itemSize = sizeof(long) + sizeof(int); // Estimate the size of the item

    if (currentMemoryUsage + itemSize > maxMemoryUsage) {
        EvictLeastRecentlyUsedItems();
    }

    caches[i][j][key] = {std::chrono::system_clock::now(), value};
    currentMemoryUsage += itemSize;
}

void PawnCache::EvictLeastRecentlyUsedItems() {
    // Flatten the 2D array and order by the time_point
    std::vector<std::tuple<int, int, long, std::chrono::system_clock::time_point>> allItems;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (const auto& [key, value] : caches[i][j]) {
                allItems.emplace_back(i, j, key, value.first);
            }
        }
    }

    std::sort(allItems.begin(), allItems.end(), [](const auto& a, const auto& b) {
        return std::get<3>(a) < std::get<3>(b);
    });

    // Calculate the number of items to remove (30% of total items)
    int itemsToRemoveCount = static_cast<int>(allItems.size() * 0.3);

    // Remove the least recently used items
    for (int k = 0; k < itemsToRemoveCount; ++k) {
        const auto& [i, j, key, _] = allItems[k];
        caches[i][j].erase(key);
        currentMemoryUsage -= sizeof(long) + sizeof(int); // Estimate the size of the item
    }
}

std::optional<int> PawnCache::GetFromCache(long key, int i, int j) {
    auto it = caches[i][j].find(key);
    if (it != caches[i][j].end()) {
        // Update the time_point to the current time
        it->second.first = std::chrono::system_clock::now();

        // Return the value
        return it->second.second;
    }

    // If the key is not found, return nullopt
    return std::nullopt;
}

long PawnCache::getMaxMemoryUsage() const {
    return maxMemoryUsage;
}

void PawnCache::setMaxMemoryUsage(long value) {
    maxMemoryUsage = value;
}

std::size_t PawnCache::size() const {
    std::size_t entryCount = 0;
    for (const auto& sideCaches : caches) {
        for (const auto& phaseCache : sideCaches) {
            entryCount += phaseCache.size();
        }
    }
    return entryCount;
}
