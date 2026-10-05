#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "PVSSearch.h"
#include "Search.h"
#include "BoardLogic.h"
#include "GamePhase.h"
#include "StructuredNNUEEvaluator.h"
#include "QSearcher.h"
#include "MoveLogic.h"
#include "UCI.h"
#include "GameLogic.h"
#include "ChessStringManipulation.h"
#include "MissingInfoAboutPrevStateFromMove.h"
#include "RepetitionHistory.h"
#include "TranspositionTable.h"
#include "MateScore.h"
#include "Tablebase.h"
#include "Option.h"
#include "SearchParameters.h"
#include "MoveOrdering.h"
#include <iostream>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstdint>
#include <vector>

int PVSSearch::moveOrderingDepth[20] = {
    1,
    2,
    2,
    3,
    3,
    4,
    4,
    5,
    5,
    6,
    6,
    7,
    7,
    8,
    8,
    9,
    9,
    10};

PVSSearch::KillerMove PVSSearch::killers[PVSSearch::MaxKillerPly][2] = {};

namespace
{
    constexpr int StaticPruningMaxDepth = 5;
    constexpr int RazoringMaxDepth = SearchParameters::Razoring::MaxDepth;
    constexpr int RazorMargin = SearchParameters::Razoring::Margin;
    constexpr int ProbCutMinDepth = SearchParameters::ProbCut::MinDepth;
    constexpr int ProbCutMaxCandidatesBase = 2;
    constexpr int MaxReductionIndex = 255;

    constexpr int MainHistoryLimit = SearchParameters::History::MainLimit;
    constexpr int CaptureHistoryLimit = SearchParameters::History::CaptureLimit;
    constexpr int ContinuationHistoryLimit = SearchParameters::History::ContinuationLimit;
    constexpr int SearchStackBack = 8;
    constexpr int SearchStackSize = 256;
    constexpr int NoStaticEval = -200000;
    int mainHistory[2][64][64] = {};
    int captureHistory[15][64][7] = {};
    uint16_t counterMoves[15][64] = {};
    constexpr int TtHitAverageWindow = 4096;
    int ttHitAverage = TtHitAverageWindow * 1024 / 2;
    int nullSuppressedSide = -1;
    int nullSuppressedUntilPly = -1;

    const std::array<int, MaxReductionIndex + 1> Reductions = [] {
        std::array<int, MaxReductionIndex + 1> values{};
        for (int i = 1; i <= MaxReductionIndex; ++i)
            values[i] = static_cast<int>(SearchParameters::LMR::LogarithmicScale *
                std::log(static_cast<double>(i)));
        return values;
    }();

    int Reduction(bool improving, int depth, int moveNumber)
    {
        const int d = std::clamp(depth, 1, MaxReductionIndex);
        const int m = std::clamp(moveNumber, 1, MaxReductionIndex);
        const int raw = Reductions[d] * Reductions[m];
        return (raw + SearchParameters::LMR::RoundingTerm) /
            SearchParameters::LMR::Divisor +
            (!improving && raw > SearchParameters::LMR::NonImprovingThreshold);
    }

    int FutilityMargin(int depth, bool improving) { return SearchParameters::ReverseFutility::DepthMargin * (depth - int(improving)); }
    int FutilityMoveCount(bool improving, int depth)
    {
        return (SearchParameters::MoveCountPruning::Base +
                SearchParameters::MoveCountPruning::DepthQuadratic * depth * depth) *
            (SearchParameters::MoveCountPruning::ImprovingBase + int(improving)) /
            SearchParameters::MoveCountPruning::Divisor -
            SearchParameters::MoveCountPruning::Offset;
    }
    int StatBonus(int depth)
    {
        return depth > SearchParameters::History::BonusDeepDepth
            ? SearchParameters::History::BonusDeepValue
            : SearchParameters::History::BonusQuadratic * depth * depth
              + SearchParameters::History::BonusLinear * depth
              + SearchParameters::History::BonusConstant;
    }

    constexpr std::size_t ContinuationTableSize = 15u * 64u * 15u * 64u;
    std::array<int, ContinuationTableSize> continuationHistory{};

    struct SearchStackFrame
    {
        int ply = 0;
        Move currentMove{};
        bool hasCurrentMove = false;
        int currentMovedPiece = 0;
        uint16_t excludedMove = 0;
        int staticEval = NoStaticEval;
        int statScore = 0;
        int moveCount = 0;
        PVSSearch::KillerMove* killer1 = nullptr;
        PVSSearch::KillerMove* killer2 = nullptr;
        int* continuationContext = nullptr;
        bool pvNode = false;
    };

    std::array<SearchStackFrame, SearchStackSize + SearchStackBack> searchStack{};

    struct TriedMoveStorage
    {
        std::array<Move*, MoveList::Capacity> quiet{};
        std::array<Move*, MoveList::Capacity> captures{};
        int quietCount = 0;
        int captureCount = 0;
    };

    std::array<TriedMoveStorage, SearchStackSize + SearchStackBack> triedMoveStack{};
    int targetSearchFrameDepth = 0;

    class TargetSearchFrameGuard
    {
    public:
        TargetSearchFrameGuard()
        {
            if (targetSearchFrameDepth >= static_cast<int>(triedMoveStack.size()))
            {
                std::cerr << "TargetSearch frame storage overflow\n";
                std::abort();
            }
            storage = &triedMoveStack[targetSearchFrameDepth++];
            storage->quietCount = 0;
            storage->captureCount = 0;
        }

        ~TargetSearchFrameGuard() { --targetSearchFrameDepth; }

        TriedMoveStorage& Get() { return *storage; }

    private:
        TriedMoveStorage* storage = nullptr;
    };

    void AppendTriedMove(std::array<Move*, MoveList::Capacity>& moves,
                         int& count, Move* move, const char* kind)
    {
        if (count >= static_cast<int>(moves.size()))
        {
            std::cerr << kind << " tried-move storage overflow\n";
            std::abort();
        }
        moves[count++] = move;
    }

    SearchStackFrame& StackFrame(int ply)
    {
        static SearchStackFrame neutral{};
        const int index = ply + SearchStackBack;
        return index >= 0 && index < static_cast<int>(searchStack.size())
            ? searchStack[index] : neutral;
    }

    const Move* StackMove(int ply)
    {
        const SearchStackFrame& frame = StackFrame(ply);
        return frame.hasCurrentMove ? &frame.currentMove : nullptr;
    }

    void ResetSearchStack()
    {
        for (int ply = -SearchStackBack; ply < SearchStackSize; ++ply)
        {
            SearchStackFrame& frame = StackFrame(ply);
            frame = SearchStackFrame{};
            frame.ply = ply;
            frame.currentMove.beginPlace = -1;
            frame.currentMove.endPlace = -1;
            frame.currentMove.promotionPiece = -2;
        }
        nullSuppressedSide = -1;
        nullSuppressedUntilPly = -1;
        ttHitAverage = TtHitAverageWindow * 1024 / 2;
    }

    bool IsQuietMove(const Move &move)
    {
        return move.endPiece == 0 && move.promotionPiece <= 0;
    }

    int HistoryScore(int side, const Move &move)
    {
        return mainHistory[side][move.beginPlace][move.endPlace];
    }

    std::size_t ContinuationIndex(int previousPiece, int previousTo,
                                  int currentPiece, int currentTo)
    {
        return (((static_cast<std::size_t>(previousPiece) * 64 + previousTo) * 15 +
                 currentPiece) * 64 + currentTo);
    }

    int ContinuationHistoryScoreAt(int ply, int offset, int currentPiece,
                                   const Move& move)
    {
        // Frame P contains the move that arrived from P-1. Therefore offset N
        // is frame P-(N-1), giving exactly P-1, P-2, P-4 and P-6 contexts.
        const SearchStackFrame& previous = StackFrame(ply - (offset - 1));
        if (!previous.hasCurrentMove || previous.currentMovedPiece <= 0 ||
            move.beginPlace < 0 || move.beginPlace >= 64)
            return 0;
        const int piece = std::clamp(currentPiece, 1, 14);
        return continuationHistory[ContinuationIndex(previous.currentMovedPiece,
            previous.currentMove.endPlace, piece, move.endPlace)];
    }

    struct QuietHistoryValues
    {
        int main = 0;
        int c1 = 0;
        int c2 = 0;
        int c4 = 0;
        int c6 = 0;

        int orderingScore() const { return main + 2*c1 + 2*c2 + 2*c4 + c6; }
        int combinedHistory() const { return main + c1 + c2 + c4; }
        int statScore() const
        {
        int score = combinedHistory() - SearchParameters::History::LmrStatOffset;
            if (score < 0 && c1 >= 0 && c2 >= 0 && main >= 0)
                score = 0;
            return score;
        }
    };

    QuietHistoryValues ReadQuietHistory(int side, int ply, int currentPiece,
                                        const Move& move)
    {
        return {HistoryScore(side, move),
            ContinuationHistoryScoreAt(ply, 1, currentPiece, move),
            ContinuationHistoryScoreAt(ply, 2, currentPiece, move),
            ContinuationHistoryScoreAt(ply, 4, currentPiece, move),
            ContinuationHistoryScoreAt(ply, 6, currentPiece, move)};
    }

    void UpdateHistory(int side, const Move &move, int depth, bool improvedAlpha)
    {
        const int magnitude = StatBonus(depth);
        const int bonus = improvedAlpha ? magnitude : -magnitude;
        int &score = mainHistory[side][move.beginPlace][move.endPlace];
        score += bonus - score * std::abs(bonus) / MainHistoryLimit;
    }

    void UpdateHistoryByBonus(int side, const Move& move, int bonus)
    {
        int& score = mainHistory[side][move.beginPlace][move.endPlace];
        score += bonus - score * std::abs(bonus) / MainHistoryLimit;
    }

    void UpdateContinuationHistory(int previousPiece, int previousTo,
                                   int currentPiece, const Move &move,
                                   int depth, bool improvedAlpha)
    {
        if (previousPiece <= 0 || previousTo < 0 || currentPiece <= 0)
            return;
        int& entry = continuationHistory[ContinuationIndex(previousPiece,
            previousTo, currentPiece, move.endPlace)];
        const int magnitude = StatBonus(depth);
        const int bonus = improvedAlpha ? magnitude : -magnitude;
        entry += bonus - entry * std::abs(bonus) / ContinuationHistoryLimit;
    }

    void UpdateContinuationHistories(int ply, int currentPiece, const Move& move,
                                     int depth, bool success)
    {
        for (int offset : {1, 2, 4, 6})
        {
            const SearchStackFrame& previous = StackFrame(ply - (offset - 1));
            if (previous.hasCurrentMove)
                UpdateContinuationHistory(previous.currentMovedPiece,
                    previous.currentMove.endPlace,
                    std::clamp(currentPiece, 1, 14),
                    move, depth, success);
        }
    }

    void UpdateContinuationHistoriesByBonus(int ply, int currentPiece,
                                             const Move& move, int bonus)
    {
        for (int offset : {1, 2, 4, 6})
        {
            const SearchStackFrame& previous = StackFrame(ply - (offset - 1));
            if (!previous.hasCurrentMove || previous.currentMovedPiece <= 0)
                continue;
            int& entry = continuationHistory[ContinuationIndex(
                previous.currentMovedPiece, previous.currentMove.endPlace,
                std::clamp(currentPiece, 1, 14), move.endPlace)];
            entry += bonus - entry * std::abs(bonus) / ContinuationHistoryLimit;
        }
    }

    int QuietOrderingScore(int side, int ply, int currentPiece, const Move& move)
    {
        return ReadQuietHistory(side, ply, currentPiece, move).orderingScore();
    }

    int QuietCombinedHistory(int side, int ply, int currentPiece, const Move& move)
    {
        return HistoryScore(side, move)
            + ContinuationHistoryScoreAt(ply, 1, currentPiece, move)
            + ContinuationHistoryScoreAt(ply, 2, currentPiece, move)
            + ContinuationHistoryScoreAt(ply, 4, currentPiece, move);
    }

    int QuietStatScore(int side, int ply, int currentPiece, const Move& move)
    {
        const int main = HistoryScore(side, move);
        const int c1 = ContinuationHistoryScoreAt(ply, 1, currentPiece, move);
        const int c2 = ContinuationHistoryScoreAt(ply, 2, currentPiece, move);
        int score = main + c1 + c2 +
            ContinuationHistoryScoreAt(ply, 4, currentPiece, move) -
            SearchParameters::History::LmrStatOffset;
        if (score < 0 && c1 >= 0 && c2 >= 0 && main >= 0)
            score = 0;
        return score;
    }

    void UpdateCaptureHistory(const Board& board, const Move& move, int depth,
                              bool success)
    {
        const int movingPiece = std::clamp(static_cast<int>(board.positionCore.pieceAt[move.beginPlace]), 0, 14);
        const int capturedPiece = std::clamp(move.endPiece % 8, 0, 6);
        int& score = captureHistory[movingPiece][move.endPlace][capturedPiece];
        const int magnitude = StatBonus(depth);
        const int bonus = success ? magnitude : -magnitude;
        score += bonus - score * std::abs(bonus) / CaptureHistoryLimit;
    }

