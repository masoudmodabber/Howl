#ifndef HASHMEMORYBUDGET_H
#define HASHMEMORYBUDGET_H

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>

struct HashMemoryAccounting
{
    std::uint64_t requestedTotalBytes = 0;
    std::uint64_t acceptedTotalBytes = 0;
    std::uint64_t nonTableReserveBytes = 0;
    std::uint64_t evalCacheBytes = 0;
    std::uint64_t exchangeCacheBytes = 0;
    std::uint64_t exchangeWithoutBeginPieceCacheBytes = 0;
    std::uint64_t ttBytes = 0;
    std::uint64_t combinedTableBytes = 0;
    std::uint64_t plannedEnvelopeBytes = 0;
    std::uint64_t unallocatedBytes = 0;
};

class HashMemoryBudget
{
public:
    static constexpr std::uint64_t Mebibyte = 1024ULL * 1024ULL;
    static constexpr std::uint64_t Kibibyte = 1024ULL;
    static constexpr int MinimumHashMiB = 8;
    static constexpr int MaximumHashMiB = 1024;
    static constexpr int DefaultHashMiB = 40;
    static constexpr std::uint64_t NonTableReserveBytes = 6ULL * Mebibyte;
    static constexpr std::uint64_t ExchangeCacheBytes = 256ULL * Kibibyte;
    static constexpr std::uint64_t ExchangeWithoutBeginPieceCacheBytes =
        64ULL * Kibibyte;
    static constexpr std::uint64_t MaximumEvalCacheBytes = 8ULL * Mebibyte;
    static constexpr std::uint64_t MinimumEvalCacheBytes = 64ULL;

    static bool ConfigureMiB(int requestedMiB, std::ostream& diagnostics);
    static bool ConfigureValue(const std::string& value, std::ostream& diagnostics);
    static bool EnsureDefaultConfigured(std::ostream& diagnostics);
    static void MarkSearchStarted();
    static bool SearchHasStarted();
    static bool IsConfigured();
    static HashMemoryAccounting Accounting();
    static std::uint64_t SelectedEvalBytes(int requestedMiB);
    static std::uint64_t LastConfigurationPeakTableBytes();

#if HOWL_CORRECTNESS_TESTING
    static void ResetForTesting();
#endif

private:
    static bool configured;
    static bool searchStarted;
    static HashMemoryAccounting accounting;
    static std::uint64_t lastConfigurationPeakTableBytes;
};

#endif
