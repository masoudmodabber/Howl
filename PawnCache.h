#ifndef PAWNCACHE_H
#define PAWNCACHE_H

#include <unordered_map>
#include <vector>
#include <chrono>
#include <cstddef>
#include <optional>

class PawnCache {
public:

    PawnCache();
    void Add(long key, int value, int i, int j);
    std::optional<int> GetFromCache(long key, int i, int j);

    long getMaxMemoryUsage() const;
    void setMaxMemoryUsage(long value);
    std::size_t size() const;

private:
    static constexpr int PhaseCount = 25;
    std::vector<std::vector<std::unordered_map<long, std::pair<std::chrono::system_clock::time_point, int>>>> caches;
    long currentMemoryUsage = 0;
    long maxMemoryUsage = 1024 * 1024 * 1;

    void EvictLeastRecentlyUsedItems();
};

#endif