    uint16_t CounterMoveFor(const Board& board, int ply)
    {
        const Move* previous = StackMove(ply);
        if (!previous || previous->endPlace < 0 || previous->endPlace >= 64)
            return 0;
        const SearchStackFrame& frame = StackFrame(ply);
        return counterMoves[std::clamp(frame.currentMovedPiece, 0, 14)]
                           [previous->endPlace];
    }

    void StoreCounterMove(const Board& board, int ply, const Move& move)
    {
        const Move* previous = StackMove(ply);
        if (!previous || previous->endPlace < 0 || previous->endPlace >= 64)
            return;
        counterMoves[std::clamp(StackFrame(ply).currentMovedPiece, 0, 14)]
                    [previous->endPlace] = TTMoveHelper::PackMove(move);
    }

    void RewardCutoffMove(const Board& board, int side, int ply, int depth,
                          int bestValue, int beta,
                          const Move& move,
                          Move* const* searchedQuiets, int searchedQuietCount,
                          Move* const* searchedCaptures, int searchedCaptureCount)
    {
        const int bonus1 = StatBonus(depth + 1);
        const int bonus2 = bestValue > beta + Option::PawnValue
            ? bonus1 : StatBonus(depth);
        if (IsQuietMove(move))
        {
            PVSSearch::RecordKiller(ply, move);
            UpdateHistoryByBonus(side, move, bonus2);
            UpdateContinuationHistoriesByBonus(
                ply, board.positionCore.pieceAt[move.beginPlace], move, bonus2);
            StoreCounterMove(board, ply, move);
            const Move* previous = StackMove(ply);
            if (previous &&
                (StackFrame(ply - 1).moveCount == 1 ||
                 (ply>0&&ply-1<PVSSearch::MaxKillerPly&&
                  PVSSearch::killers[ply-1][0]==*previous)) &&
                previous->endPlace >= 0 && previous->endPlace < 64 &&
                previous->endPiece == 0 && previous->promotionPiece <= 0)
                UpdateContinuationHistoriesByBonus(
                    ply - 1, StackFrame(ply).currentMovedPiece,
                    *previous, -bonus1);
            for (int i = 0; i < searchedQuietCount; ++i)
            {
                Move* prior = searchedQuiets[i];
                if (prior != &move)
                {
                    UpdateHistoryByBonus(side, *prior, -bonus2);
                    UpdateContinuationHistoriesByBonus(
                        ply, board.positionCore.pieceAt[prior->beginPlace], *prior, -bonus2);
                }
            }
            if (board.positionCore.pieceAt[move.beginPlace] % 8 != 1)
            {
                Move reverse = move;
                std::swap(reverse.beginPlace, reverse.endPlace);
                UpdateHistoryByBonus(side, reverse, -bonus2);
            }
        }
        else
        {
            const int movingPiece = std::clamp(static_cast<int>(board.positionCore.pieceAt[move.beginPlace]), 0, 14);
            const int capturedPiece = std::clamp(move.endPiece % 8, 0, 6);
            int& winning = captureHistory[movingPiece][move.endPlace][capturedPiece];
            winning += bonus1 - winning * std::abs(bonus1) / CaptureHistoryLimit;
            for (int i = 0; i < searchedCaptureCount; ++i)
            {
                Move* prior = searchedCaptures[i];
                if (prior != &move)
                {
                    const int priorPiece = std::clamp(static_cast<int>(board.positionCore.pieceAt[prior->beginPlace]), 0, 14);
                    const int priorCaptured = std::clamp(prior->endPiece % 8, 0, 6);
                    int& failed = captureHistory[priorPiece][prior->endPlace][priorCaptured];
                    failed += -bonus1 - failed * std::abs(bonus1) / CaptureHistoryLimit;
                }
            }
        }
    }

    void RewardCutoffMove(const Board& board, int side, int ply, int depth,
                          int bestValue, int beta,
                          const Move& move,
                          const std::vector<Move*>& searchedQuiets,
                          const std::vector<Move*>& searchedCaptures)
    {
        RewardCutoffMove(board, side, ply, depth, bestValue, beta, move,
                         searchedQuiets.data(), static_cast<int>(searchedQuiets.size()),
                         searchedCaptures.data(), static_cast<int>(searchedCaptures.size()));
    }

    void RewardCutoffMove(const Board& board, int side, int ply, int depth,
                          const Move& move,
                          const std::vector<Move*>& searchedQuiets,
                          const std::vector<Move*>& searchedCaptures)
    {
        RewardCutoffMove(board, side, ply, depth, 0, 0, move,
                         searchedQuiets, searchedCaptures);
    }

    int CombinedHistoryScore(int side, const Move &previousMove, const Move &move)
    {
        (void)previousMove;
        return HistoryScore(side, move);
    }

    bool MatchesPackedMove(const Move& move, uint16_t packedMove)
    {
        if (packedMove == 0)
            return false;
        return move.beginPlace == TTMoveHelper::UnpackFrom(packedMove) &&
            move.endPlace == TTMoveHelper::UnpackTo(packedMove) &&
            (TTMoveHelper::UnpackPromotion(packedMove) == 0
                ? move.promotionPiece <= 0
                : move.promotionPiece == TTMoveHelper::UnpackPromotion(packedMove));
    }

    uint64_t SearchKey(uint64_t positionKey, uint16_t excludedMove)
    {
        if (excludedMove == 0)
            return positionKey;
        uint64_t component = static_cast<uint64_t>(excludedMove) + 0x9e3779b97f4a7c15ULL;
        component ^= component >> 30;
        component *= 0xbf58476d1ce4e5b9ULL;
        component ^= component >> 27;
        component *= 0x94d049bb133111ebULL;
        component ^= component >> 31;
        return positionKey ^ component;
    }

    Move UnpackMove(uint16_t packed)
    {
        Move move{};
        move.beginPlace = TTMoveHelper::UnpackFrom(packed);
        move.endPlace = TTMoveHelper::UnpackTo(packed);
        move.promotionPiece = TTMoveHelper::UnpackPromotion(packed);
        move.endPiece = 0;
        return move;
    }

    void PrepareChildStack(int childPly, const Move& move, int movedPiece, int statScore)
    {
        SearchStackFrame& child = StackFrame(childPly);
        child = SearchStackFrame{};
        child.ply = childPly;
        child.currentMove = move;
        child.hasCurrentMove = move.beginPlace >= 0 && move.endPlace >= 0 &&
            move.promotionPiece != -1;
        child.currentMovedPiece = std::clamp(movedPiece, 0, 14);
        child.statScore = statScore;
        const SearchStackFrame& previous = StackFrame(childPly - 1);
        if (previous.hasCurrentMove && previous.currentMovedPiece > 0 && movedPiece > 0)
            child.continuationContext = &continuationHistory[ContinuationIndex(
                previous.currentMovedPiece, previous.currentMove.endPlace,
                movedPiece, move.endPlace)];
    }

    bool IsKillerMove(int ply, const Move& move)
    {
        if (ply < 0 || ply >= PVSSearch::MaxKillerPly)
            return false;
        return PVSSearch::killers[ply][0] == move ||
            PVSSearch::killers[ply][1] == move;
    }

    int ContextualLMRReduction(int depth, int moveNumber, int childDepth,
                               int side, int ply,
                               const Move& move, bool isPVNode,
                               bool cutNode, bool improving,
                               bool ttPvEvidence, bool escapesThreat,
                               bool singularMove, bool singularLMR)
    {
        if (childDepth < 1 || singularMove)
            return 0;
        int reduction = Reduction(improving, depth, moveNumber);
        if (moveNumber <= 2 && ttHitAverage / TtHitAverageWindow >=
            SearchParameters::LMR::TtHitLowThreshold)
            reduction = 0;
        if (isPVNode || ttPvEvidence) reduction -= SearchParameters::LMR::TtPvAdjustment;
        if (cutNode && IsQuietMove(move)) reduction += SearchParameters::LMR::CutNodeAdjustment;
        const int currentPiece = 1;
        const int statScore = IsQuietMove(move)
            ? QuietStatScore(side, ply, currentPiece, move) : 0;
        const int previousStatScore = StackFrame(ply).statScore;
        if (statScore >= SearchParameters::LMR::GoodStatScore &&
            previousStatScore < SearchParameters::LMR::BadPreviousStatScore)
            --reduction;
        else if (previousStatScore >= SearchParameters::LMR::GoodPreviousStatScore &&
                 statScore < SearchParameters::LMR::BadStatScore)
            ++reduction;
        reduction -= statScore / SearchParameters::LMR::StatScoreDivisor;
        if (singularLMR) reduction -= SearchParameters::LMR::SingularAdjustment;
        if (ttHitAverage / TtHitAverageWindow > SearchParameters::LMR::TtHitHighThreshold) --reduction;
        if (!IsQuietMove(move) && depth < SearchParameters::LMR::ShallowCaptureDepth &&
            moveNumber > SearchParameters::LMR::LateCaptureMoveCount) ++reduction;
        reduction = std::max(0, reduction);
        const int reducedChildDepth = std::clamp(childDepth - reduction, 1, childDepth);
        return childDepth - reducedChildDepth;
    }

    constexpr int CheckOrderingBonus = 40;

    int BasePvsOrderingScore(const Move *move)
    {
        return move->value + (move->givesCheck ? CheckOrderingBonus : 0);
    }

    bool IsMateScore(int score)
    {
        return MateScore::IsMate(score);
    }

    SearchBound ClassifyBound(int value, int alpha, int beta)
    {
        if (value <= alpha)
            return SearchBound::Upper;
        if (value >= beta)
            return SearchBound::Lower;
        return SearchBound::Exact;
    }

    class MovePicker
    {
    public:
        MovePicker(Board& boardValue, int depthValue, int depthGoneValue,
                   int turnValue, const Move& previousMoveValue,
                   uint16_t ttMoveValue, MovePickerAttackerCache* attackerCacheValue = nullptr)
            : board(boardValue), depth(depthValue), depthGone(depthGoneValue),
              turn(turnValue), previousMove(previousMoveValue), packedTTMove(ttMoveValue),
              packedCounterMove(CounterMoveFor(boardValue, depthGoneValue)),
              attackerCache(attackerCacheValue) {}

        MovePicker(const MovePicker&) = delete;
        MovePicker& operator=(const MovePicker&) = delete;

        Move* Next(bool moveCountPruning = false)
        {
            while (true)
            {
                switch (stage)
                {
                case Stage::TT:
                    stage = Stage::GoodTactical;
                    PrepareTT();
                    if (ttEntry >= 0)
                        return MarkReturned(ttEntry);
                    break;
                case Stage::GoodTactical:
                    GenerateTactical();
                    if (goodCursor == 0 && goodCount > 1)
                    {
                        std::stable_sort(goodTactical.begin(), goodTactical.begin() + goodCount,
                            [this](uint8_t a, uint8_t b) {
                                if (entries[a].key != entries[b].key)
                                    return entries[a].key > entries[b].key;
                                return MoveOrderingTieKeyGreater(entries[a].tieKey, entries[b].tieKey);
                            });
                    }
                    if (Move* move = Select(goodTactical, goodCount, goodCursor))
                        return move;
                    stage = Stage::KillerOne;
                    break;
                case Stage::KillerOne:
                    stage = Stage::KillerTwo;
                    if (Move* move = BuildKiller(0))
                        return move;
                    break;
                case Stage::KillerTwo:
                    stage = Stage::Countermove;
                    if (Move* move = BuildKiller(1))
                        return move;
                    break;
                case Stage::Countermove:
                    stage = Stage::StrongQuiets;
                    if (Move* move = BuildRefutation(packedCounterMove))
                        return move;
                    break;
                case Stage::StrongQuiets:
                    if (moveCountPruning)
                    {
                        stage = Stage::BadTactical;
                        break;
                    }
                    ScoreAndClassifyQuiets();
                    if (strongQuietCursor == 0 && strongQuietCount > 1)
                    {
                        std::stable_sort(strongQuiets.begin(), strongQuiets.begin() + strongQuietCount,
                            [this](uint8_t a, uint8_t b) {
                                if (entries[a].key != entries[b].key)
                                    return entries[a].key > entries[b].key;
                                return MoveOrderingTieKeyGreater(entries[a].tieKey, entries[b].tieKey);
                            });
                    }
                    if (Move* move = Select(strongQuiets, strongQuietCount, strongQuietCursor))
                        return move;
                    stage = Stage::RemainingQuiets;
                    break;
                case Stage::RemainingQuiets:
                    if (moveCountPruning)
                    {
                        stage = Stage::BadTactical;
                        break;
                    }
                    ScoreAndClassifyQuiets();
                    if (remainingQuietCursor == 0 && remainingQuietCount > 1)
                    {
                        std::stable_sort(remainingQuiets.begin(), remainingQuiets.begin() + remainingQuietCount,
                            [this](uint8_t a, uint8_t b) {
                                if (entries[a].key != entries[b].key)
                                    return entries[a].key > entries[b].key;
                                return MoveOrderingTieKeyGreater(entries[a].tieKey, entries[b].tieKey);
                            });
                    }
                    if (Move* move = Select(remainingQuiets, remainingQuietCount,
                                            remainingQuietCursor))
                        return move;
                    stage = Stage::BadTactical;
                    break;
                case Stage::BadTactical:
                    GenerateTactical();
                    if (badCursor == 0 && badCount > 1)
                    {
                        std::stable_sort(badTactical.begin(), badTactical.begin() + badCount,
                            [this](uint8_t a, uint8_t b) {
                                if (entries[a].key != entries[b].key)
                                    return entries[a].key > entries[b].key;
                                return MoveOrderingTieKeyGreater(entries[a].tieKey, entries[b].tieKey);
                            });
                    }
                    if (Move* move = Select(badTactical, badCount, badCursor))
                        return move;
                    stage = Stage::Done;
                    break;
                case Stage::Done:
                    return nullptr;
                }
            }
        }

