#ifndef MoveLogic_h
#define MoveLogic_h

#include "ExchangeChessCache.h"
#include "Move.h"
#include "Board.h"
#include <cstddef>
#include <cstdint>

struct MoveList {
    Move* moves[256];
    int count = 0;
};

struct DeferredMove {
    const Move* templateMove = nullptr;
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
    static MoveList MoveGenerator(Board &thisBoard, int depth, int depthGone, bool onlyCapturesAndChecks = false);
    static MoveList QSearchStage1Generator(Board &thisBoard, int depth, int depthGone, DeferredMove* deferredMoves, int& deferredCount, const Move& prevMove = Move{});
    static MoveList MaterializeStage2(Board &thisBoard, int depth, int depthGone, const DeferredMove* deferredMoves, int deferredCount);
    static void ScoreAndSortMoves(Board& thisBoard, MoveList& moveList, int depth, int depthGone, const AttackerState& whiteAttacker, const AttackerState& blackAttacker);
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
