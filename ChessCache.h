#ifndef CHESSCACHE_H
#define CHESSCACHE_H

#include <map>
#include <chrono>
#include <optional>

class ChessCache {
public:
    std::map<long, std::pair<std::chrono::system_clock::time_point, int> > evalCache;
    long currentMemoryUsage = 0;
    long maxMemoryUsage = 1024*1024*10;
    long getMaxMemoryUsage();
    void setMaxMemoryUsage(long value);
    void addToCache(long key, int value);
    std::optional<int> getFromCache(long key);

private:
    void evictLeastRecentlyUsedItems();
};

#endif