        bool HasTTMove()
        {
            PrepareTT();
            return ttEntry >= 0;
        }
        int GeneratedCount() const { return entryCount; }

    private:
        static constexpr uint8_t ListTactical = 0;
        static constexpr uint8_t ListQuiet = 1;

        struct Entry
        {
            uint8_t listId = 0;
            uint8_t moveIndex = 0;
            int key = 0;
            MoveOrderingTieKey tieKey{};
            bool returned = false;
            bool scored = false;
        };

        enum class Stage : uint8_t
        {
            TT,
            GoodTactical,
            KillerOne,
            KillerTwo,
            Countermove,
            StrongQuiets,
            RemainingQuiets,
            BadTactical,
            Done
        };

        Move& GetMove(const Entry& entry)
        {
            return entry.listId == ListTactical ? tacticalMoves[entry.moveIndex] : quietMoves[entry.moveIndex];
        }

        const Move& GetMove(const Entry& entry) const
        {
            return entry.listId == ListTactical ? tacticalMoves[entry.moveIndex] : quietMoves[entry.moveIndex];
        }

        bool MatchesTT(const Move& move) const
        {
            if (packedTTMove == 0)
                return false;
            return move.beginPlace == TTMoveHelper::UnpackFrom(packedTTMove) &&
                   move.endPlace == TTMoveHelper::UnpackTo(packedTTMove) &&
                   (move.promotionPiece > 0 ? move.promotionPiece : 0) ==
                       TTMoveHelper::UnpackPromotion(packedTTMove);
        }

        int Add(uint8_t listId, uint8_t moveIndex)
        {
            if (entryCount >= static_cast<int>(entries.size()))
                return -1;
            Entry& entry = entries[entryCount];
            entry.listId = listId;
            entry.moveIndex = moveIndex;
            entry.returned = false;
            entry.scored = false;
            return entryCount++;
        }

        Move* MarkReturned(int index)
        {
            entries[index].returned = true;
            Score(index);
            return &GetMove(entries[index]);
        }

        Move* Select(std::array<uint8_t, 256>& indices, int count, int& cursor)
        {
            while (cursor < count)
            {
                const int index = indices[cursor++];
                if (!entries[index].returned)
                    return MarkReturned(index);
            }
            return nullptr;
        }

        void Score(int index)
        {
            Entry& entry = entries[index];
            if (entry.scored)
                return;
            const AttackerState& white = attackerCache ? attackerCache->GetWhite(board) :
                (attackersReady ? whiteAttacker : (whiteAttacker = MoveLogic::SetWhiteAttacker(board)));
            const AttackerState& black = attackerCache ? attackerCache->GetBlack(board) :
                (attackersReady ? blackAttacker : (blackAttacker = MoveLogic::SetBlackAttacker(board)));
            attackersReady = true;
            Move& m = GetMove(entry);
            if (!m.givesCheckComputed)
            {
                m.givesCheck = MoveLogic::MoveGivesCheck(board, m);
                m.givesCheckComputed = true;
            }
            MoveLogic::ScoreMove(board, m, white, black);
            entry.scored = true;
        }

        bool TTHintIsTactical() const
        {
            if (packedTTMove == 0)
                return false;
            const int from = TTMoveHelper::UnpackFrom(packedTTMove);
            const int to = TTMoveHelper::UnpackTo(packedTTMove);
            if (TTMoveHelper::UnpackPromotion(packedTTMove) != 0 || board.positionCore.pieceAt[to] != 0)
                return true;
            return board.positionCore.pieceAt[from] % 8 == 1 && to == board.unpassentPlace;
        }

        void PrepareTT()
        {
            if (ttPrepared || packedTTMove == 0)
                return;
            ttPrepared = true;
            if (TTHintIsTactical())
                GenerateTactical();
            else
                GenerateQuietPseudoMoves();
        }

        void GenerateTactical()
        {
            if (tacticalGenerated)
                return;
            tacticalGenerated = true;
            AttackerState emptyWhite{};
            AttackerState emptyBlack{};
            MoveLogic::MoveGeneratorInto(
                board, depth, depthGone, true, false, emptyWhite, emptyBlack, tacticalMoves, false);
            for (int i = 0; i < tacticalMoves.count; ++i)
            {
                const Move& candidate = tacticalMoves[i];
                if (candidate.endPiece % 8 == 6)
                    continue;
                const int index = Add(ListTactical, static_cast<uint8_t>(i));
                if (index >= 0)
                {
                    Move& m = GetMove(entries[index]);
                    const int movingPiece = board.positionCore.pieceAt[m.beginPlace];
                    const int capturedPiece = m.endPiece;
                    static constexpr int MgValue[7] = {0, 100, 320, 330, 500, 900, 0};
                    const int victimType = capturedPiece % 8;
                    entries[index].key = SearchParameters::MoveOrdering::CaptureVictimMultiplier *
                        MgValue[std::min(victimType, 6)] +
                        captureHistory[std::clamp(movingPiece, 0, 14)]
                                      [m.endPlace]
                                      [std::clamp(capturedPiece % 8, 0, 6)];
                    entries[index].tieKey = MakeMoveOrderingTieKey(m, movingPiece, false, false);
                    if (MatchesTT(m))
                        ttEntry = index;
                    if (MoveLogic::SEE_GE(board, m,
                                         -SearchParameters::SEE::GoodCaptureCoefficient *
                                         entries[index].key /
                                         SearchParameters::MoveOrdering::GoodCaptureSeeDivisor))
                        goodTactical[goodCount++] = static_cast<uint8_t>(index);
                    else
                        badTactical[badCount++] = static_cast<uint8_t>(index);
                }
            }
        }

        Move* BuildKiller(int killerIndex)
        {
            GenerateQuietPseudoMoves();
            if (depthGone < 0 || depthGone >= PVSSearch::MaxKillerPly)
                return nullptr;
            const PVSSearch::KillerMove& killer = PVSSearch::killers[depthGone][killerIndex];
            if (!killer.isValid())
                return nullptr;
            for (int i = 0; i < quietEntryCount; ++i)
            {
                const int index = quietEntries[i];
                const Move& m = GetMove(entries[index]);
                if (!entries[index].returned &&
                    m.beginPlace == killer.beginPlace &&
                    m.endPlace == killer.endPlace &&
                    (m.promotionPiece > 0
                         ? m.promotionPiece : 0) == killer.promotionPiece)
                    return MarkReturned(index);
            }
            return nullptr;
        }

        Move* BuildRefutation(uint16_t packedMove)
        {
            if (packedMove == 0 || packedMove == packedTTMove)
                return nullptr;
            GenerateQuietPseudoMoves();
            for (int i = 0; i < quietEntryCount; ++i)
            {
                const int index = quietEntries[i];
                const Move& m = GetMove(entries[index]);
                if (!entries[index].returned && MatchesPackedMove(m, packedMove))
                    return MarkReturned(index);
            }
            return nullptr;
        }

        void GenerateQuietPseudoMoves()
        {
            if (quietsGenerated)
                return;
            quietsGenerated = true;
            AttackerState emptyWhite{};
            AttackerState emptyBlack{};
            MoveLogic::MoveGeneratorInto(
                board, depth, depthGone, false, false, emptyWhite, emptyBlack, quietMoves, true);
            for (int i = 0; i < quietMoves.count; ++i)
            {
                const Move& candidate = quietMoves[i];
                if (IsQuietMove(candidate))
                {
                    const int index = Add(ListQuiet, static_cast<uint8_t>(i));
                    if (index >= 0)
                    {
                        quietEntries[quietEntryCount++] = static_cast<uint8_t>(index);
                        if (MatchesTT(GetMove(entries[index])))
                            ttEntry = index;
                    }
                }
            }
        }

        bool IsKiller(const Move& move) const
        {
            if (depthGone < 0 || depthGone >= PVSSearch::MaxKillerPly)
                return false;
            for (int i = 0; i < 2; ++i)
            {
                const PVSSearch::KillerMove& killer = PVSSearch::killers[depthGone][i];
                if (killer.isValid() && move.beginPlace == killer.beginPlace &&
                    move.endPlace == killer.endPlace &&
                    (move.promotionPiece > 0 ? move.promotionPiece : 0) ==
                        killer.promotionPiece)
                    return true;
            }
            return false;
        }

        void ScoreAndClassifyQuiets()
        {
            if (quietsScored)
                return;
            quietsScored = true;
            GenerateQuietPseudoMoves();
            for (int i = 0; i < quietEntryCount; ++i)
            {
                const int index = quietEntries[i];
                const Move& m = GetMove(entries[index]);
                if (entries[index].returned || IsKiller(m))
                    continue;
                Score(index);
                const int currentPiece = board.positionCore.pieceAt[m.beginPlace];
                entries[index].key = QuietOrderingScore(
                    turn, depthGone, currentPiece, m);
                entries[index].tieKey = MakeMoveOrderingTieKey(m, currentPiece, true, true);
                if (entries[index].key >=
                    SearchParameters::MoveOrdering::StrongQuietDepthCoefficient * depth)
                    strongQuiets[strongQuietCount++] = static_cast<uint8_t>(index);
                else
                    remainingQuiets[remainingQuietCount++] = static_cast<uint8_t>(index);
            }
        }

        Board& board;
        int depth;
        int depthGone;
        int turn;
        const Move& previousMove;
        uint16_t packedTTMove;
        uint16_t packedCounterMove;
        MoveList tacticalMoves;
        MoveList quietMoves;
        std::array<Entry, 256> entries;
        std::array<uint8_t, 256> goodTactical;
        std::array<uint8_t, 256> badTactical;
        std::array<uint8_t, 256> quietEntries;
        std::array<uint8_t, 256> strongQuiets;
        std::array<uint8_t, 256> remainingQuiets;
        int entryCount = 0;
        int goodCount = 0;
        int badCount = 0;
        int quietEntryCount = 0;
        int strongQuietCount = 0;
        int remainingQuietCount = 0;
        int goodCursor = 0;
        int badCursor = 0;
        int strongQuietCursor = 0;
        int remainingQuietCursor = 0;
        int ttEntry = -1;
        AttackerState whiteAttacker{};
        AttackerState blackAttacker{};
        MovePickerAttackerCache* attackerCache = nullptr;
        bool tacticalGenerated = false;
        bool quietsGenerated = false;
        bool quietsScored = false;
        bool attackersReady = false;
        bool ttPrepared = false;
        Stage stage = Stage::TT;
    };

    bool NullMoveMaterialEligible(const Board &board, int turn)
    {
        const int minorPieceCount =
            static_cast<int>(PositionCorePieceListsView{board.positionCore}[turn * 8 + 2].size() +
                             PositionCorePieceListsView{board.positionCore}[turn * 8 + 3].size());
        const bool hasNonPawn = minorPieceCount > 0 ||
            PositionCorePieceListsView{board.positionCore}[turn * 8 + 4].size() > 0 ||
            PositionCorePieceListsView{board.positionCore}[turn * 8 + 5].size() > 0;
        return hasNonPawn;
    }

    bool IsAdvancedPassedPawnPush(const Board& board, const Move& move, int side)
    {
        if (board.positionCore.pieceAt[move.beginPlace] % 8 != 1 || move.endPiece != 0)
            return false;
        const int rank = move.endPlace / 8;
        if ((side == 0 && rank < 5) || (side == 1 && rank > 2))
            return false;
        const int file = move.endPlace % 8;
        const int enemyPawn = side == 0 ? 9 : 1;
        const int step = side == 0 ? 8 : -8;
        for (int base = move.endPlace + step; base >= 0 && base < 64; base += step)
        {
            const int row = base / 8;
            for (int df = -1; df <= 1; ++df)
            {
                const int targetFile = file + df;
                if (targetFile >= 0 && targetFile < 8 &&
                    board.positionCore.pieceAt[row * 8 + targetFile] == enemyPawn)
                    return false;
            }
        }
        return true;
    }

