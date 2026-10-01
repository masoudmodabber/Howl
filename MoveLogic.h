#ifndef MoveLogic_h
#define MoveLogic_h

#include "ExchangeChessCache.h"
#include "Move.h"
#include "Board.h"
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <new>
#include <type_traits>

struct MoveList {
    static constexpr int Capacity = 256;
    using Slot = std::aligned_storage_t<sizeof(Move), alignof(Move)>;
    Slot storage[Capacity];
    int count;

    MoveList() noexcept : count(0) {}
    MoveList(const MoveList& other) noexcept : count(other.count)
    {
        if (count > 0)
            std::memcpy(storage, other.storage, static_cast<std::size_t>(count) * sizeof(Slot));
    }
    MoveList(MoveList&& other) noexcept : count(other.count)
    {
        if (count > 0)
            std::memcpy(storage, other.storage, static_cast<std::size_t>(count) * sizeof(Slot));
    }
    ~MoveList() = default;

    MoveList& operator=(const MoveList& other) noexcept
    {
        if (this != &other)
        {
            count = other.count;
            if (count > 0)
                std::memcpy(storage, other.storage, static_cast<std::size_t>(count) * sizeof(Slot));
        }
        return *this;
    }
    MoveList& operator=(MoveList&& other) noexcept { return *this = other; }

    Move* At(int index) noexcept
    {
        assert(index >= 0 && index < Capacity);
        return reinterpret_cast<Move*>(&storage[index]);
    }
    const Move* At(int index) const noexcept
    {
        assert(index >= 0 && index < Capacity);
        return reinterpret_cast<const Move*>(&storage[index]);
    }

    Move& operator[](int index) noexcept { return *At(index); }
    const Move& operator[](int index) const noexcept { return *At(index); }

    Move* Get(int index) noexcept { return At(index); }
    const Move* Get(int index) const noexcept { return At(index); }

    Move* begin() noexcept { return At(0); }
    const Move* begin() const noexcept { return At(0); }
    Move* end() noexcept { return At(count); }
    const Move* end() const noexcept { return At(count); }

    void Clear() noexcept { count = 0; }

    Move* AppendCopy(const Move* source) noexcept
    {
        assert(count < Capacity);
        Move* result = ::new (&storage[count++]) Move{};
        result->beginPlace = source->beginPlace;
        result->CastleFlag = source->CastleFlag;
        result->endPlace = source->endPlace;
        result->promotionPiece = source->promotionPiece;
        result->PublicFlag = source->PublicFlag;
        result->unpassentPlace = source->unpassentPlace;
        return result;
    }

    Move* AppendValue(const Move& source) noexcept
    {
        assert(count < Capacity);
        Move* result = ::new (&storage[count++]) Move(source);
        return result;
    }

    void Discard(Move* candidate) noexcept
    {
        assert(count > 0 && candidate == At(count - 1));
        --count;
    }
};

struct DeferredMove {
    Move templateMove{};
    int endPiece = 0;
    int customValue = 0;
    bool hasCustomValue = false;
};

struct AttackerState {
    std::uint32_t pieceCounts[64] = {};
    int orderingScores[64] = {};
};

class MoveLogic
{
public:
    static void Initialize();
    static MoveList MoveGenerator(Board &thisBoard, int depth, int depthGone, bool onlyCapturesAndChecks = false, bool includeQuietChecks = true);
    static MoveList MoveGenerator(Board &thisBoard, int depth, int depthGone, bool onlyCapturesAndChecks, bool scoreAndSort, const AttackerState& whiteAttacker, const AttackerState& blackAttacker, bool includeQuietChecks = true);
    static void MoveGeneratorInto(Board &thisBoard, int depth, int depthGone, bool onlyCapturesAndChecks, bool scoreAndSort, const AttackerState& whiteAttacker, const AttackerState& blackAttacker, MoveList& moveList, bool includeQuietChecks = true);
    static MoveList QSearchStage1Generator(Board &thisBoard, int depth, int depthGone, DeferredMove* deferredMoves, int& deferredCount, const Move& prevMove = Move{}, bool includeQuietChecks = true, bool deepResolution = false);
    static MoveList MaterializeStage2(Board &thisBoard, int depth, int depthGone, const DeferredMove* deferredMoves, int deferredCount);
    static bool HasAnyLegalMove(Board &thisBoard, const Move& prevMove, int depthGone);
    static void ScoreAndSortMoves(Board& thisBoard, MoveList& moveList, int depth, int depthGone, const AttackerState& whiteAttacker, const AttackerState& blackAttacker);
    static void ScoreMove(Board& thisBoard, Move& move, const AttackerState& whiteAttacker, const AttackerState& blackAttacker);
    static bool SEE_GE(Board& thisBoard, Move& move, int threshold);
    static bool MoveGivesCheck(Board& thisBoard, const Move& move);
    static AttackerState SetWhiteAttacker(Board &thisBoard);
    static AttackerState SetBlackAttacker(Board &thisBoard);
    static Move *MoveCopy(Move *move);
    static int Exchange(std::uint32_t attacker, std::uint32_t defender, int attackPlace, int beginPiece, int endPiece, int promotionPiece);
    static int ExchangeWithoutBeginPiece(std::uint32_t attacker, std::uint32_t defender, int attackPlace, int beginPiece, int endPiece, int promotionPiece);
    static bool Same(Move &move2, Move &move3, Move &move4, Move &move);
    static void Cleanup();
    static std::size_t ExchangeCacheSize();
    static std::size_t ExchangeWithoutBeginPieceCacheSize();
    static ExchangeCacheStatistics ExchangeCacheStats();
    static ExchangeCacheStatistics ExchangeWithoutBeginPieceCacheStats();
    static void ResetExchangeCacheStats();
    static bool ResizeExchangeCache(std::size_t capacityBytes);
    static bool ResizeExchangeWithoutBeginPieceCache(std::size_t capacityBytes);
    static std::size_t ExchangeCacheCapacityBytes();
    static std::size_t ExchangeWithoutBeginPieceCacheCapacityBytes();
#if HOWL_CORRECTNESS_TESTING
    static void SetExchangeCacheAllocationFailureThresholdForTesting(
        std::size_t capacityBytes);
#endif

private:
    static double pieceValue[15];
    static int pieceMoveStack[15];
    static ExchangeChessCache ExchangeCache;
    static ExchangeChessCache ExchangeCacheWithoutBeginPiece;
    static bool initialized;
};

#endif
