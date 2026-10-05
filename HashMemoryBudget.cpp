#include "HashMemoryBudget.h"

#include "MoveLogic.h"
#include "TranspositionTable.h"

#include <algorithm>
#include <charconv>
#include <ostream>
#include <string_view>

bool HashMemoryBudget::configured = false;
bool HashMemoryBudget::searchStarted = false;
HashMemoryAccounting HashMemoryBudget::accounting{};
std::uint64_t HashMemoryBudget::lastConfigurationPeakTableBytes = 0;

namespace
{
std::string_view Trim(std::string_view value)
{
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
    {
        value.remove_prefix(1);
    }
    while (!value.empty() && (value.back() == ' ' || value.back() == '\t'))
    {
        value.remove_suffix(1);
    }
    return value;
}
}

bool HashMemoryBudget::ConfigureMiB(int requestedMiB, std::ostream& diagnostics)
{
    if (requestedMiB < MinimumHashMiB || requestedMiB > MaximumHashMiB)
    {
        diagnostics << "info string Hash value must be between "
                    << MinimumHashMiB << " and " << MaximumHashMiB << " MiB\n";
        return false;
    }
    if (searchStarted)
    {
        diagnostics << "info string Hash change rejected after search has started; "
                       "resizing while search may be active is not yet supported\n";
        return false;
    }

    const std::uint64_t requestedTotal =
        static_cast<std::uint64_t>(requestedMiB) * Mebibyte;

    // Release largest tables before replacement allocation to avoid transient memory spikes
    TranspositionTable::Resize(0);
    lastConfigurationPeakTableBytes =
        MoveLogic::ExchangeCacheCapacityBytes() +
        MoveLogic::ExchangeWithoutBeginPieceCacheCapacityBytes();

    if (!MoveLogic::ResizeExchangeCache(ExchangeCacheBytes))
    {
        diagnostics << "info string Exchange cache allocation failed; cache disabled\n";
    }
    lastConfigurationPeakTableBytes = std::max(
        lastConfigurationPeakTableBytes,
        static_cast<std::uint64_t>(MoveLogic::ExchangeCacheCapacityBytes()) +
            MoveLogic::ExchangeWithoutBeginPieceCacheCapacityBytes());

    if (!MoveLogic::ResizeExchangeWithoutBeginPieceCache(
            ExchangeWithoutBeginPieceCacheBytes))
    {
        diagnostics << "info string ExchangeWithoutBeginPiece cache allocation failed; "
                       "cache disabled\n";
    }
    lastConfigurationPeakTableBytes = std::max(
        lastConfigurationPeakTableBytes,
        static_cast<std::uint64_t>(MoveLogic::ExchangeCacheCapacityBytes()) +
            MoveLogic::ExchangeWithoutBeginPieceCacheCapacityBytes());

    const std::uint64_t exchangeActual = MoveLogic::ExchangeCacheCapacityBytes();
    const std::uint64_t exchangeWithoutActual =
        MoveLogic::ExchangeWithoutBeginPieceCacheCapacityBytes();
    const std::uint64_t fixedAndEval = NonTableReserveBytes + exchangeActual + exchangeWithoutActual;

    // Allocate TT from remaining unallocated budget bytes
    std::uint64_t ttTargetBytes = 0;
    if (requestedTotal > fixedAndEval)
    {
        ttTargetBytes = requestedTotal - fixedAndEval;
    }

    std::uint64_t actualTT = ttTargetBytes;
    while (actualTT >= sizeof(TTEntry) &&
           !TranspositionTable::Resize(static_cast<std::size_t>(actualTT)))
    {
        actualTT /= 2;
    }
    if (actualTT < sizeof(TTEntry))
    {
        TranspositionTable::Resize(0);
        if (ttTargetBytes >= sizeof(TTEntry))
        {
            diagnostics << "info string TT allocation failed; TT disabled\n";
        }
    }

    const std::uint64_t ttActual = TranspositionTable::CapacityBytes();
    const std::uint64_t combined = exchangeActual + exchangeWithoutActual + ttActual;
    const std::uint64_t envelope = NonTableReserveBytes + combined;
    lastConfigurationPeakTableBytes = std::max(lastConfigurationPeakTableBytes, combined);

    accounting.requestedTotalBytes = requestedTotal;
    accounting.acceptedTotalBytes = requestedTotal;
    accounting.nonTableReserveBytes = NonTableReserveBytes;
    accounting.exchangeCacheBytes = exchangeActual;
    accounting.exchangeWithoutBeginPieceCacheBytes = exchangeWithoutActual;
    accounting.ttBytes = ttActual;
    accounting.combinedTableBytes = combined;
    accounting.plannedEnvelopeBytes = envelope;
    accounting.unallocatedBytes = requestedTotal - envelope;
    configured = true;
    return true;
}

bool HashMemoryBudget::ConfigureValue(const std::string& value,
                                      std::ostream& diagnostics)
{
    const std::string_view trimmed = Trim(value);
    if (trimmed.empty())
    {
        diagnostics << "info string Hash value is missing\n";
        return false;
    }

    int requestedMiB = 0;
    const char* first = trimmed.data();
    const char* last = first + trimmed.size();
    const auto result = std::from_chars(first, last, requestedMiB);
    if (result.ec != std::errc{} || result.ptr != last)
    {
        diagnostics << "info string Hash value is malformed\n";
        return false;
    }
    return ConfigureMiB(requestedMiB, diagnostics);
}

bool HashMemoryBudget::EnsureDefaultConfigured(std::ostream& diagnostics)
{
    return configured || ConfigureMiB(DefaultHashMiB, diagnostics);
}

void HashMemoryBudget::MarkSearchStarted()
{
    searchStarted = true;
}

bool HashMemoryBudget::SearchHasStarted()
{
    return searchStarted;
}

bool HashMemoryBudget::IsConfigured()
{
    return configured;
}

HashMemoryAccounting HashMemoryBudget::Accounting()
{
    return accounting;
}

std::uint64_t HashMemoryBudget::LastConfigurationPeakTableBytes()
{
    return lastConfigurationPeakTableBytes;
}

#if HOWL_CORRECTNESS_TESTING
void HashMemoryBudget::ResetForTesting()
{
    TranspositionTable::Resize(0);
    MoveLogic::ResizeExchangeCache(0);
    MoveLogic::ResizeExchangeWithoutBeginPieceCache(0);
    TranspositionTable::SetAllocationFailureThresholdForTesting(0);
    MoveLogic::SetExchangeCacheAllocationFailureThresholdForTesting(0);
    configured = false;
    searchStarted = false;
    accounting = {};
    lastConfigurationPeakTableBytes = 0;
}
#endif