    bool LastCaptureExtension(const Board& board, const Move& move)
    {
        if (move.endPiece % 8 <= 1)
            return false;
        int nonPawnMaterial = 0;
        for (int sideOffset : {0, 8})
        {
            nonPawnMaterial += static_cast<int>(PositionCorePieceListsView{board.positionCore}[sideOffset + 2].size()) * Option::KnightValue;
            nonPawnMaterial += static_cast<int>(PositionCorePieceListsView{board.positionCore}[sideOffset + 3].size()) * Option::BishopValue;
            nonPawnMaterial += static_cast<int>(PositionCorePieceListsView{board.positionCore}[sideOffset + 4].size()) * Option::RookValue;
            nonPawnMaterial += static_cast<int>(PositionCorePieceListsView{board.positionCore}[sideOffset + 5].size()) * Option::QueenValue;
        }
        static const int values[7] = {0, Option::PawnValue, Option::KnightValue,
            Option::BishopValue, Option::RookValue, Option::QueenValue, 0};
        nonPawnMaterial -= values[std::clamp(move.endPiece % 8, 0, 6)];
        return nonPawnMaterial <= 2 * Option::RookValue;
    }

    bool MovedPieceGivesCheck(const Board& board, const Move& move)
    {
        const bool movingWhite = !board.sideToMove;
        const int enemyKingIndex = movingWhite ? 14 : 6;
        if (PositionCorePieceListsView{board.positionCore}[enemyKingIndex].empty())
            return false;
        const int kingSquare = PositionCorePieceListsView{board.positionCore}[enemyKingIndex].front();
        const long long kingBit = Option::PowerTwo[kingSquare];
        long long occupancy = (PositionCoreLogic::WhiteOccupancy(board.positionCore) | PositionCoreLogic::BlackOccupancy(board.positionCore)) &
            ~Option::PowerTwo[move.beginPlace];
        occupancy |= Option::PowerTwo[move.endPlace];
        if ((move.PublicFlag & Option::PowerTwo[6]) != 0)
            occupancy &= ~Option::PowerTwo[move.endPlace + (movingWhite ? -8 : 8)];
        const int piece = move.promotionPiece > 0
            ? move.promotionPiece % 8 : board.positionCore.pieceAt[move.beginPlace] % 8;
        switch (piece)
        {
        case 1:
            return (((movingWhite ? AttackPlaces::WhitePawnAttackPlaces[move.endPlace]
                                  : AttackPlaces::BlackPawnAttackPlaces[move.endPlace])) & kingBit) != 0;
        case 2:
            return (AttackPlaces::KnightAttackPlaces[move.endPlace] & kingBit) != 0;
        case 3:
            return (AttackPlaces::BishopAttack[move.endPlace][kingSquare] & occupancy) == kingBit;
        case 4:
            return (AttackPlaces::RookAttack[move.endPlace][kingSquare] & occupancy) == kingBit;
        case 5:
            return (AttackPlaces::QueenAttack[move.endPlace][kingSquare] & occupancy) == kingBit;
        case 6:
            return (AttackPlaces::KingAttackPlaces[move.endPlace] & kingBit) != 0;
        default:
            return false;
        }
    }

    int TempoForPosition(const Board& board)
    {
        static constexpr int weights[25] = {
            10000, 10000, 10000, 10000, 10000, 10000, 10000, 8902, 7804,
            6706, 5609, 4511, 3413, 2844, 2275, 1706, 1138, 569, 0, 0, 0,
            0, 0, 0, 0};
        const int phase = GamePhase::Calculate(board.positionCore);
        return (Option::TempoMiddleGame * (10000 - weights[phase]) +
                Option::TempoEndGame * weights[phase]) / 10000;
    }

}

void PVSSearch::ResetHistory()
{
    std::fill(&mainHistory[0][0][0], &mainHistory[0][0][0] + 2 * 64 * 64, 0);
    std::fill(&captureHistory[0][0][0], &captureHistory[0][0][0] + 15 * 64 * 7, 0);
    std::fill(&counterMoves[0][0], &counterMoves[0][0] + 15 * 64, 0);
    continuationHistory.fill(0);
    ResetSearchStack();
}

int PVSSearch::QCaptureOrderingScore(const Board& board, const Move& move)
{
    static constexpr int value[7] = {0, 100, 350, 350, 550, 975, 2500};
    const int movingPiece = std::clamp(static_cast<int>(board.positionCore.pieceAt[move.beginPlace]), 0, 14);
    const int capturedPiece = std::clamp(move.endPiece % 8, 0, 6);
    return SearchParameters::MoveOrdering::CaptureVictimMultiplier * value[capturedPiece] +
        captureHistory[movingPiece][move.endPlace][capturedPiece];
}

int PVSSearch::QQuietEvasionOrderingScore(int side, int ply,
                                           const Board& board, const Move& move)
{
    const int movingPiece = std::clamp(static_cast<int>(board.positionCore.pieceAt[move.beginPlace]), 1, 14);
    return HistoryScore(side, move) +
        ContinuationHistoryScoreAt(ply, 1, movingPiece, move);
}


void PVSSearch::ResetKillers()
{
    for (int i = 0; i < MaxKillerPly; ++i)
    {
        killers[i][0] = KillerMove{};
        killers[i][1] = KillerMove{};
    }
}

#if HOWL_CORRECTNESS_TESTING
bool PVSSearch::NullMoveMaterialEligibleForTesting(const Board &board)
{
    return NullMoveMaterialEligible(board, board.sideToMove ? 1 : 0);
}
#endif

void PVSSearch::RecordKiller(int ply, const Move &move)
{
    if (ply < 0 || ply >= MaxKillerPly)
        return;
    if (move.endPiece > 0 || move.promotionPiece > 0)
        return;

    KillerMove km{move.beginPlace, move.endPlace, move.promotionPiece};
    if (killers[ply][0] != move)
    {
        killers[ply][1] = killers[ply][0];
        killers[ply][0] = km;
    }
}

#if HOWL_CORRECTNESS_TESTING
namespace
{
    int g_futilityPruningSkippedQuietMoves = 0;
    PVSSearch::MoveOrderingStats g_moveOrderingStats;
    PVSSearch::MoveOrderingQualityStats g_orderingQualityStats;

    void RecordMoveOrderingCutoff(int moveIndex, bool isTTMove, const Move &move, int depth, int ply, bool isPV, bool isCutNode)
    {
        g_orderingQualityStats.recordCutoff(moveIndex + 1, isPV, isCutNode, IsQuietMove(move));
        g_moveOrderingStats.totalBetaCutoffs++;
        g_moveOrderingStats.allCutoffs.add(moveIndex);

        if (depth <= 2)
            g_moveOrderingStats.depth1To2.add(moveIndex);
        else if (depth <= 5)
            g_moveOrderingStats.depth3To5.add(moveIndex);
        else
            g_moveOrderingStats.depth6Plus.add(moveIndex);

        if (isTTMove)
        {
            g_moveOrderingStats.cutoffsTTMove++;
        }
        else if (move.endPiece > 0 || move.promotionPiece > 0)
        {
            g_moveOrderingStats.cutoffsCaptureOrPromotion++;
        }
        else
        {
            g_moveOrderingStats.cutoffsQuiet++;
            g_moveOrderingStats.quietCutoffs.add(moveIndex);
        }

        if (ply >= 0 && ply < PVSSearch::MaxKillerPly && move.endPiece == 0 && move.promotionPiece <= 0)
        {
            if (PVSSearch::killers[ply][0] == move)
            {
                g_moveOrderingStats.killerBetaCutoffs++;
                g_moveOrderingStats.killer1BetaCutoffs++;
            }
            else if (PVSSearch::killers[ply][1] == move)
            {
                g_moveOrderingStats.killerBetaCutoffs++;
                g_moveOrderingStats.killer2BetaCutoffs++;
            }
        }
    }

    void PrintBucketGroup(const char *title, const PVSSearch::IndexBuckets &b)
    {
        std::cout << "\n[" << title << "]\n";
        std::cout << "  Total cutoffs: " << b.total << "\n";
        auto printRow = [b](const char *label, uint64_t count) {
            double pct = (b.total > 0) ? (100.0 * count / static_cast<double>(b.total)) : 0.0;
            char buf[128];
            snprintf(buf, sizeof(buf), "  %-12s %8lu  (%6.2f%%)\n", label, (unsigned long)count, pct);
            std::cout << buf;
        };
        printRow("Index 0:", b.idx0);
        printRow("Index 1:", b.idx1);
        printRow("Index 2:", b.idx2);
        printRow("Index 3:", b.idx3);
        printRow("Indices 4-7:", b.idx4To7);
        printRow("Index 8+:", b.idx8Plus);
    }
}

int PVSSearch::FutilityPruningSkippedQuietMovesForTesting()
{
    return g_futilityPruningSkippedQuietMoves;
}

void PVSSearch::ResetFutilityPruningSkippedQuietMovesForTesting()
{
    g_futilityPruningSkippedQuietMoves = 0;
}

PVSSearch::MoveOrderingStats PVSSearch::GetMoveOrderingStatsForTesting()
{
    return g_moveOrderingStats;
}

void PVSSearch::ResetMoveOrderingStatsForTesting()
{
    g_moveOrderingStats = MoveOrderingStats{};
}

void PVSSearch::PrintMoveOrderingStatsForTesting()
{
    std::cout << "=== Move Ordering Cutoff Stats (Recursive PVS) ===\n";
    std::cout << "Overall total beta cutoffs: " << g_moveOrderingStats.totalBetaCutoffs << "\n";

    PrintBucketGroup("All Depths (Overall)", g_moveOrderingStats.allCutoffs);
    PrintBucketGroup("Depth 1 to 2", g_moveOrderingStats.depth1To2);
    PrintBucketGroup("Depth 3 to 5", g_moveOrderingStats.depth3To5);
    PrintBucketGroup("Depth 6+", g_moveOrderingStats.depth6Plus);
    PrintBucketGroup("Quiet Moves Only", g_moveOrderingStats.quietCutoffs);

    std::cout << "\n[Move Category Breakdown]\n";
    auto printCat = [total = g_moveOrderingStats.totalBetaCutoffs](const char *label, uint64_t count) {
        double pct = (total > 0) ? (100.0 * count / static_cast<double>(total)) : 0.0;
        char buf[128];
        snprintf(buf, sizeof(buf), "  %-24s %8lu  (%6.2f%%)\n", label, (unsigned long)count, pct);
        std::cout << buf;
    };
    printCat("TT move:", g_moveOrderingStats.cutoffsTTMove);
    printCat("Capture or promotion:", g_moveOrderingStats.cutoffsCaptureOrPromotion);
    printCat("Quiet:", g_moveOrderingStats.cutoffsQuiet);

    std::cout << "\n[Killer Move Stats]\n";
    std::cout << "  Killer candidates encountered: " << g_moveOrderingStats.killerCandidatesEncountered << "\n";
    printCat("Killer beta cutoffs:", g_moveOrderingStats.killerBetaCutoffs);
    printCat("Killer 1 cutoffs:", g_moveOrderingStats.killer1BetaCutoffs);
    printCat("Killer 2 cutoffs:", g_moveOrderingStats.killer2BetaCutoffs);
}

PVSSearch::LMRStats g_lmrStats;

void RecordLMRSearch(int moveIndex, int depth, int ply, const Move &move, int reductionAmount, bool triggeredReSearch)
{
    g_lmrStats.totalReducedSearches++;
    if (triggeredReSearch)
        g_lmrStats.reducedTriggeredReSearch++;
    else
        g_lmrStats.reducedFailLow++;

    const int reductionBucket = std::clamp(reductionAmount, 1, 5) - 1;
    g_lmrStats.reductionAttempts[reductionBucket]++;
    if (triggeredReSearch)
        g_lmrStats.reductionReSearches[reductionBucket]++;

    if (moveIndex == 1) g_lmrStats.idx1.record(triggeredReSearch);
    else if (moveIndex == 2) g_lmrStats.idx2.record(triggeredReSearch);
    else if (moveIndex == 3) g_lmrStats.idx3.record(triggeredReSearch);
    else if (moveIndex >= 4 && moveIndex <= 7) g_lmrStats.idx4To7.record(triggeredReSearch);
    else g_lmrStats.idx8Plus.record(triggeredReSearch);

    if (depth == 3) g_lmrStats.depth3.record(triggeredReSearch);
    else if (depth >= 4 && depth <= 5) g_lmrStats.depth4To5.record(triggeredReSearch);
    else if (depth >= 6 && depth <= 8) g_lmrStats.depth6To8.record(triggeredReSearch);
    else g_lmrStats.depth9Plus.record(triggeredReSearch);

    bool isKiller = (ply >= 0 && ply < PVSSearch::MaxKillerPly && (PVSSearch::killers[ply][0] == move || PVSSearch::killers[ply][1] == move));
    if (move.endPiece > 0)
    {
        g_lmrStats.moveLosingCapture.record(triggeredReSearch);
    }
    else if (isKiller)
    {
        g_lmrStats.moveKillerQuiet.record(triggeredReSearch);
    }
    else
    {
        g_lmrStats.moveQuiet.record(triggeredReSearch);
    }

    // Indices 4 to 7 detailed tracking
    if (moveIndex >= 4 && moveIndex <= 7)
    {
        if (depth >= 3 && depth <= 4) g_lmrStats.idx4To7_depth3To4.record(triggeredReSearch);
        else if (depth == 5) g_lmrStats.idx4To7_depth5.record(triggeredReSearch);
        else if (depth == 6) g_lmrStats.idx4To7_depth6.record(triggeredReSearch);
        else if (depth >= 7 && depth <= 8) g_lmrStats.idx4To7_depth7To8.record(triggeredReSearch);
        else if (depth >= 9) g_lmrStats.idx4To7_depth9Plus.record(triggeredReSearch);

        if (move.endPiece > 0)
            g_lmrStats.idx4To7_losingCapture.record(triggeredReSearch);
        else if (isKiller)
            g_lmrStats.idx4To7_killer.record(triggeredReSearch);
        else
            g_lmrStats.idx4To7_quiet.record(triggeredReSearch);
    }
}

void RecordLMRReSearchResult(int moveIndex, int finalVal, int origAlpha, int origBeta)
{
    if (finalVal <= origAlpha)
        g_lmrStats.reSearchFailLow++;
    else if (finalVal < origBeta)
        g_lmrStats.reSearchPV++;
    else
        g_lmrStats.reSearchBetaCutoff++;

    if (moveIndex >= 4 && moveIndex <= 7)
    {
        g_lmrStats.idx4To7_reSearchTotal++;
        if (finalVal <= origAlpha)
            g_lmrStats.idx4To7_reSearchFailLow++;
        else if (finalVal < origBeta)
            g_lmrStats.idx4To7_reSearchPV++;
        else
            g_lmrStats.idx4To7_reSearchBetaCutoff++;
    }
}

void PrintLMRBucketRow(const char *label, const PVSSearch::LMRBucket &b)
{
    double failLowPct = (b.reducedSearches > 0) ? (100.0 * b.failLow / static_cast<double>(b.reducedSearches)) : 0.0;
    double reSearchPct = (b.reducedSearches > 0) ? (100.0 * b.reSearches / static_cast<double>(b.reducedSearches)) : 0.0;
    char buf[256];
    snprintf(buf, sizeof(buf), "  %-20s Total: %7lu | Fail-low: %7lu (%6.2f%%) | Re-search: %7lu (%6.2f%%)\n",
             label, (unsigned long)b.reducedSearches, (unsigned long)b.failLow, failLowPct, (unsigned long)b.reSearches, reSearchPct);
    std::cout << buf;
}

PVSSearch::LMRStats PVSSearch::GetLMRStatsForTesting()
{
    return g_lmrStats;
}

void PVSSearch::ResetLMRStatsForTesting()
{
    g_lmrStats = LMRStats{};
}

void PVSSearch::PrintLMRStatsForTesting()
{
    std::cout << "=== Late Move Reduction (LMR) Stats (Recursive PVS) ===\n";
    std::cout << "Total LMR reduced searches: " << g_lmrStats.totalReducedSearches << "\n";

    double totalFailLowPct = (g_lmrStats.totalReducedSearches > 0) ? (100.0 * g_lmrStats.reducedFailLow / static_cast<double>(g_lmrStats.totalReducedSearches)) : 0.0;
    double totalReSearchPct = (g_lmrStats.totalReducedSearches > 0) ? (100.0 * g_lmrStats.reducedTriggeredReSearch / static_cast<double>(g_lmrStats.totalReducedSearches)) : 0.0;

    std::cout << "  Immediate fail-low (no re-search): " << g_lmrStats.reducedFailLow << " (" << totalFailLowPct << "%)\n";
    std::cout << "  Triggered greater-depth re-search: " << g_lmrStats.reducedTriggeredReSearch << " (" << totalReSearchPct << "%)\n";
    std::cout << "  Full-depth confirmations:          " << g_lmrStats.fullDepthConfirmations << "\n";

    std::cout << "\n[Reduction Depth Breakdown]\n";
    auto printRed = [](const char *label, uint64_t attempts, uint64_t reSearch) {
        double rsPct = (attempts > 0) ? (100.0 * reSearch / static_cast<double>(attempts)) : 0.0;
        char buf[256];
        snprintf(buf, sizeof(buf), "  %-8s Attempts: %7lu | Re-search: %7lu (%6.2f%%)\n",
                 label, (unsigned long)attempts, (unsigned long)reSearch, rsPct);
        std::cout << buf;
    };
    printRed("R1:", g_lmrStats.reductionAttempts[0], g_lmrStats.reductionReSearches[0]);
    printRed("R2:", g_lmrStats.reductionAttempts[1], g_lmrStats.reductionReSearches[1]);
    printRed("R3:", g_lmrStats.reductionAttempts[2], g_lmrStats.reductionReSearches[2]);
    printRed("R4:", g_lmrStats.reductionAttempts[3], g_lmrStats.reductionReSearches[3]);
    printRed("R5+:", g_lmrStats.reductionAttempts[4], g_lmrStats.reductionReSearches[4]);

    std::cout << "\n[Full-Depth Re-Search Outcome Breakdown]\n";
    std::cout << "  Total full-depth confirmations: " << g_lmrStats.fullDepthConfirmations << "\n";
    auto printOutcome = [total = g_lmrStats.fullDepthConfirmations](const char *label, uint64_t count) {
        double pct = (total > 0) ? (100.0 * count / static_cast<double>(total)) : 0.0;
        char buf[128];
        snprintf(buf, sizeof(buf), "  %-32s %7lu  (%6.2f%%)\n", label, (unsigned long)count, pct);
        std::cout << buf;
    };
    printOutcome("Final value <= original alpha:", g_lmrStats.reSearchFailLow);
    printOutcome("Final value > alpha & < beta:", g_lmrStats.reSearchPV);
    printOutcome("Final value >= beta (cutoff):", g_lmrStats.reSearchBetaCutoff);

    std::cout << "\n[By Move Index]\n";
    PrintLMRBucketRow("Index 1:", g_lmrStats.idx1);
    PrintLMRBucketRow("Index 2:", g_lmrStats.idx2);
    PrintLMRBucketRow("Index 3:", g_lmrStats.idx3);
    PrintLMRBucketRow("Indices 4-7:", g_lmrStats.idx4To7);
    PrintLMRBucketRow("Index 8+:", g_lmrStats.idx8Plus);

    std::cout << "\n[By Remaining Depth]\n";
    PrintLMRBucketRow("Depth 3:", g_lmrStats.depth3);
    PrintLMRBucketRow("Depth 4-5:", g_lmrStats.depth4To5);
    PrintLMRBucketRow("Depth 6-8:", g_lmrStats.depth6To8);
    PrintLMRBucketRow("Depth 9+:", g_lmrStats.depth9Plus);

    std::cout << "\n[By Move Type]\n";
    PrintLMRBucketRow("Quiet moves:", g_lmrStats.moveQuiet);
    PrintLMRBucketRow("Losing captures:", g_lmrStats.moveLosingCapture);
    PrintLMRBucketRow("Killer quiet moves:", g_lmrStats.moveKillerQuiet);

    std::cout << "\n=======================================================\n";
    std::cout << "=== Detailed Investigation: Move Indices 4 to 7 ===\n";
    std::cout << "=======================================================\n";
    std::cout << "\n[Indices 4-7 By Depth]\n";
    PrintLMRBucketRow("Depth 3-4:", g_lmrStats.idx4To7_depth3To4);
    PrintLMRBucketRow("Depth 5:",   g_lmrStats.idx4To7_depth5);
    PrintLMRBucketRow("Depth 6:",   g_lmrStats.idx4To7_depth6);
    PrintLMRBucketRow("Depth 7-8:", g_lmrStats.idx4To7_depth7To8);
    PrintLMRBucketRow("Depth 9+:",  g_lmrStats.idx4To7_depth9Plus);

    std::cout << "\n[Indices 4-7 Re-Search Outcomes]\n";
    std::cout << "  Total full-depth re-searches: " << g_lmrStats.idx4To7_reSearchTotal << "\n";
    auto printIdx4To7Outcome = [total = g_lmrStats.idx4To7_reSearchTotal](const char *label, uint64_t count) {
        double pct = (total > 0) ? (100.0 * count / static_cast<double>(total)) : 0.0;
        char buf[128];
        snprintf(buf, sizeof(buf), "  %-32s %7lu  (%6.2f%%)\n", label, (unsigned long)count, pct);
        std::cout << buf;
    };
    printIdx4To7Outcome("Final value <= original alpha:", g_lmrStats.idx4To7_reSearchFailLow);
    printIdx4To7Outcome("Final value > alpha & < beta:", g_lmrStats.idx4To7_reSearchPV);
    printIdx4To7Outcome("Final value >= beta (cutoff):", g_lmrStats.idx4To7_reSearchBetaCutoff);

    std::cout << "\n[Indices 4-7 By Move Type]\n";
    PrintLMRBucketRow("Ordinary quiet:",   g_lmrStats.idx4To7_quiet);
    PrintLMRBucketRow("Killer quiet:",     g_lmrStats.idx4To7_killer);
    PrintLMRBucketRow("Losing capture:",   g_lmrStats.idx4To7_losingCapture);
}

PVSSearch::FutilityStats g_futilityStats;

void RecordFutilityCandidate(int moveIndex, int depth, int ply, const Move &move, int staticEval, int origAlpha, int origBeta, int actualValue, bool givesCheck)
{
    g_futilityStats.totalCandidates++;
    if (actualValue <= origAlpha)
        g_futilityStats.candidatesFailLow++;
    else if (actualValue < origBeta)
        g_futilityStats.candidatesPV++;
    else
        g_futilityStats.candidatesBetaCutoff++;

    if (depth == 1) g_futilityStats.depth1.record(actualValue, origAlpha, origBeta);
    else if (depth == 2) g_futilityStats.depth2.record(actualValue, origAlpha, origBeta);

    if (moveIndex == 1) g_futilityStats.idx1.record(actualValue, origAlpha, origBeta);
    else if (moveIndex == 2) g_futilityStats.idx2.record(actualValue, origAlpha, origBeta);
    else if (moveIndex == 3) g_futilityStats.idx3.record(actualValue, origAlpha, origBeta);
    else if (moveIndex >= 4 && moveIndex <= 7) g_futilityStats.idx4To7.record(actualValue, origAlpha, origBeta);
    else g_futilityStats.idx8Plus.record(actualValue, origAlpha, origBeta);

    bool isKiller = (ply >= 0 && ply < PVSSearch::MaxKillerPly && (PVSSearch::killers[ply][0] == move || PVSSearch::killers[ply][1] == move));
    if (givesCheck)
        g_futilityStats.quietGivingCheck.record(actualValue, origAlpha, origBeta);
    else if (isKiller)
        g_futilityStats.killerQuiet.record(actualValue, origAlpha, origBeta);
    else
        g_futilityStats.ordinaryQuiet.record(actualValue, origAlpha, origBeta);

    int staticGap = origAlpha - staticEval;
    if (staticGap < 150) g_futilityStats.gap0To149.record(actualValue, origAlpha, origBeta);
    else if (staticGap < 300) g_futilityStats.gap150To299.record(actualValue, origAlpha, origBeta);
    else if (staticGap < 500) g_futilityStats.gap300To499.record(actualValue, origAlpha, origBeta);
    else g_futilityStats.gap500Plus.record(actualValue, origAlpha, origBeta);
}

void PrintFutilityBucketRow(const char *label, const PVSSearch::FutilityBucket &b)
{
    double flPct = (b.total > 0) ? (100.0 * b.failLow / static_cast<double>(b.total)) : 0.0;
    double pvPct = (b.total > 0) ? (100.0 * b.pv / static_cast<double>(b.total)) : 0.0;
    double cutPct = (b.total > 0) ? (100.0 * b.cutoff / static_cast<double>(b.total)) : 0.0;
    char buf[256];
    snprintf(buf, sizeof(buf), "  %-24s Total: %7lu | <=alpha: %7lu (%5.1f%%) | PV: %5lu (%4.1f%%) | >=beta: %5lu (%4.1f%%)\n",
             label, (unsigned long)b.total, (unsigned long)b.failLow, flPct, (unsigned long)b.pv, pvPct, (unsigned long)b.cutoff, cutPct);
    std::cout << buf;
}

PVSSearch::FutilityStats PVSSearch::GetFutilityStatsForTesting()
{
    return g_futilityStats;
}

void PVSSearch::ResetFutilityStatsForTesting()
{
    g_futilityStats = FutilityStats{};
}

void PVSSearch::PrintFutilityStatsForTesting()
{
    std::cout << "=== Futility Pruning Safety Stats (Recursive PVS) ===\n";
    std::cout << "Total futility candidates evaluated: " << g_futilityStats.totalCandidates << "\n";

    double flPct = (g_futilityStats.totalCandidates > 0) ? (100.0 * g_futilityStats.candidatesFailLow / static_cast<double>(g_futilityStats.totalCandidates)) : 0.0;
    double pvPct = (g_futilityStats.totalCandidates > 0) ? (100.0 * g_futilityStats.candidatesPV / static_cast<double>(g_futilityStats.totalCandidates)) : 0.0;
    double cutPct = (g_futilityStats.totalCandidates > 0) ? (100.0 * g_futilityStats.candidatesBetaCutoff / static_cast<double>(g_futilityStats.totalCandidates)) : 0.0;

    char buf[256];
    snprintf(buf, sizeof(buf), "  Overall <= original alpha (safe to prune): %7lu (%5.1f%%)\n  Overall > alpha and < beta (would improve PV): %7lu (%5.1f%%)\n  Overall >= beta (would cause cutoff):       %7lu (%5.1f%%)\n",
             (unsigned long)g_futilityStats.candidatesFailLow, flPct, (unsigned long)g_futilityStats.candidatesPV, pvPct, (unsigned long)g_futilityStats.candidatesBetaCutoff, cutPct);
    std::cout << buf;

    std::cout << "\n[By Remaining Depth]\n";
    PrintFutilityBucketRow("Depth 1:", g_futilityStats.depth1);
    PrintFutilityBucketRow("Depth 2:", g_futilityStats.depth2);

    std::cout << "\n[By Move Index]\n";
    PrintFutilityBucketRow("Index 1:", g_futilityStats.idx1);
    PrintFutilityBucketRow("Index 2:", g_futilityStats.idx2);
    PrintFutilityBucketRow("Index 3:", g_futilityStats.idx3);
    PrintFutilityBucketRow("Indices 4-7:", g_futilityStats.idx4To7);
    PrintFutilityBucketRow("Index 8+:", g_futilityStats.idx8Plus);

    std::cout << "\n[By Move Type]\n";
    PrintFutilityBucketRow("Ordinary quiet:", g_futilityStats.ordinaryQuiet);
    PrintFutilityBucketRow("Killer quiet:", g_futilityStats.killerQuiet);
    PrintFutilityBucketRow("Quiet giving check:", g_futilityStats.quietGivingCheck);

    std::cout << "\n[By Static Evaluation Gap (alpha - staticEval)]\n";
    PrintFutilityBucketRow("Gap 0 to 149:", g_futilityStats.gap0To149);
    PrintFutilityBucketRow("Gap 150 to 299:", g_futilityStats.gap150To299);
    PrintFutilityBucketRow("Gap 300 to 499:", g_futilityStats.gap300To499);
    PrintFutilityBucketRow("Gap 500+:", g_futilityStats.gap500Plus);
}

PVSSearch::NullMoveStats g_nullMoveStats;

void RecordNullMoveAttempt(int depth, int R, int staticMargin, const Board &board, int turn, bool isCutoff, bool isVerified)
{
    g_nullMoveStats.totalAttempts++;
    if (isCutoff) {
        g_nullMoveStats.totalCutoffs++;
        if (isVerified) g_nullMoveStats.totalVerifiedCutoffs++;
        else g_nullMoveStats.totalFalseCutoffs++;
    } else {
        g_nullMoveStats.totalFailLow++;
    }

    // Depth
    if (depth == 4) g_nullMoveStats.depth4.recordAttempt(isCutoff, isVerified);
    else if (depth >= 5 && depth <= 6) g_nullMoveStats.depth5To6.recordAttempt(isCutoff, isVerified);
    else if (depth >= 7 && depth <= 8) g_nullMoveStats.depth7To8.recordAttempt(isCutoff, isVerified);
    else if (depth >= 9) g_nullMoveStats.depth9Plus.recordAttempt(isCutoff, isVerified);

    // Reduction R
    if (R == 3) g_nullMoveStats.r3.recordAttempt(isCutoff, isVerified);
    else if (R == 4) g_nullMoveStats.r4.recordAttempt(isCutoff, isVerified);

    // Margin
    if (staticMargin < 50) g_nullMoveStats.margin0To49.recordAttempt(isCutoff, isVerified);
    else if (staticMargin < 150) g_nullMoveStats.margin50To149.recordAttempt(isCutoff, isVerified);
    else if (staticMargin < 300) g_nullMoveStats.margin150To299.recordAttempt(isCutoff, isVerified);
    else g_nullMoveStats.margin300Plus.recordAttempt(isCutoff, isVerified);

    // Material Class
    int knights = PositionCorePieceListsView{board.positionCore}[turn * 8 + 2].size();
    int bishops = PositionCorePieceListsView{board.positionCore}[turn * 8 + 3].size();
    int rooks = PositionCorePieceListsView{board.positionCore}[turn * 8 + 4].size();
    int queens = PositionCorePieceListsView{board.positionCore}[turn * 8 + 5].size();
    int totalNonPawnCount = knights + bishops + rooks + queens;

    if (queens > 0) g_nullMoveStats.matQueen.recordAttempt(isCutoff, isVerified);
    else if (rooks > 0) g_nullMoveStats.matRookNoQueen.recordAttempt(isCutoff, isVerified);
    else g_nullMoveStats.matMinorsOnly.recordAttempt(isCutoff, isVerified);

    if (totalNonPawnCount == 1) g_nullMoveStats.matSinglePiece.recordAttempt(isCutoff, isVerified);
    else if (totalNonPawnCount >= 2) g_nullMoveStats.matMultiPiece.recordAttempt(isCutoff, isVerified);
}

void PrintNullBucketRow(const char *label, const PVSSearch::NullMoveBucket &b)
{
    double flPct = (b.attempts > 0) ? (100.0 * b.failLow / static_cast<double>(b.attempts)) : 0.0;
    double cutPct = (b.attempts > 0) ? (100.0 * b.cutoffs / static_cast<double>(b.attempts)) : 0.0;
    double verPct = (b.cutoffs > 0) ? (100.0 * b.verifiedCutoffs / static_cast<double>(b.cutoffs)) : 0.0;
    double falsePct = (b.cutoffs > 0) ? (100.0 * b.falseCutoffs / static_cast<double>(b.cutoffs)) : 0.0;

    char buf[256];
    snprintf(buf, sizeof(buf), "  %-24s Attempts: %6lu | Fail-low: %6lu (%5.1f%%) | Cutoffs: %6lu (%5.1f%%) | Confirmed: %6lu (%5.1f%%) | False: %4lu (%4.1f%%)\n",
             label, (unsigned long)b.attempts, (unsigned long)b.failLow, flPct, (unsigned long)b.cutoffs, cutPct, (unsigned long)b.verifiedCutoffs, verPct, (unsigned long)b.falseCutoffs, falsePct);
    std::cout << buf;
}

PVSSearch::NullMoveStats PVSSearch::GetNullMoveStatsForTesting()
{
    return g_nullMoveStats;
}

void PVSSearch::ResetNullMoveStatsForTesting()
{
    g_nullMoveStats = NullMoveStats{};
}

void PVSSearch::PrintNullMoveStatsForTesting()
{
    std::cout << "=== Null Move Pruning (NMP) Safety Stats (Recursive PVS) ===\n";
    std::cout << "Total null move attempts: " << g_nullMoveStats.totalAttempts << "\n";

    double flPct = (g_nullMoveStats.totalAttempts > 0) ? (100.0 * g_nullMoveStats.totalFailLow / static_cast<double>(g_nullMoveStats.totalAttempts)) : 0.0;
    double cutPct = (g_nullMoveStats.totalAttempts > 0) ? (100.0 * g_nullMoveStats.totalCutoffs / static_cast<double>(g_nullMoveStats.totalAttempts)) : 0.0;
    double verPct = (g_nullMoveStats.totalCutoffs > 0) ? (100.0 * g_nullMoveStats.totalVerifiedCutoffs / static_cast<double>(g_nullMoveStats.totalCutoffs)) : 0.0;
    double falsePct = (g_nullMoveStats.totalCutoffs > 0) ? (100.0 * g_nullMoveStats.totalFalseCutoffs / static_cast<double>(g_nullMoveStats.totalCutoffs)) : 0.0;

    char buf[256];
    snprintf(buf, sizeof(buf), "  Null move fail-lows:           %6lu (%5.1f%%)\n  Null move cutoffs:             %6lu (%5.1f%%)\n  Confirmed genuine cutoffs:     %6lu (%5.1f%% of cutoffs)\n  False cutoffs (failed verify): %6lu (%5.1f%% of cutoffs)\n",
             (unsigned long)g_nullMoveStats.totalFailLow, flPct, (unsigned long)g_nullMoveStats.totalCutoffs, cutPct, (unsigned long)g_nullMoveStats.totalVerifiedCutoffs, verPct, (unsigned long)g_nullMoveStats.totalFalseCutoffs, falsePct);
    std::cout << buf;

    std::cout << "\n[By Remaining Depth]\n";
    PrintNullBucketRow("Depth 4:", g_nullMoveStats.depth4);
    PrintNullBucketRow("Depth 5-6:", g_nullMoveStats.depth5To6);
    PrintNullBucketRow("Depth 7-8:", g_nullMoveStats.depth7To8);
    PrintNullBucketRow("Depth 9+:", g_nullMoveStats.depth9Plus);

    std::cout << "\n[By Reduction R]\n";
    PrintNullBucketRow("R = 3:", g_nullMoveStats.r3);
    PrintNullBucketRow("R = 4:", g_nullMoveStats.r4);

    std::cout << "\n[By Side-to-Move Material Class]\n";
    PrintNullBucketRow("Queen present:", g_nullMoveStats.matQueen);
    PrintNullBucketRow("Rook (no queen):", g_nullMoveStats.matRookNoQueen);
    PrintNullBucketRow("Minor pieces only:", g_nullMoveStats.matMinorsOnly);
    PrintNullBucketRow("Single non-pawn piece:", g_nullMoveStats.matSinglePiece);
    PrintNullBucketRow(">=2 non-pawn pieces:", g_nullMoveStats.matMultiPiece);

    std::cout << "\n[By Static Margin (staticEval - beta)]\n";
    PrintNullBucketRow("Margin 0 to 49:", g_nullMoveStats.margin0To49);
    PrintNullBucketRow("Margin 50 to 149:", g_nullMoveStats.margin50To149);
    PrintNullBucketRow("Margin 150 to 299:", g_nullMoveStats.margin150To299);
    PrintNullBucketRow("Margin 300+:", g_nullMoveStats.margin300Plus);
}

PVSSearch::MoveOrderingQualityStats& PVSSearch::GetMoveOrderingQualityStatsForTesting()
{
    return g_orderingQualityStats;
}

void PVSSearch::ResetMoveOrderingQualityStatsForTesting()
{
    g_orderingQualityStats.reset();
}

int PVSSearch::DiagnosticCombinedHistory(int side, const Move &prevMove, const Move &move)
{
    return CombinedHistoryScore(side, prevMove, move);
}

int PVSSearch::DiagnosticMainHistory(int side, const Move &move)
{
    return HistoryScore(side, move);
}

int PVSSearch::DiagnosticContinuationHistory(const Move &prevMove, const Move &move)
{
    (void)prevMove;
    (void)move;
    return 0;
}

int PVSSearch::DiagnosticUnifiedOrderingScore(int turn, const Move &prevMove, int depthGone, const Move *m)
{
    const bool isQuiet = IsQuietMove(*m);
    if (!isQuiet)
    {
        if (m->value >= 0)
            return 1000000 + BasePvsOrderingScore(m);
        else
            return BasePvsOrderingScore(m);
    }

    if (depthGone >= 0 && depthGone < PVSSearch::MaxKillerPly)
    {
        if (PVSSearch::killers[depthGone][0] == *m)
            return 500000 + (m->givesCheck ? CheckOrderingBonus : 0);
        if (PVSSearch::killers[depthGone][1] == *m)
            return 400000 + (m->givesCheck ? CheckOrderingBonus : 0);
    }

    return 100000 + CombinedHistoryScore(turn, prevMove, *m) + BasePvsOrderingScore(m);
}
#endif

namespace
{
struct TargetResult {
    int value = 0;
    std::string pv;
    uint16_t provenance = 0;
    bool selective = false;
};

void UpdateCounterMove(const Board& board, const Move& move)
{
    StoreCounterMove(board, 0, move);
}

int CaptureOrder(const Board& b, const Move& m)
{
    static constexpr int v[7]={0,100,350,350,550,975,2500};
    const int mover=b.positionCore.pieceAt[m.beginPlace];
    const int victim=m.endPiece>8?m.endPiece-8:m.endPiece;
    const int captured=std::clamp(victim,0,6);
    return SearchParameters::MoveOrdering::CaptureVictimMultiplier*v[captured]+
        captureHistory[mover][m.endPlace][captured];
}

class ProbCutPicker
{
public:
    ProbCutPicker(Board& boardValue, int depth, int ply, uint16_t ttMoveValue,
                  int seeThresholdValue)
        : board(boardValue), ttMove(ttMoveValue), seeThreshold(seeThresholdValue)
    {
        moves = MoveLogic::MoveGenerator(board, depth, ply, true, false);
        for (int i = 0; i < moves.count; ++i)
        {
            Move* move = &moves[i];
            if (move->endPiece % 8 == 6)
                continue;
            if (move->endPiece == 0 && move->promotionPiece <= 0)
                continue;
            if (!MoveLogic::SEE_GE(board, *move, seeThreshold))
                continue;
            entries[count++] = {move, CaptureOrder(board, *move), false};
            if (MatchesPackedMove(*move, ttMove))
                ttIndex = count - 1;
        }
    }

    Move* Next()
    {
        if (!ttReturned && ttIndex >= 0)
        {
            ttReturned = true;
            entries[ttIndex].returned = true;
            return entries[ttIndex].move;
        }
        int best = -1;
        for (int i = 0; i < count; ++i)
            if (!entries[i].returned &&
                (best < 0 || entries[i].score > entries[best].score))
                best = i;
        if (best < 0)
            return nullptr;
        entries[best].returned = true;
        return entries[best].move;
    }

private:
    struct Entry { Move* move; int score; bool returned; };
    Board& board;
    MoveList moves;
    uint16_t ttMove;
    int seeThreshold;
    std::array<Entry, 256> entries;
    int count = 0;
    int ttIndex = -1;
    bool ttReturned = false;
};

int CapturedEndgameValue(const Move& move)
{
    static const int values[7] = {0, Option::PawnValue, Option::KnightValue,
        Option::BishopValue, Option::RookValue, Option::QueenValue, 0};
    return values[std::clamp(move.endPiece % 8, 0, 6)];
}

bool PackedMoveIsTactical(const Board& board, uint16_t packed)
{
    if (packed == 0)
        return false;
    const int from = TTMoveHelper::UnpackFrom(packed);
    const int to = TTMoveHelper::UnpackTo(packed);
    return TTMoveHelper::UnpackPromotion(packed) != 0 || board.positionCore.pieceAt[to] != 0 ||
        (board.positionCore.pieceAt[from] % 8 == 1 && to == board.unpassentPlace);
}

int QuietOrder(int side,int ply,const Move&m)
{
    return QuietOrderingScore(side, ply, 1, m);
}

TargetResult TargetSearch(bool pv,int alpha,int beta,int depth,Move& prev,
                          Move& m1,Move& m2,Move& m3,Board& b,bool nullAllowed,
                          int ply,bool cutNode)
{
    if (Search::stopRequested.load(std::memory_order_relaxed)) return {0,{}};
    if (ply > 0 && RepetitionHistory::IsRepetition(b.ZobristHashCode)) return {0,{}};
    if (depth <= 0) {
        std::unique_ptr<MovePrintValue> q(QSearcher::QSearch(pv,alpha,beta,prev,ply,0,false,depth,m1,m2,m3,b,false,ply,beta-alpha==1));
        return {q->value,q->printString,q->provenance,q->selective};
    }
    ++Search::searchNodeCount;
    if (Search::strictNodeLimit || (Search::searchNodeCount & 2047) == 0)
        Search::CheckLimits();
    if (Search::stopRequested.load(std::memory_order_relaxed)) return {0,{}};
    alpha=std::max(alpha,MateScore::MatedAtPly(ply));
    beta=std::min(beta,MateScore::MateAtPly(ply+1));
    if(alpha>=beta) return {alpha,{}};

    const int side=b.sideToMove?1:0;
    const bool inCheck=BoardLogic::UnderAttack(b,PositionCorePieceListsView{b.positionCore}[side*8+6].front(),!b.sideToMove);
    if (ply >= SearchStackSize - 1)
        return {inCheck ? 0 : StructuredNNUE::Evaluate(b), {}};
    TargetSearchFrameGuard targetFrameGuard;
    TriedMoveStorage& triedMoves = targetFrameGuard.Get();
    SearchStackFrame& ss=StackFrame(ply);
    ss.ply=ply; ss.currentMove=prev;
    ss.hasCurrentMove=prev.beginPlace>=0&&prev.endPlace>=0&&prev.promotionPiece!=-1;
    if (ss.currentMovedPiece == 0 && ss.hasCurrentMove)
        ss.currentMovedPiece = std::clamp(static_cast<int>(b.positionCore.pieceAt[prev.endPlace]),0,14);
    ss.moveCount=0; ss.pvNode=pv;
    if(ply<PVSSearch::MaxKillerPly){ss.killer1=&PVSSearch::killers[ply][0];ss.killer2=&PVSSearch::killers[ply][1];}

    const uint64_t key=SearchKey(b.ZobristHashCode,ss.excludedMove);
    TTEntry tt{};
    bool hit=TranspositionTable::Probe(key,tt);
    ttHitAverage = static_cast<int>(
        int64_t(TtHitAverageWindow - 1) * ttHitAverage / TtHitAverageWindow) +
        1024 * int(hit);
    int ttValue=hit?MateScore::FromTranspositionTable(tt.score,ply):0;
    uint8_t ttBound=hit?TTBaseFlag(tt.flag):TT_NONE;
    bool ttPv=pv||(hit&&(tt.metadata&TT_META_PV));
    if(!pv&&hit&&tt.depth>=depth&&
       (ttBound==TT_EXACT||(ttBound==TT_LOWER_BOUND&&ttValue>=beta)||
        (ttBound==TT_UPPER_BOUND&&ttValue<=alpha))) {
        if(tt.bestMove!=0) {
            Move tm=UnpackMove(tt.bestMove);
            const bool quiet=b.positionCore.pieceAt[tm.endPlace]==0&&tm.promotionPiece<=0;
            const int movingPiece=std::clamp(static_cast<int>(b.positionCore.pieceAt[tm.beginPlace]),0,14);
            if(quiet&&ttBound==TT_LOWER_BOUND&&ttValue>=beta){
                PVSSearch::RecordKiller(ply,tm);StoreCounterMove(b,ply,tm);
                UpdateHistory(side,tm,depth,true);
                UpdateContinuationHistories(ply,movingPiece,tm,depth,true);
                if(movingPiece%8!=1){Move reverse=tm;std::swap(reverse.beginPlace,reverse.endPlace);
                    UpdateHistoryByBonus(side,reverse,-StatBonus(depth));}
                if(ss.hasCurrentMove&&IsQuietMove(prev)&&
                   StackFrame(ply-1).moveCount<=SearchParameters::History::TtCutoffPreviousMoveCount)
                    UpdateContinuationHistoriesByBonus(
                        ply-1,ss.currentMovedPiece,prev,-StatBonus(depth+1));
            }
            else if(quiet&&ttBound==TT_UPPER_BOUND&&ttValue<=alpha){
                UpdateHistory(side,tm,depth,false);
                UpdateContinuationHistories(ply,movingPiece,tm,depth,false);
            }
        }
        TranspositionTable::RecordCutoff(); return {ttValue,{}};
    }
    int tablebaseMaxValue=200000;
    const bool tablebaseEligible = ss.excludedMove==0 && ply>0 &&
        Option::SyzygyProbeLimit>0 && Tablebase::IsAvailable() &&
        Tablebase::PieceCount(b)<=Option::SyzygyProbeLimit &&
        Tablebase::PieceCount(b)<=Tablebase::MaxPieces() &&
        Tablebase::IsPositionStateSupported(b);
    if(tablebaseEligible) {
        const std::optional<Tablebase::Wdl> wdl=Tablebase::ProbeWdl(b,Option::SyzygyProbeLimit);
        if(wdl.has_value()){
            Search::tablebaseHits.fetch_add(1,std::memory_order_relaxed);
            const int tbValue=Tablebase::Score(*wdl);
            const uint8_t tbBound=*wdl==Tablebase::Wdl::Draw?TT_EXACT:
                (*wdl==Tablebase::Wdl::Win?TT_LOWER_BOUND:TT_UPPER_BOUND);
            TranspositionTable::Store(key,MateScore::ToTranspositionTable(tbValue,ply),
                std::min(SearchStackSize-1,depth+6),tbBound,0,TT_NO_STATIC_EVAL,pv);
            if(tbBound==TT_EXACT||(tbBound==TT_LOWER_BOUND&&tbValue>=beta)||
               (tbBound==TT_UPPER_BOUND&&tbValue<=alpha))
                return {tbValue,{}};
            if(pv&&tbBound==TT_LOWER_BOUND) alpha=std::max(alpha,tbValue);
            if(pv&&tbBound==TT_UPPER_BOUND) tablebaseMaxValue=tbValue;
        }
    }

    int rawStaticEval=NoStaticEval,eval=NoStaticEval;
    if(!inCheck) {
        if(prev.promotionPiece==-1&&ss.staticEval!=NoStaticEval)
            rawStaticEval=ss.staticEval;
        else if(hit&&tt.staticEval!=TT_NO_STATIC_EVAL) rawStaticEval=tt.staticEval;
        else {
            rawStaticEval=StructuredNNUE::Evaluate(b)-
                ss.statScore/SearchParameters::StaticEvaluation::StatScoreDivisor;
            if(ss.excludedMove==0)
                TranspositionTable::Store(key,0,-128,TT_EVAL_ONLY,0,rawStaticEval,pv);
        }
        ss.staticEval=rawStaticEval; eval=rawStaticEval;
        if(hit&&ttBound==TT_LOWER_BOUND&&ttValue>eval) eval=ttValue;
        else if(hit&&ttBound==TT_UPPER_BOUND&&ttValue<eval) eval=ttValue;
    } else ss.staticEval=NoStaticEval;
    bool improving=false;
    if(!inCheck){const auto&s2=StackFrame(ply-2);const auto&s4=StackFrame(ply-4);improving=s2.staticEval!=NoStaticEval?rawStaticEval>=s2.staticEval:(s4.staticEval==NoStaticEval||rawStaticEval>=s4.staticEval);}

    if(!pv&&!inCheck&&ss.excludedMove==0&&alpha>-MateScore::Threshold&&beta<MateScore::Threshold){
        if(depth<SearchParameters::Razoring::MaxDepth&&
           eval<=alpha-SearchParameters::Razoring::Margin){std::unique_ptr<MovePrintValue>q(QSearcher::QSearch(false,alpha,beta,prev,ply,0,false,0,m1,m2,m3,b,false,ply,true));if(q->value<=alpha)return {q->value,q->printString};}
        if(depth<SearchParameters::ReverseFutility::MaxDepth&&
           eval-SearchParameters::ReverseFutility::DepthMargin*(depth-int(improving))>=beta&&
           eval<MateScore::Threshold)return {eval,{}};
    }
    const bool suppressed=nullSuppressedSide==side&&ply<=nullSuppressedUntilPly;
    if(!pv&&!inCheck&&nullAllowed&&!suppressed&&ss.excludedMove==0&&prev.promotionPiece!=-1&&
       ss.statScore<SearchParameters::NullMove::StatScoreLimit&&NullMoveMaterialEligible(b,side)&&eval>=beta&&eval>=rawStaticEval&&
       rawStaticEval>=beta-SearchParameters::NullMove::StaticEvalDepthSlope*depth+
           SearchParameters::NullMove::StaticEvalBase-int(improving)*SearchParameters::NullMove::ImprovingMargin){
        int R=(SearchParameters::NullMove::Base+SearchParameters::NullMove::DepthSlope*depth)/
            SearchParameters::NullMove::Divisor+
            std::min((eval-beta)/SearchParameters::NullMove::EvalDivisor,
                     SearchParameters::NullMove::MaxEvalBonus); R=std::clamp(R,1,depth);
        Move n{}; n.promotionPiece=-1; MissingInfoAboutPrevStateFromMove u(b);
        GameLogic::DoMove(b,n,prev,ply,ply); PrepareChildStack(ply+1,n,0,0);
        StackFrame(ply+1).staticEval=-rawStaticEval+2*TempoForPosition(b);
        TargetResult nr=TargetSearch(false,-beta,-beta+1,depth-R,n,m2,m3,prev,b,false,ply+1,!cutNode);
        int score=-nr.value; GameLogic::UndoMove(b,n,u);
        if(score>=beta){if(MateScore::IsMate(score))score=beta;if(depth<SearchParameters::NullMove::VerificationDepth)return {score,{}};
            int os=nullSuppressedSide,ou=nullSuppressedUntilPly;nullSuppressedSide=side;nullSuppressedUntilPly=ply+
                SearchParameters::NullMove::SuppressionNumerator*(depth-R)/
                SearchParameters::NullMove::SuppressionDenominator;
            TargetResult vr=TargetSearch(false,beta-1,beta,depth-R,prev,m1,m2,m3,b,false,ply,false);
            nullSuppressedSide=os;nullSuppressedUntilPly=ou;if(vr.value>=beta)return {score,{}};}
    }
    if(ss.excludedMove==0&&depth>=5&&!pv&&!inCheck&&std::abs(beta)<MateScore::Threshold){
        const int raised=beta+SearchParameters::ProbCut::BetaMargin-
            SearchParameters::ProbCut::ImprovingMargin*int(improving);
        ProbCutPicker probCutPicker(b,depth,ply,hit?tt.bestMove:0,
                                    raised-rawStaticEval);
        int tried=0;
        while(Move*c=probCutPicker.Next()){if(tried>=ProbCutMaxCandidatesBase+2*int(cutNode))break;
            const int moved=b.positionCore.pieceAt[c->beginPlace];
            MissingInfoAboutPrevStateFromMove u(b,*c);GameLogic::DoMove(b,*c,prev,depth,ply,&u);
            if(BoardLogic::UnderAttack(b,PositionCorePieceListsView{b.positionCore}[side*8+6].front(),b.sideToMove)){GameLogic::UndoMove(b,*c,u);continue;}++tried;
            std::unique_ptr<MovePrintValue>q(QSearcher::QSearch(false,-raised,-raised+1,*c,ply+1,0,false,0,m2,m3,prev,b,false,ply+1,true));int qs=-q->value;
            if(qs>=raised){PrepareChildStack(ply+1,*c,moved,0);TargetResult pc=TargetSearch(false,-raised,-raised+1,depth-4,*c,m2,m3,prev,b,true,ply+1,!cutNode);qs=-pc.value;}
            GameLogic::UndoMove(b,*c,u);if(qs>=raised)return {qs,{}};}
    }
    if(ss.excludedMove==0&&depth>=7&&(!hit||tt.bestMove==0)){
        TargetSearch(pv,alpha,beta,depth-7,prev,m1,m2,m3,b,nullAllowed,ply,cutNode);
        const bool iidHit=TranspositionTable::Probe(key,tt);
        if(iidHit){hit=true;ttValue=MateScore::FromTranspositionTable(tt.score,ply);
            ttBound=TTBaseFlag(tt.flag);ttPv=pv||(tt.metadata&TT_META_PV);}
    }

    const bool ttCapture=hit&&PackedMoveIsTactical(b,tt.bestMove);
    MovePicker picker(b,depth,ply,side,prev,hit?tt.bestMove:0);
    int oldAlpha=alpha,best=-200000,legal=0; uint16_t bestPacked=0;
    Move* bestMove=nullptr; std::string bestPv;
    uint16_t bestProvenance = 0;
    bool bestSelective = false;
    bool pruneQuiets=false;
    while(Move* move=picker.Next(pruneQuiets)){
        if(TTMoveHelper::PackMove(*move)==ss.excludedMove)continue;
        ++ss.moveCount; const bool quiet=IsQuietMove(*move); const bool tactical=!quiet;
        pruneQuiets=ply>0&&NullMoveMaterialEligible(b,side)&&
            best>MateScore::MatedAtPly(SearchStackSize-1)&&
            ss.moveCount>=FutilityMoveCount(improving,depth);
        const int movingPiece=std::clamp(static_cast<int>(b.positionCore.pieceAt[move->beginPlace]),0,14);
        if(!move->givesCheckComputed){move->givesCheck=MoveLogic::MoveGivesCheck(b,*move);move->givesCheckComputed=true;}
        const bool givesCheck=move->givesCheck;
        int extension=0; bool singularLMR=false;
        if(hit&&tt.bestMove==TTMoveHelper::PackMove(*move)&&ss.excludedMove==0&&depth>=6&&
           ttValue>-MateScore::Threshold&&ttValue<MateScore::Threshold&&
           (ttBound==TT_LOWER_BOUND||ttBound==TT_EXACT)&&tt.depth>=depth-3){
            const int sb=ttValue-2*depth; ss.excludedMove=tt.bestMove;
            TargetResult sr=TargetSearch(false,sb-1,sb,depth/2,prev,m1,m2,m3,b,nullAllowed,ply,cutNode);
            ss.excludedMove=0;
            if(sr.value<sb){extension=1;singularLMR=true;}else if(sb>=beta)return {sb,{}};
        }
        const bool discoveredCheck=givesCheck&&!MovedPieceGivesCheck(b,*move);
        const bool actualCastle=movingPiece%8==6&&std::abs(move->endPlace-move->beginPlace)==2;
        if(!extension&&givesCheck&&(discoveredCheck||MoveLogic::SEE_GE(b,*move,0)))extension=1;
        else if(!extension&&IsKillerMove(ply,*move)&&IsAdvancedPassedPawnPush(b,*move,side))extension=1;
        else if(!extension&&LastCaptureExtension(b,*move))extension=1;
        else if(!extension&&actualCastle)extension=1;
        const int childDepth=depth-1+extension;
        const bool lmr=depth>=3&&ss.moveCount>1&&
            (quiet||pruneQuiets||rawStaticEval+CapturedEndgameValue(*move)<=alpha||cutNode||
             ttHitAverage/TtHitAverageWindow<SearchParameters::LMR::TtHitLowThreshold);
        const QuietHistoryValues quietHistory=quiet
            ? ReadQuietHistory(side,ply,movingPiece,*move) : QuietHistoryValues{};
        const int currentStat=quiet?quietHistory.statScore():0;
        StackFrame(ply+1).statScore=currentStat;
        const int previousStat=ss.statScore;
        int lmrDepth=childDepth;
        if(lmr){
            int r=Reduction(improving,depth,ss.moveCount);
            if(ttPv)r-=SearchParameters::LMR::TtPvAdjustment;
            if(ttHitAverage/TtHitAverageWindow>SearchParameters::LMR::TtHitHighThreshold)--r;
            if(StackFrame(ply-1).moveCount>SearchParameters::LMR::PreviousMoveCountThreshold)--r;
            if(singularLMR)r-=SearchParameters::LMR::SingularAdjustment;
            if(quiet){
                if(ttCapture)r+=SearchParameters::LMR::TtCaptureAdjustment;
                if(cutNode)r+=SearchParameters::LMR::CutNodeAdjustment;
                else if(movingPiece%8!=6 || std::abs(move->endPlace-move->beginPlace)!=2){
                    Move reverse=*move;std::swap(reverse.beginPlace,reverse.endPlace);
                    reverse.PublicFlag=0;reverse.promotionPiece=0;
                    if(!MoveLogic::SEE_GE(b,reverse,0))r-=SearchParameters::LMR::EscapeCaptureAdjustment;
                }
                if(currentStat>=SearchParameters::LMR::GoodStatScore&&
                   previousStat<SearchParameters::LMR::BadPreviousStatScore)--r;
                else if(previousStat>=SearchParameters::LMR::GoodPreviousStatScore&&
                        currentStat<SearchParameters::LMR::BadStatScore)++r;
                r-=currentStat/SearchParameters::LMR::StatScoreDivisor;
            } else if(depth<SearchParameters::LMR::ShallowCaptureDepth&&
                      ss.moveCount>SearchParameters::LMR::LateCaptureMoveCount) ++r;
            r=std::clamp(r,0,std::max(0,childDepth-1));lmrDepth=childDepth-r;
        }
        if(ply>0&&NullMoveMaterialEligible(b,side)&&best>MateScore::MatedAtPly(SearchStackSize-1)){
            if(quiet&&!givesCheck){
                const int combined=quietHistory.combinedHistory();
                const SearchStackFrame& previousFrame=StackFrame(ply-1);
                if(lmrDepth<SearchParameters::CounterMovePruning::ReducedDepthBase+
                    (previousFrame.statScore>0||previousFrame.moveCount==1)&&
                   quietHistory.c1<SearchParameters::CounterMovePruning::ContinuationThreshold&&
                   quietHistory.c2<SearchParameters::CounterMovePruning::ContinuationThreshold)continue;
                if(lmrDepth<SearchParameters::ParentFutility::MaxReducedDepth&&!inCheck&&
                   rawStaticEval+SearchParameters::ParentFutility::BaseMargin+
                   SearchParameters::ParentFutility::DepthMargin*lmrDepth<=alpha&&
                   combined<SearchParameters::History::ParentFutilityLimit)continue;
                const int see=-(SearchParameters::SEE::QuietBase-
                    std::min(lmrDepth,SearchParameters::SEE::QuietDepthCap))*lmrDepth*lmrDepth;
                if(!MoveLogic::SEE_GE(b,*move,see))continue;
            } else if(!MoveLogic::SEE_GE(b,*move,
                       -SearchParameters::SEE::TacticalDepthMargin*depth))continue;
        }
        MissingInfoAboutPrevStateFromMove u(b,*move);GameLogic::DoMove(b,*move,prev,depth,ply,&u);
        if(BoardLogic::UnderAttack(b,PositionCorePieceListsView{b.positionCore}[side*8+6].front(),b.sideToMove)){GameLogic::UndoMove(b,*move,u);--ss.moveCount;continue;}
        ++legal; PrepareChildStack(ply+1,*move,movingPiece,currentStat);
        TargetResult child; int value;
        if(pv&&legal==1){child=TargetSearch(true,-beta,-alpha,childDepth,*move,m2,m3,prev,b,true,ply+1,false);value=-child.value;}
        else {child=TargetSearch(false,-alpha-1,-alpha,lmrDepth,*move,m2,m3,prev,b,true,ply+1,lmr?true:!cutNode);value=-child.value;
            if(lmr&&value>alpha&&lmrDepth!=childDepth){child=TargetSearch(false,-alpha-1,-alpha,childDepth,*move,m2,m3,prev,b,true,ply+1,!cutNode);value=-child.value;
                if(quiet){int bonus=value>alpha?StatBonus(childDepth):-StatBonus(childDepth);
                    if(ply<PVSSearch::MaxKillerPly&&PVSSearch::killers[ply][0]==*move)
                        bonus+=bonus/SearchParameters::History::LmrKillerBonusDivisor;
                    UpdateContinuationHistoriesByBonus(ply,movingPiece,*move,bonus);}}
            if(pv&&value>alpha&&value<beta){child=TargetSearch(true,-beta,-alpha,childDepth,*move,m2,m3,prev,b,true,ply+1,false);value=-child.value;}}
        GameLogic::UndoMove(b,*move,u);
        if(quiet)
            AppendTriedMove(triedMoves.quiet, triedMoves.quietCount, move, "quiet");
        else
            AppendTriedMove(triedMoves.captures, triedMoves.captureCount, move, "capture");
        if(value>best){best=value;bestMove=move;bestPacked=TTMoveHelper::PackMove(*move);bestPv=ChessStringManipulation::PVToString(*move,0,false,b)+(child.pv.empty()?"":" "+child.pv);bestProvenance=child.provenance;bestSelective=child.selective;
            if(value>alpha){alpha=value;if(value>=beta)break;}}
    }
    if(legal==0)best=ss.excludedMove?alpha:(inCheck?MateScore::MatedAtPly(ply):0);
    else if(bestMove)RewardCutoffMove(
        b,side,ply,depth,best,beta,*bestMove,
        triedMoves.quiet.data(),triedMoves.quietCount,
        triedMoves.captures.data(),triedMoves.captureCount);
    else if((depth>=3||pv)&&ss.hasCurrentMove&&IsQuietMove(prev))
        UpdateContinuationHistoriesByBonus(
            ply-1,ss.currentMovedPiece,prev,StatBonus(depth));
    best=std::min(best,tablebaseMaxValue);
    const uint8_t flag=best>=beta?TT_LOWER_BOUND:(best<=oldAlpha?TT_UPPER_BOUND:TT_EXACT);
    if(ss.excludedMove==0&&!Search::stopRequested.load(std::memory_order_relaxed))
        TranspositionTable::Store(key,MateScore::ToTranspositionTable(best,ply),depth,flag,bestPacked,rawStaticEval,pv||ttPv);
    return {best,bestPv,bestProvenance,bestSelective};
}
}

MovePrintValue *PVSSearch::PVS(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, bool selectiveSearch, bool cutNode)
{
    (void)MAtESearch;(void)previousMoveWasCheck;(void)selectiveSearch;
    TargetResult r=TargetSearch(isPVNode,alpha,beta,depth,prevMove,move1,move2,move3,board4,isNullMoveAllowed,depthGone,cutNode);
    auto*out=new MovePrintValue();out->value=r.value;out->printString=r.pv;out->provenance=r.provenance;out->selective=r.selective;out->bound=r.value>=beta?SearchBound::Lower:(r.value<=alpha?SearchBound::Upper:SearchBound::Exact);out->proof=ExactProof;
    if(nullWindowSearch) out->AddProvenance(SearchProvenance::NullVerification);
    return out;
}

void PVSSearch::deleteMoveList(MoveList) {}

double PVSSearch::NullMoveReduction(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool mateSearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch)
{
    Move nullMove = Move();
    nullMove.promotionPiece = -1;

    Board *boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
    MissingInfoAboutPrevStateFromMove missingInfoAboutPrevStateFromMove(board4);
    GameLogic::DoMove(board4, nullMove, prevMove, depthGone, depthGone);
    double valueReduced;
    std::unique_ptr<MovePrintValue> childResult;
    if (depth > 6)
    {
        childResult.reset(PVS(false, -beta, -beta + 1, depth - 4 - 1, nullMove, move2, move3, prevMove, board4, mateSearch, false, depthGone + 1, previousMoveWasCheck, true));
        valueReduced = -childResult->value;
    }
    else if (depth >= 4)
    {
        childResult.reset(PVS(false, -beta, -beta + 1, depth - 3 - 1, nullMove, move2, move3, prevMove, board4, mateSearch, false, depthGone + 1, previousMoveWasCheck, true));
        valueReduced = -childResult->value;
    }
    else
    {
        childResult.reset(QSearcher::QSearch(false, -beta, -beta + 1, nullMove, depthGone + 1, 0, true, 0, move2, move3, prevMove, board4, false, depthGone + 1, nullWindowSearch));
        valueReduced = -childResult->value;
    }
    GameLogic::UndoMove(board4, nullMove, missingInfoAboutPrevStateFromMove);
    if (UCI::IsTest())
    {
        Board::AreBoardsEqual(board4, *boardCopy);
        delete boardCopy;
        boardCopy = nullptr;
    }
    return valueReduced;
}
