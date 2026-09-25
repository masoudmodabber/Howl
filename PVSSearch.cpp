#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "PVSSearch.h"
#include "Search.h"
#include "BoardLogic.h"
#include "EvaluationLogic.h"
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
#include <iostream>
#include <algorithm>
#include <array>
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
    constexpr int ProbCutPickerLimit = 8; // Legacy inactive SearchNode storage bound.
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
        const int movingPiece = std::clamp(board.mainBoard[move.beginPlace], 0, 14);
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
                          const std::vector<Move*>& searchedQuiets,
                          const std::vector<Move*>& searchedCaptures)
    {
        const int bonus1 = StatBonus(depth + 1);
        const int bonus2 = bestValue > beta + Option::PawnValue
            ? bonus1 : StatBonus(depth);
        if (IsQuietMove(move))
        {
            PVSSearch::RecordKiller(ply, move);
            UpdateHistoryByBonus(side, move, bonus2);
            UpdateContinuationHistoriesByBonus(
                ply, board.mainBoard[move.beginPlace], move, bonus2);
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
            for (Move* prior : searchedQuiets)
                if (prior != &move)
                {
                    UpdateHistoryByBonus(side, *prior, -bonus2);
                    UpdateContinuationHistoriesByBonus(
                        ply, board.mainBoard[prior->beginPlace], *prior, -bonus2);
                }
            if (board.mainBoard[move.beginPlace] % 8 != 1)
            {
                Move reverse = move;
                std::swap(reverse.beginPlace, reverse.endPlace);
                UpdateHistoryByBonus(side, reverse, -bonus2);
            }
        }
        else
        {
            const int movingPiece = std::clamp(board.mainBoard[move.beginPlace], 0, 14);
            const int capturedPiece = std::clamp(move.endPiece % 8, 0, 6);
            int& winning = captureHistory[movingPiece][move.endPlace][capturedPiece];
            winning += bonus1 - winning * std::abs(bonus1) / CaptureHistoryLimit;
            for (Move* prior : searchedCaptures)
                if (prior != &move)
                {
                    const int priorPiece = std::clamp(board.mainBoard[prior->beginPlace], 0, 14);
                    const int priorCaptured = std::clamp(prior->endPiece % 8, 0, 6);
                    int& failed = captureHistory[priorPiece][prior->endPlace][priorCaptured];
                    failed += -bonus1 - failed * std::abs(bonus1) / CaptureHistoryLimit;
                }
        }
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
                   uint16_t ttMoveValue)
            : board(boardValue), depth(depthValue), depthGone(depthGoneValue),
              turn(turnValue), previousMove(previousMoveValue), packedTTMove(ttMoveValue),
              packedCounterMove(CounterMoveFor(boardValue, depthGoneValue)) {}

        ~MovePicker()
        {
            for (int i = 0; i < entryCount; ++i)
                delete entries[i].move;
        }

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
                    if (Move* move = Select(remainingQuiets, remainingQuietCount,
                                            remainingQuietCursor))
                        return move;
                    stage = Stage::BadTactical;
                    break;
                case Stage::BadTactical:
                    GenerateTactical();
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
        struct Entry
        {
            Move* move = nullptr;
            int key = 0;
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

        bool MatchesTT(const Move& move) const
        {
            if (packedTTMove == 0)
                return false;
            return move.beginPlace == TTMoveHelper::UnpackFrom(packedTTMove) &&
                   move.endPlace == TTMoveHelper::UnpackTo(packedTTMove) &&
                   (move.promotionPiece > 0 ? move.promotionPiece : 0) ==
                       TTMoveHelper::UnpackPromotion(packedTTMove);
        }

        int Add(Move* move)
        {
            if (entryCount >= static_cast<int>(entries.size()))
            {
                delete move;
                return -1;
            }
            Entry& entry = entries[entryCount];
            entry.move = move;
            entry.move->depth = depth;
            entry.move->depthGone = depthGone;
            entry.move->moveCount = Search::moveCount;
            entry.returned = false;
            entry.scored = false;
            return entryCount++;
        }

        Move* MarkReturned(int index)
        {
            entries[index].returned = true;
            Score(index);
            return entries[index].move;
        }

        Move* Select(std::array<int, 256>& indices, int count, int& cursor)
        {
            if (cursor >= count)
                return nullptr;
            int best = cursor;
            for (int i = cursor + 1; i < count; ++i)
            {
                if (entries[indices[i]].returned)
                    continue;
                if (entries[indices[best]].returned ||
                    entries[indices[i]].key > entries[indices[best]].key)
                    best = i;
            }
            std::swap(indices[cursor], indices[best]);
            const int index = indices[cursor++];
            return entries[index].returned ? Select(indices, count, cursor) : MarkReturned(index);
        }

        void EnsureAttackers()
        {
            if (attackersReady)
                return;
            whiteAttacker = MoveLogic::SetWhiteAttacker(board);
            blackAttacker = MoveLogic::SetBlackAttacker(board);
            attackersReady = true;
        }

        void Score(int index)
        {
            Entry& entry = entries[index];
            if (entry.scored)
                return;
            EnsureAttackers();
            if (!entry.move->givesCheckComputed)
            {
                entry.move->givesCheck = MoveLogic::MoveGivesCheck(board, *entry.move);
                entry.move->givesCheckComputed = true;
            }
            MoveLogic::ScoreMove(board, *entry.move, whiteAttacker, blackAttacker);
            entry.scored = true;
        }

        bool TTHintIsTactical() const
        {
            if (packedTTMove == 0)
                return false;
            const int from = TTMoveHelper::UnpackFrom(packedTTMove);
            const int to = TTMoveHelper::UnpackTo(packedTTMove);
            if (TTMoveHelper::UnpackPromotion(packedTTMove) != 0 || board.mainBoard[to] != 0)
                return true;
            return board.mainBoard[from] % 8 == 1 && to == board.unpassentPlace;
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
            MoveList generated = MoveLogic::MoveGenerator(
                board, depth, depthGone, true, false, emptyWhite, emptyBlack, false);
            for (int i = 0; i < generated.count; ++i)
            {
                Move* candidate = generated.moves[i];
                if (candidate->endPiece % 8 == 6)
                {
                    delete candidate;
                    continue;
                }
                const int index = Add(candidate);
                if (index >= 0)
                {
                    const int movingPiece = board.mainBoard[entries[index].move->beginPlace];
                    const int capturedPiece = entries[index].move->endPiece;
                    static constexpr int MgValue[7] = {0, 100, 320, 330, 500, 900, 0};
                    const int victimType = capturedPiece % 8;
                    entries[index].key = SearchParameters::MoveOrdering::CaptureVictimMultiplier *
                        MgValue[std::min(victimType, 6)] +
                        captureHistory[std::clamp(movingPiece, 0, 14)]
                                      [entries[index].move->endPlace]
                                      [std::clamp(capturedPiece % 8, 0, 6)];
                    if (MatchesTT(*entries[index].move))
                        ttEntry = index;
                    if (MoveLogic::SEE_GE(board, *entries[index].move,
                                         -SearchParameters::SEE::GoodCaptureCoefficient *
                                         entries[index].key /
                                         SearchParameters::MoveOrdering::GoodCaptureSeeDivisor))
                        goodTactical[goodCount++] = index;
                    else
                        badTactical[badCount++] = index;
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
                if (!entries[index].returned &&
                    entries[index].move->beginPlace == killer.beginPlace &&
                    entries[index].move->endPlace == killer.endPlace &&
                    (entries[index].move->promotionPiece > 0
                         ? entries[index].move->promotionPiece : 0) == killer.promotionPiece)
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
                if (!entries[index].returned && MatchesPackedMove(*entries[index].move, packedMove))
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
            MoveList generated = MoveLogic::MoveGenerator(
                board, depth, depthGone, false, false, emptyWhite, emptyBlack, true);
            for (int i = 0; i < generated.count; ++i)
            {
                Move* candidate = generated.moves[i];
                if (IsQuietMove(*candidate))
                {
                    const int index = Add(candidate);
                    if (index >= 0)
                    {
                        quietEntries[quietEntryCount++] = index;
                        if (MatchesTT(*entries[index].move))
                            ttEntry = index;
                    }
                }
                else
                    delete candidate;
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
                if (entries[index].returned || IsKiller(*entries[index].move))
                    continue;
                Score(index);
                const int currentPiece = board.mainBoard[entries[index].move->beginPlace];
                entries[index].key = QuietOrderingScore(
                    turn, depthGone, currentPiece, *entries[index].move);
                if (entries[index].key >=
                    SearchParameters::MoveOrdering::StrongQuietDepthCoefficient * depth)
                    strongQuiets[strongQuietCount++] = index;
                else
                    remainingQuiets[remainingQuietCount++] = index;
            }
        }

        Board& board;
        int depth;
        int depthGone;
        int turn;
        const Move& previousMove;
        uint16_t packedTTMove;
        uint16_t packedCounterMove;
        std::array<Entry, 256> entries{};
        std::array<int, 256> goodTactical{};
        std::array<int, 256> badTactical{};
        std::array<int, 256> quietEntries{};
        std::array<int, 256> strongQuiets{};
        std::array<int, 256> remainingQuiets{};
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
            static_cast<int>(board.pieces[turn * 8 + 2].size() +
                             board.pieces[turn * 8 + 3].size());
        const bool hasNonPawn = minorPieceCount > 0 ||
            board.pieces[turn * 8 + 4].size() > 0 ||
            board.pieces[turn * 8 + 5].size() > 0;
        return hasNonPawn;
    }

    bool IsAdvancedPassedPawnPush(const Board& board, const Move& move, int side)
    {
        if (board.mainBoard[move.beginPlace] % 8 != 1 || move.endPiece != 0)
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
                    board.mainBoard[row * 8 + targetFile] == enemyPawn)
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
            nonPawnMaterial += static_cast<int>(board.pieces[sideOffset + 2].size()) * Option::KnightValue;
            nonPawnMaterial += static_cast<int>(board.pieces[sideOffset + 3].size()) * Option::BishopValue;
            nonPawnMaterial += static_cast<int>(board.pieces[sideOffset + 4].size()) * Option::RookValue;
            nonPawnMaterial += static_cast<int>(board.pieces[sideOffset + 5].size()) * Option::QueenValue;
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
        if (board.pieces[enemyKingIndex].count == 0)
            return false;
        const int kingSquare = board.pieces[enemyKingIndex].front();
        const long long kingBit = Option::PowerTwo[kingSquare];
        long long occupancy = (board.whitePieces | board.blackPieces) &
            ~Option::PowerTwo[move.beginPlace];
        occupancy |= Option::PowerTwo[move.endPlace];
        if ((move.PublicFlag & Option::PowerTwo[6]) != 0)
            occupancy &= ~Option::PowerTwo[move.endPlace + (movingWhite ? -8 : 8)];
        const int piece = move.promotionPiece > 0
            ? move.promotionPiece % 8 : board.mainBoard[move.beginPlace] % 8;
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
        const int phase = std::clamp(EvaluationLogic::CalculatePhase(board), 0, 24);
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
    const int movingPiece = std::clamp(board.mainBoard[move.beginPlace], 0, 14);
    const int capturedPiece = std::clamp(move.endPiece % 8, 0, 6);
    return SearchParameters::MoveOrdering::CaptureVictimMultiplier * value[capturedPiece] +
        captureHistory[movingPiece][move.endPlace][capturedPiece];
}

int PVSSearch::QQuietEvasionOrderingScore(int side, int ply,
                                           const Board& board, const Move& move)
{
    const int movingPiece = std::clamp(board.mainBoard[move.beginPlace], 1, 14);
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
    int knights = board.pieces[turn * 8 + 2].size();
    int bishops = board.pieces[turn * 8 + 3].size();
    int rooks = board.pieces[turn * 8 + 4].size();
    int queens = board.pieces[turn * 8 + 5].size();
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
struct TargetResult { int value=0; std::string pv; };

void deleteMoveList(MoveList list)
{
    PVSSearch::deleteMoveList(list);
}

void UpdateCounterMove(const Board& board, const Move& move)
{
    StoreCounterMove(board, 0, move);
}

int CaptureOrder(const Board& b, const Move& m)
{
    static constexpr int v[7]={0,100,350,350,550,975,2500};
    const int mover=b.mainBoard[m.beginPlace];
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
            Move* move = moves.moves[i];
            if (move->endPiece == 0 && move->promotionPiece <= 0)
                continue;
            if (!MoveLogic::SEE_GE(board, *move, seeThreshold))
                continue;
            entries[count++] = {move, CaptureOrder(board, *move), false};
            if (MatchesPackedMove(*move, ttMove))
                ttIndex = count - 1;
        }
    }

    ~ProbCutPicker() { deleteMoveList(moves); }

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
    MoveList moves{};
    uint16_t ttMove;
    int seeThreshold;
    std::array<Entry, 256> entries{};
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
    return TTMoveHelper::UnpackPromotion(packed) != 0 || board.mainBoard[to] != 0 ||
        (board.mainBoard[from] % 8 == 1 && to == board.unpassentPlace);
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
        return {q->value,q->printString};
    }
    ++Search::searchNodeCount;
    if (Search::strictNodeLimit || (Search::searchNodeCount & 2047) == 0)
        Search::CheckLimits();
    if (Search::stopRequested.load(std::memory_order_relaxed)) return {0,{}};
    alpha=std::max(alpha,MateScore::MatedAtPly(ply));
    beta=std::min(beta,MateScore::MateAtPly(ply+1));
    if(alpha>=beta) return {alpha,{}};

    const int side=b.sideToMove?1:0;
    const bool inCheck=BoardLogic::UnderAttack(b,b.pieces[side*8+6].front(),!b.sideToMove);
    if (ply >= SearchStackSize - 1)
        return {inCheck ? 0 : EvaluationLogic::Evaluate(b), {}};
    SearchStackFrame& ss=StackFrame(ply);
    ss.ply=ply; ss.currentMove=prev;
    ss.hasCurrentMove=prev.beginPlace>=0&&prev.endPlace>=0&&prev.promotionPiece!=-1;
    if (ss.currentMovedPiece == 0 && ss.hasCurrentMove)
        ss.currentMovedPiece = std::clamp(b.mainBoard[prev.endPlace],0,14);
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
            const bool quiet=b.mainBoard[tm.endPlace]==0&&tm.promotionPiece<=0;
            const int movingPiece=std::clamp(b.mainBoard[tm.beginPlace],0,14);
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
            rawStaticEval=EvaluationLogic::Evaluate(b)-
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
            const int moved=b.mainBoard[c->beginPlace];
            MissingInfoAboutPrevStateFromMove u(b,*c);GameLogic::DoMove(b,*c,prev,depth,ply,&u);
            if(BoardLogic::UnderAttack(b,b.pieces[side*8+6].front(),b.sideToMove)){GameLogic::UndoMove(b,*c,u);continue;}++tried;
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
    std::vector<Move*>quietTried,captureTried; bool pruneQuiets=false;
    while(Move* move=picker.Next(pruneQuiets)){
        if(TTMoveHelper::PackMove(*move)==ss.excludedMove)continue;
        ++ss.moveCount; const bool quiet=IsQuietMove(*move); const bool tactical=!quiet;
        pruneQuiets=ply>0&&NullMoveMaterialEligible(b,side)&&
            best>MateScore::MatedAtPly(SearchStackSize-1)&&
            ss.moveCount>=FutilityMoveCount(improving,depth);
        const int movingPiece=std::clamp(b.mainBoard[move->beginPlace],0,14);
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
        if(BoardLogic::UnderAttack(b,b.pieces[side*8+6].front(),b.sideToMove)){GameLogic::UndoMove(b,*move,u);--ss.moveCount;continue;}
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
        GameLogic::UndoMove(b,*move,u); if(quiet)quietTried.push_back(move);else captureTried.push_back(move);
        if(value>best){best=value;bestMove=move;bestPacked=TTMoveHelper::PackMove(*move);bestPv=ChessStringManipulation::PVToString(*move,0,false,b)+(child.pv.empty()?"":" "+child.pv);
            if(value>alpha){alpha=value;if(value>=beta)break;}}
    }
    if(legal==0)best=ss.excludedMove?alpha:(inCheck?MateScore::MatedAtPly(ply):0);
    else if(bestMove)RewardCutoffMove(b,side,ply,depth,best,beta,*bestMove,quietTried,captureTried);
    else if((depth>=3||pv)&&ss.hasCurrentMove&&IsQuietMove(prev))
        UpdateContinuationHistoriesByBonus(
            ply-1,ss.currentMovedPiece,prev,StatBonus(depth));
    best=std::min(best,tablebaseMaxValue);
    const uint8_t flag=best>=beta?TT_LOWER_BOUND:(best<=oldAlpha?TT_UPPER_BOUND:TT_EXACT);
    if(ss.excludedMove==0&&!Search::stopRequested.load(std::memory_order_relaxed))
        TranspositionTable::Store(key,MateScore::ToTranspositionTable(best,ply),depth,flag,bestPacked,rawStaticEval,pv||ttPv);
    return {best,bestPv};
}
}

MovePrintValue *PVSSearch::PVS(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, bool selectiveSearch, bool cutNode)
{
    (void)MAtESearch;(void)previousMoveWasCheck;(void)nullWindowSearch;(void)selectiveSearch;
    TargetResult r=TargetSearch(isPVNode,alpha,beta,depth,prevMove,move1,move2,move3,board4,isNullMoveAllowed,depthGone,cutNode);
    auto*out=new MovePrintValue();out->value=r.value;out->printString=r.pv;out->bound=r.value>=beta?SearchBound::Lower:(r.value<=alpha?SearchBound::Upper:SearchBound::Exact);out->proof=ExactProof;out->selective=false;return out;
}

MovePrintValue *PVSSearch::SearchNode(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, bool selectiveSearch, bool cutNode)
{
    if (Search::stopRequested.load(std::memory_order_relaxed))
    {
        MovePrintValue *abortRet = new MovePrintValue();
        abortRet->value = 0;
        abortRet->bound = SearchBound::Upper;
        abortRet->MarkSpeculative(SearchProvenance::Aborted);
        return abortRet;
    }


    SearchStackFrame& ss = StackFrame(depthGone);
    ss.ply = depthGone;
    ss.currentMove = prevMove;
    ss.hasCurrentMove = prevMove.beginPlace >= 0 && prevMove.beginPlace < 64 &&
        prevMove.endPlace >= 0 && prevMove.endPlace < 64 && prevMove.promotionPiece != -1;
    const int inheritedStaticEval = ss.staticEval;
    ss.staticEval = (prevMove.promotionPiece == -1) ? inheritedStaticEval : NoStaticEval;
    ss.moveCount = 0;
    ss.pvNode = isPVNode;

    Move *SelectedMove = nullptr;
    Board *boardCopy = nullptr;

    MovePrintValue *retValue = new MovePrintValue();
    retValue->printString = "";
    if (depthGone >= 0 && depthGone < MaxKillerPly)
    {
        ss.killer1 = &killers[depthGone][0];
        ss.killer2 = &killers[depthGone][1];
    }

    MovePrintValue *MPValue = new MovePrintValue();
    MPValue->printString = "";

    int turn;
    if (!board4.sideToMove)
    {
        turn = 0;
    }
    else
    {
        turn = 1;
    }

    const int origAlpha = alpha;
    const int origBeta = beta;
    if (depth == 0)
    {
        delete retValue;
        retValue = nullptr;
        delete MPValue;
        MPValue = nullptr;
        if (MAtESearch)
        {
            MovePrintValue *mateHorizonResult = new MovePrintValue();
            mateHorizonResult->printString = "";
            const auto setTargetFailure = [&]()
            {
                mateHorizonResult->MarkSpeculative(SearchProvenance::ReducedSearch);
                if (beta < 0)
                {
                    mateHorizonResult->value = beta;
                    mateHorizonResult->bound = SearchBound::Lower;
                }
                else
                {
                    mateHorizonResult->value = alpha;
                    mateHorizonResult->bound = SearchBound::Upper;
                }
            };
            const bool inCheck = BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove);
            MoveList horizonMoves = MoveLogic::MoveGenerator(board4, 0, depthGone);
            int availMoves = 0;
            for (int i = 0; i < horizonMoves.count; ++i)
            {
                Move *m = horizonMoves.moves[i];
                MissingInfoAboutPrevStateFromMove undo(board4, *m);
                GameLogic::DoMove(board4, *m, prevMove, depthGone, depthGone, &undo);
                if (!BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
                {
                    availMoves++;
                    GameLogic::UndoMove(board4, *m, undo);
                    break;
                }
                GameLogic::UndoMove(board4, *m, undo);
            }
            deleteMoveList(horizonMoves);
            if (availMoves == 0)
            {
                if (inCheck)
                {
                    mateHorizonResult->value = MateScore::MatedAtPly(depthGone);
                    return mateHorizonResult;
                }
                else
                {
                    setTargetFailure();
                    return mateHorizonResult;
                }
            }
            setTargetFailure();
            return mateHorizonResult;
        }
        MovePrintValue *qResult = StartQSearch(isPVNode, alpha, beta, prevMove, depthGone, move1, move2, move3, board4, nullWindowSearch, previousMoveWasCheck);
        return qResult;
    }

    if (!MAtESearch && depthGone > 0)
    {
        const int lowerMateBound = MateScore::MatedAtPly(depthGone);
        if (alpha < lowerMateBound)
            alpha = lowerMateBound;
        if (alpha >= beta)
        {
            retValue->value = alpha;
            retValue->bound = SearchBound::Lower;
            retValue->proof = LowerProof;
            delete MPValue;
            return retValue;
        }

        const int upperMateBound = MateScore::MateAtPly(depthGone + 1);
        if (beta > upperMateBound)
            beta = upperMateBound;
        if (alpha >= beta)
        {
            retValue->value = beta;
            retValue->bound = SearchBound::Upper;
            retValue->proof = UpperProof;
            delete MPValue;
            return retValue;
        }
    }

    Search::searchNodeCount++;
    if ((Search::searchNodeCount & 2047) == 0 ||
        (Search::maxNodes > 0 && Search::searchNodeCount >= Search::maxNodes))
    {
        Search::CheckLimits();
    }
    if (Search::stopRequested.load(std::memory_order_relaxed))
    {
        delete MPValue;
        retValue->value = 0;
        retValue->bound = SearchBound::Upper;
        retValue->MarkSpeculative(SearchProvenance::Aborted);
        return retValue;
    }

    bool isNullWindow = (beta - alpha <= 1);

    if (BoardLogic::UnderAttack(board4, board4.pieces[(1 - turn) * 8 + 6].front(), board4.sideToMove))
    {
        retValue->value = 160000;
        retValue->MarkSpeculative(SearchProvenance::InvalidMove);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    }

    if (!MAtESearch && depthGone > 0)
    {
        const std::optional<Tablebase::Wdl> wdl =
            Tablebase::ProbeWdl(board4, Option::SyzygyProbeLimit);
        if (wdl.has_value())
        {
            Search::tablebaseHits.fetch_add(1, std::memory_order_relaxed);
            retValue->value = Tablebase::Score(*wdl);
            retValue->bound = SearchBound::Exact;
            delete MPValue;
            return retValue;
        }
    }
    if (depth == 1 && BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
    {
        previousMoveWasCheck = true;
    }
    const bool nodeInCheck = BoardLogic::UnderAttack(
        board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove);
    const uint64_t ttKey = SearchKey(board4.ZobristHashCode, ss.excludedMove);
    TTEntry ttEntry{};
    bool ttHit = TranspositionTable::Probe(ttKey, ttEntry);
    ttHitAverage = static_cast<int>(
        int64_t(TtHitAverageWindow - 1) * ttHitAverage / TtHitAverageWindow) +
        1024 * int(ttHit);
    if (ttHit)
    {
        ttEntry.score = MateScore::FromTranspositionTable(ttEntry.score, depthGone);
    }
    const bool ttPv = isPVNode || (ttHit && (ttEntry.metadata & TT_META_PV) != 0);
#if HOWL_CORRECTNESS_TESTING
    TTTelemetryStats &ttStats = TranspositionTable::TelemetryStats();
    ttStats.eligibleProbes++;
    TTTelemetryBucket *depthBucket = nullptr;
    if (depth <= 2) depthBucket = &ttStats.depth1To2;
    else if (depth <= 5) depthBucket = &ttStats.depth3To5;
    else if (depth <= 8) depthBucket = &ttStats.depth6To8;
    else depthBucket = &ttStats.depth9Plus;

    if (depthBucket) depthBucket->probes++;

    if (ttHit)
    {
        ttStats.hits++;
        if (depthBucket) depthBucket->hits++;

        if (ttEntry.depth >= depth) ttStats.hitsSufficientDepth++;
        else ttStats.hitsInsufficientDepth++;

        uint8_t baseFlag = TTBaseFlag(ttEntry.flag);
        if (baseFlag == TT_EXACT) ttStats.hitsExact++;
        else if (baseFlag == TT_LOWER_BOUND) ttStats.hitsLower++;
        else if (baseFlag == TT_UPPER_BOUND) ttStats.hitsUpper++;
    }
    else
    {
        ttStats.misses++;
    }
#endif
    if (TranspositionTable::CutoffsEnabled() && !MAtESearch && !isPVNode &&
        ttHit && ttEntry.depth >= depth && TTFlagIsRigorous(ttEntry.flag))
    {
        const uint8_t ttBase = TTBaseFlag(ttEntry.flag);
        const bool cutoff = (ttEntry.score >= beta &&
                             (ttBase == TT_LOWER_BOUND || ttBase == TT_EXACT)) ||
                            (ttEntry.score < beta &&
                             (ttBase == TT_UPPER_BOUND || ttBase == TT_EXACT));
        if (cutoff && ttEntry.bestMove != 0)
        {
            Move ttMove = UnpackMove(ttEntry.bestMove);
            const bool enPassant = board4.mainBoard[ttMove.beginPlace] % 8 == 1 &&
                ttMove.endPlace == board4.unpassentPlace;
            if (board4.mainBoard[ttMove.endPlace] == 0 &&
                ttMove.promotionPiece == 0 && !enPassant)
            {
                const bool success = ttEntry.score >= beta;
                UpdateHistory(turn, ttMove, depth, success);
                UpdateContinuationHistories(depthGone,
                    board4.mainBoard[ttMove.beginPlace], ttMove, depth, success);
            }
        }
        if (ttBase == TT_EXACT && cutoff)
        {
            TranspositionTable::RecordCutoff();
#if HOWL_CORRECTNESS_TESTING
            ttStats.cutoffsExact++;
            ttStats.totalCutoffs++;
            if (depthBucket) depthBucket->cutoffs++;
#endif
            retValue->value = ttEntry.score;
            retValue->selective = (ttEntry.flag & TT_SELECTIVE_FRONTIER) != 0;
            if (retValue->selective) retValue->AddProvenance(SearchProvenance::Quiescence);
            retValue->bound = SearchBound::Exact;
            retValue->proof = ExactProof;
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else if (ttBase == TT_LOWER_BOUND && ttEntry.score >= beta)
        {
            TranspositionTable::RecordCutoff();
#if HOWL_CORRECTNESS_TESTING
            ttStats.cutoffsLower++;
            ttStats.totalCutoffs++;
            if (depthBucket) depthBucket->cutoffs++;
#endif
            retValue->value = ttEntry.score;
            retValue->selective = (ttEntry.flag & TT_SELECTIVE_FRONTIER) != 0;
            if (retValue->selective) retValue->AddProvenance(SearchProvenance::Quiescence);
            retValue->bound = SearchBound::Lower;
            retValue->proof = LowerProof;
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else if (ttBase == TT_UPPER_BOUND && ttEntry.score <= alpha)
        {
            TranspositionTable::RecordCutoff();
#if HOWL_CORRECTNESS_TESTING
            ttStats.cutoffsUpper++;
            ttStats.totalCutoffs++;
            if (depthBucket) depthBucket->cutoffs++;
#endif
            retValue->value = ttEntry.score;
            retValue->selective = (ttEntry.flag & TT_SELECTIVE_FRONTIER) != 0;
            if (retValue->selective) retValue->AddProvenance(SearchProvenance::Quiescence);
            retValue->bound = SearchBound::Upper;
            retValue->proof = UpperProof;
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
    }
    if (!nodeInCheck)
    {
        if (ttHit && ttEntry.staticEval != TT_NO_STATIC_EVAL)
            ss.staticEval = ttEntry.staticEval;
        else if (prevMove.promotionPiece == -1 && StackFrame(depthGone - 1).staticEval != NoStaticEval)
            ss.staticEval = -StackFrame(depthGone - 1).staticEval;
        else
        {
            ss.staticEval = EvaluationLogic::Evaluate(board4);
            if (prevMove.promotionPiece != -1)
                ss.staticEval -= ss.statScore / 512;
        }
        if (ttHit && TTFlagIsRigorous(ttEntry.flag))
        {
            const uint8_t ttBase = TTBaseFlag(ttEntry.flag);
            if (ttBase == TT_LOWER_BOUND && ttEntry.score > ss.staticEval)
                ss.staticEval = ttEntry.score;
            else if (ttBase == TT_UPPER_BOUND && ttEntry.score < ss.staticEval)
                ss.staticEval = ttEntry.score;
        }
    }

    const bool improving = depthGone < 2 || StackFrame(depthGone - 2).staticEval == NoStaticEval ||
        ss.staticEval > StackFrame(depthGone - 2).staticEval;
    const bool staticPruningContext = !isPVNode && !nodeInCheck &&
        !MAtESearch && depth <= StaticPruningMaxDepth &&
        alpha > -MateScore::Threshold && beta < MateScore::Threshold;
    if (staticPruningContext)
    {
        const int staticValue = ss.staticEval;

        if (depth < RazoringMaxDepth)
        {
            if (staticValue <= alpha - RazorMargin)
            {
                MovePrintValue* razorResult = StartQSearch(
                    false, alpha, beta, prevMove, depthGone, move1, move2,
                    move3, board4, nullWindowSearch, previousMoveWasCheck);
                if (razorResult->ProvesUpper(alpha))
                {
                    delete retValue;
                    delete MPValue;
                    return razorResult;
                }
                delete razorResult;
            }
        }

        if (depth < SearchParameters::ReverseFutility::MaxDepth &&
            staticValue - FutilityMargin(depth, improving) >= beta &&
            staticValue < MateScore::Threshold)
        {
            retValue->value = staticValue;
            retValue->bound = SearchBound::Lower;
            retValue->MarkSpeculative(SearchProvenance::ForwardPruning);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
    }
    const bool nullSuppressed = nullSuppressedSide == turn &&
        depthGone <= nullSuppressedUntilPly;
    if (isNullMoveAllowed && prevMove.promotionPiece != -1 && !isPVNode &&
        !nodeInCheck && !MAtESearch && depth >= 1 &&
        alpha > -159800 && beta < 159800 && !nullSuppressed &&
        ss.statScore < SearchParameters::NullMove::StatScoreLimit && ss.excludedMove == 0)
    {
        const int nonPawnMaterialCount =
            static_cast<int>(board4.pieces[turn * 8 + 2].size() +
                             board4.pieces[turn * 8 + 3].size() +
                             board4.pieces[turn * 8 + 4].size() +
                             board4.pieces[turn * 8 + 5].size());
        int totalPieceCount = 0;
        int totalNonPawnMaterialCount = 0;
        for (int sideOffset : {0, 8})
        {
            for (int piece = 1; piece <= 6; ++piece)
                totalPieceCount += static_cast<int>(board4.pieces[sideOffset + piece].size());
            for (int piece = 2; piece <= 5; ++piece)
                totalNonPawnMaterialCount += static_cast<int>(board4.pieces[sideOffset + piece].size());
        }
        if (NullMoveMaterialEligible(board4, turn))
        {
            int staticEval = ss.staticEval;
            if (staticEval >= beta &&
                staticEval >= beta - SearchParameters::NullMove::StaticEvalDepthSlope * depth +
                    SearchParameters::NullMove::StaticEvalBase -
                    int(improving) * SearchParameters::NullMove::ImprovingMargin)
            {
                int R = (SearchParameters::NullMove::Base +
                         SearchParameters::NullMove::DepthSlope * depth) /
                        SearchParameters::NullMove::Divisor +
                    std::min((staticEval - beta) /
                                 SearchParameters::NullMove::EvalDivisor,
                             SearchParameters::NullMove::MaxEvalBonus);
                R = std::min(R, depth - 1);
                Move nullMove{};
                nullMove.promotionPiece = -1;
                MissingInfoAboutPrevStateFromMove undoInfo(board4);
                GameLogic::DoMove(board4, nullMove, prevMove, depthGone, depthGone);

                PrepareChildStack(depthGone + 1, nullMove, 0, 0);
                StackFrame(depthGone + 1).staticEval =
                    -ss.staticEval + 2 * TempoForPosition(board4);
                std::unique_ptr<MovePrintValue> nullRes(PVS(false, -beta, -beta + 1, depth - 1 - R, nullMove, move2, move3, prevMove, board4, MAtESearch, false, depthGone + 1, previousMoveWasCheck, true, selectiveSearch, !cutNode));
                int nullScore = -nullRes->value;

                GameLogic::UndoMove(board4, nullMove, undoInfo);

                const bool nullFailedHigh = nullScore >= beta;
                bool verificationPerformed = false;
                bool cutoffAccepted = nullFailedHigh;
                const bool verificationRequired = nullFailedHigh && depth >= 13 &&
                    std::abs(beta) < MateScore::Threshold;
                if (verificationRequired)
                {
                    verificationPerformed = true;
                    const int verificationDepth = std::max(0, depth - R);
                    const int oldSuppressedSide = nullSuppressedSide;
                    const int oldSuppressedUntil = nullSuppressedUntilPly;
                    const SearchStackFrame savedVerificationFrame = ss;
                    nullSuppressedSide = turn;
                    nullSuppressedUntilPly = depthGone + 3 * (depth - R) / 4;
                    std::unique_ptr<MovePrintValue> verification(PVS(
                        false, beta - 1, beta, verificationDepth, prevMove,
                        move1, move2, move3, board4, MAtESearch, false,
                        depthGone, previousMoveWasCheck, true,
                        selectiveSearch, false));
                    cutoffAccepted = verification->value >= beta;
                    ss = savedVerificationFrame;
                    nullSuppressedSide = oldSuppressedSide;
                    nullSuppressedUntilPly = oldSuppressedUntil;
                }

#if HOWL_CORRECTNESS_TESTING
                RecordNullMoveAttempt(depth, R, staticEval - beta, board4, turn,
                                      nullFailedHigh,
                                      verificationPerformed && cutoffAccepted);
#endif

                if (cutoffAccepted)
                {
                    retValue->value = IsMateScore(nullScore) ? beta : nullScore;
                    retValue->bound = SearchBound::Lower;
                    retValue->MarkSpeculative(SearchProvenance::NullMove);
                    retValue->printString = "null";
                    delete MPValue;
                    MPValue = nullptr;
                    return retValue;
                }
            }
        }
        if (Search::stopRequested.load(std::memory_order_relaxed))
        {
            delete MPValue;
            retValue->value = 0;
            retValue->bound = SearchBound::Upper;
            retValue->MarkSpeculative(SearchProvenance::Aborted);
            return retValue;
        }
    }

    if (!MAtESearch && ss.excludedMove == 0 && depth >= 7 &&
        (!ttHit || ttEntry.bestMove == 0))
    {
        const SearchStackFrame savedFrame = ss;
        std::unique_ptr<MovePrintValue> iidResult(PVS(
            isPVNode, alpha, beta, depth - 7, prevMove, move1, move2, move3,
            board4, MAtESearch, isNullMoveAllowed, depthGone,
            previousMoveWasCheck, nullWindowSearch, selectiveSearch, cutNode));
        ss = savedFrame;
        TTEntry iidEntry{};
        if (TranspositionTable::Probe(ttKey, iidEntry) && iidEntry.bestMove != 0)
        {
            ttEntry = iidEntry;
            ttEntry.score = MateScore::FromTranspositionTable(ttEntry.score, depthGone);
            ttHit = true;
        }
    }
#if HOWL_CORRECTNESS_TESTING
    {
        MoveList shadowMoveList = MoveLogic::MoveGenerator(board4, depth, depthGone);
        TranspositionTable::CheckShadowEntryOnProbe(board4.ZobristHashCode, depth, alpha, beta, isPVNode, shadowMoveList, false);
        deleteMoveList(shadowMoveList);
    }
#endif
    MovePicker movePicker(board4, depth, depthGone, turn, prevMove,
                          ttHit ? ttEntry.bestMove : 0);
#if HOWL_CORRECTNESS_TESTING
    const bool hasTTMove = movePicker.HasTTMove();
    if (ttHit && ttEntry.bestMove != 0)
    {
        ttStats.ttMoveFoundNoCutoff++;
        if (hasTTMove) ttStats.ttMoveMatchedLegal++;
        else ttStats.ttMoveMissingFromGenerated++;
        if (hasTTMove) TranspositionTable::RecordHitStats(true, true);
    }
#endif
    int inCheck = -1;
    int staticEval = -200000;
    int bestMoveValue = -200000;
    bool bestMoveSelective = false;
    bool bestLowerProof = false;
    bool allUpperProof = true;
    std::string SelectedPV = "";
    int availMoves = 0;
    int quietMovesSearched = 0;
    int selectedMoveRank = 0;
    std::vector<Move*> searchedQuiets;
    std::vector<Move*> searchedCaptures;
    {
        bool firstMove = true;
        if (!isPVNode && !nodeInCheck && !MAtESearch && depth >= ProbCutMinDepth &&
            beta < 159500 && alpha > -159500)
        {
            const int probBeta = std::min(200000,
                beta + SearchParameters::ProbCut::BetaMargin -
                SearchParameters::ProbCut::ImprovingMargin * int(improving));
            int forcingMovesTried = 0;
            MovePicker probPicker(board4, depth, depthGone, turn, prevMove,
                                  ttHit ? ttEntry.bestMove : 0);
            for (int i = 0; i < ProbCutPickerLimit &&
                         forcingMovesTried < ProbCutMaxCandidatesBase + 2 * int(cutNode); ++i)
            {
                Move* nextProbMove = probPicker.Next();
                if (nextProbMove == nullptr)
                    break;
                Move &probMove = *nextProbMove;
                MissingInfoAboutPrevStateFromMove probUndo(board4, probMove);
                GameLogic::DoMove(board4, probMove, prevMove, depthGone, depthGone, &probUndo);
                if (BoardLogic::UnderAttack(
                        board4, board4.pieces[turn * 8 + 6].front(), board4.sideToMove))
                {
                    GameLogic::UndoMove(board4, probMove, probUndo);
                    continue;
                }
                const bool givesCheck = BoardLogic::UnderAttack(
                    board4, board4.pieces[board4.sideToMove * 8 + 6].front(),
                    !board4.sideToMove);
                const bool forcing = probMove.endPiece > 0 ||
                    probMove.promotionPiece > 0 || givesCheck;
                if (!forcing || RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                {
                    GameLogic::UndoMove(board4, probMove, probUndo);
                    continue;
                }
                forcingMovesTried++;

                std::unique_ptr<MovePrintValue> qResult(QSearcher::QSearch(
                    false, -probBeta, -probBeta + 1, probMove, depthGone + 1,
                    givesCheck ? 1 : 0, true, 0, move2, move3, prevMove,
                    board4, false, depthGone + 1, true));
                int probValue = -qResult->value;
                if (probValue >= probBeta && !IsMateScore(probValue))
                {
                    PrepareChildStack(depthGone + 1, probMove,
                        board4.mainBoard[probMove.beginPlace], 0);
                    std::unique_ptr<MovePrintValue> reducedResult(PVS(
                        false, -probBeta, -probBeta + 1, depth - 4, probMove,
                        move2, move3, prevMove, board4, false, true,
                        depthGone + 1, givesCheck, true, true, !cutNode));
                    probValue = -reducedResult->value;
                    if (probValue >= probBeta && !IsMateScore(probValue))
                    {
                        GameLogic::UndoMove(board4, probMove, probUndo);
                        retValue->value = beta;
                        retValue->bound = SearchBound::Lower;
                        retValue->MarkSpeculative(SearchProvenance::ProbCut);
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                }
                GameLogic::UndoMove(board4, probMove, probUndo);
                if (Search::stopRequested.load(std::memory_order_relaxed))
                {
                    delete MPValue;
                    MPValue = nullptr;
                    retValue->value = 0;
                    retValue->bound = SearchBound::Upper;
                    retValue->MarkSpeculative(SearchProvenance::Aborted);
                    return retValue;
                }
            }
        }

        for (int i = 0; ; ++i)
        {
            Move *move = movePicker.Next();
            if (move == nullptr)
                break;
            if (ss.excludedMove != 0 && MatchesPackedMove(*move, ss.excludedMove))
                continue;
            if (Search::stopRequested.load(std::memory_order_relaxed))
            {
                delete MPValue;
                MPValue = nullptr;
                retValue->value = 0;
                retValue->bound = SearchBound::Upper;
                retValue->MarkSpeculative(SearchProvenance::Aborted);
                return retValue;
            }
            ss.moveCount = i + 1;
            const int alphaBeforeMove = alpha;
            int LMRDepth = 0;
            uint8_t moveProof = NoProof;
            const bool isTTMove = ttHit &&
                MatchesPackedMove(*move, ttEntry.bestMove);
            int moveExtension = 0;
            bool singularLMR = false;
            if (depth >= 6 && isTTMove && depthGone > 0 && ss.excludedMove == 0 &&
                TTFlagIsRigorous(ttEntry.flag) &&
                (TTBaseFlag(ttEntry.flag) == TT_LOWER_BOUND ||
                 TTBaseFlag(ttEntry.flag) == TT_EXACT) &&
                ttEntry.depth >= depth - 3 && !IsMateScore(ttEntry.score))
            {
                bool ttMoveLegal = false;
                MissingInfoAboutPrevStateFromMove singularLegalityUndo(board4, *move);
                GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone,
                                  &singularLegalityUndo);
                ttMoveLegal = !BoardLogic::UnderAttack(
                    board4, board4.pieces[turn * 8 + 6].front(), board4.sideToMove);
                GameLogic::UndoMove(board4, *move, singularLegalityUndo);
                if (ttMoveLegal)
                {
                    const int singularBeta = ttEntry.score - 2 * depth;
                    const SearchStackFrame savedFrame = ss;
                    ss.excludedMove = ttEntry.bestMove;
                    std::unique_ptr<MovePrintValue> singularResult(PVS(
                        false, singularBeta - 1, singularBeta, depth / 2,
                        prevMove, move1, move2, move3, board4, MAtESearch,
                        isNullMoveAllowed, depthGone, previousMoveWasCheck,
                        true, selectiveSearch, cutNode));
                    ss = savedFrame;
                    if (singularResult->value < singularBeta)
                    {
                        moveExtension = 1;
                        singularLMR = true;
                    }
                    else if (singularBeta >= beta)
                    {
                        retValue->value = singularBeta;
                        retValue->bound = SearchBound::Lower;
                        retValue->proof = LowerProof;
                        delete MPValue;
                        return retValue;
                    }
                }
            }
            const bool discoveredCheck = move->givesCheck &&
                !MovedPieceGivesCheck(board4, *move);
            if (move->givesCheck && (discoveredCheck || move->value >= 0))
            {
                moveExtension = 1;
            }
            if (IsKillerMove(depthGone, *move) &&
                IsAdvancedPassedPawnPush(board4, *move, turn))
            {
                moveExtension = 1;
            }
            if (LastCaptureExtension(board4, *move))
            {
                moveExtension = 1;
            }
            if (board4.mainBoard[move->beginPlace] % 8 == 6 &&
                std::abs(move->endPlace - move->beginPlace) == 2)
            {
                moveExtension = 1;
            }
            if (firstMove)
            {
                bool firstMoveWasRepetition = false;
                boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4, *move);
                GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone, missingInfoAboutPrevStateFromMove);
                if (BoardLogic::UnderAttack(
                        board4, board4.pieces[turn * 8 + 6].front(), board4.sideToMove))
                {
                    GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                    delete missingInfoAboutPrevStateFromMove;
                    missingInfoAboutPrevStateFromMove = nullptr;
                    if (UCI::IsTest())
                    {
                        Board::AreBoardsEqual(board4, *boardCopy);
                        delete boardCopy;
                        boardCopy = nullptr;
                    }
                    continue;
                }
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                {
                    firstMoveWasRepetition = true;
                    availMoves++;
                    allUpperProof = false;
                    retValue->AddProvenance(SearchProvenance::Repetition);
                    bestMoveValue = 0;
                    bestMoveSelective = false;
                    SelectedMove = move;
                    selectedMoveRank = i + 1;
                    move->value = 0;
                }
                else
                {
                    if (IsQuietMove(*move)) searchedQuiets.push_back(move);
                    else searchedCaptures.push_back(move);
                    bool tempPVNode = false;
                    if (isPVNode || move->isRefuteWithoutNullMove)
                    {
                        tempPVNode = true;
                    }
                    delete MPValue;
                    PrepareChildStack(depthGone + 1, *move,
                        board4.mainBoard[move->beginPlace], IsQuietMove(*move)
                            ? QuietStatScore(turn, depthGone,
                                board4.mainBoard[move->beginPlace], *move) : 0);
                    MPValue = PVS(tempPVNode, -beta, -alpha,
                                  depth - 1 + moveExtension, *move, move2, move3,
                                  prevMove, board4, MAtESearch, true,
                                  depthGone + 1, previousMoveWasCheck, nullWindowSearch,
                                  selectiveSearch, false);
                    bestMoveValue = -MPValue->value;
                    moveProof = InvertProof(MPValue->proof);
                    bestLowerProof = (moveProof & LowerProof) != 0;
                    retValue->provenance |= MPValue->provenance;
                    if (bestMoveValue != -160000)
                        allUpperProof = allUpperProof && ((moveProof & UpperProof) != 0);
                    bestMoveSelective = MPValue->selective || selectiveSearch;
                    SelectedMove = move;
                    selectedMoveRank = i + 1;
                    SelectedPV = MPValue->printString;
                    if (bestMoveValue != -160000)
                    {
                        availMoves++;
                    }
                    if (selectiveSearch)
                        bestMoveSelective = true;
                    move->value = bestMoveValue;
                }
                GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                delete missingInfoAboutPrevStateFromMove;
                missingInfoAboutPrevStateFromMove = nullptr;
                if (UCI::IsTest())
                {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                if (Search::stopRequested.load(std::memory_order_relaxed))
                {
                    delete MPValue;
                    MPValue = nullptr;
                    retValue->value = 0;
                    retValue->bound = SearchBound::Upper;
                    retValue->MarkSpeculative(SearchProvenance::Aborted);
                    return retValue;
                }
                firstMove = false;
                if (bestMoveValue > alpha)
                {
                    if (bestMoveValue >= beta)
                    {
#if HOWL_CORRECTNESS_TESTING
                        RecordMoveOrderingCutoff(i, hasTTMove, *move, depth, depthGone, isPVNode, nullWindowSearch || beta == alpha + 1);
#endif
                        if (!selectiveSearch)
                        {
                            RewardCutoffMove(board4, turn, depthGone, depth, *move,
                                             searchedQuiets, searchedCaptures);
                            if (isPVNode)
                            {
                                move->isRefuteWithoutNullMove = true;
                            }
                            // TODO: remove rest of list from movelist for memory management
                            if (bestMoveValue != 0)
                            {
                                uint16_t packed = TTMoveHelper::PackMove(*move);
                                MovePrintValue cutoffResult;
                                cutoffResult.bound = ClassifyBound(bestMoveValue, origAlpha, origBeta);
                                cutoffResult.SetProof(bestLowerProof, false);
                                cutoffResult.selective = bestMoveSelective;
                                const uint8_t cutoffFlag = TTFlagForResult(cutoffResult);
                                TranspositionTable::Store(ttKey,
                                    MateScore::ToTranspositionTable(move->value, depthGone),
                                    depth, cutoffFlag, packed, ss.staticEval, ttPv);
                            }
                        }
                        retValue->value = move->value;
                        retValue->bound = ClassifyBound(
                            bestMoveValue, origAlpha, origBeta);
                        retValue->selective = bestMoveSelective;
                        retValue->SetProof(bestLowerProof, false);
                        retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + MPValue->printString;
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                    alpha = bestMoveValue;
                }
                move->isRefuteWithoutNullMove = false;
                // AddToTable(board4, move, depth, false, nullWindowSearch, depthGone);
            }
            else
            {
                bool tempRepeat = false;
                bool trustedValue = false;
                const int nominalChildDepth = depth - 1 + moveExtension;
                const int combinedHistory = IsQuietMove(*move)
                    ? QuietCombinedHistory(turn, depthGone,
                        board4.mainBoard[move->beginPlace], *move)
                    : 0;
                const bool ttPvEvidence = ttPv;
                LMRDepth = ContextualLMRReduction(
                    depth, i + 1, nominalChildDepth, turn, depthGone, *move,
                    isPVNode, cutNode, improving, ttPvEvidence, nodeInCheck,
                    moveExtension > 0, singularLMR);
                if (MAtESearch)
                    LMRDepth = 0;

                const int expectedReducedDepth = std::max(
                    1, nominalChildDepth - LMRDepth);
                const bool pruningContext = !isPVNode && !nodeInCheck &&
                    !MAtESearch && !isTTMove &&
                    alpha > -159800 && beta < 159800;
                const bool forcingMove = move->givesCheck ||
                    move->promotionPiece > 0 || move->isRefuteWithoutNullMove;
                const bool highConfidenceMove = forcingMove ||
                    IsKillerMove(depthGone, *move) || combinedHistory >= 4096 ||
                    (move->endPiece > 0 && move->value >= 0);
                const bool moveCountPruningCandidate = pruningContext &&
                    !highConfidenceMove && IsQuietMove(*move) &&
                    i + 1 >= FutilityMoveCount(improving, depth);

                bool valueFutilityCandidate = false;
                if (pruningContext && !highConfidenceMove &&
                    IsQuietMove(*move) &&
                    expectedReducedDepth < SearchParameters::ParentFutility::MaxReducedDepth)
                {
                    if (staticEval == -200000)
                        staticEval = ss.staticEval;
                    valueFutilityCandidate = staticEval +
                        SearchParameters::ParentFutility::BaseMargin +
                        SearchParameters::ParentFutility::DepthMargin * expectedReducedDepth <= alpha &&
                        combinedHistory < SearchParameters::History::ParentFutilityLimit;
                }
                const bool counterHistoryPruningCandidate = pruningContext &&
                    IsQuietMove(*move) &&
                    expectedReducedDepth < SearchParameters::CounterMovePruning::ReducedDepthBase +
                        (ss.statScore > 0 || ss.moveCount == 1) &&
                    ContinuationHistoryScoreAt(depthGone, 1,
                        board4.mainBoard[move->beginPlace], *move) <
                            SearchParameters::CounterMovePruning::ContinuationThreshold &&
                    ContinuationHistoryScoreAt(depthGone, 2,
                        board4.mainBoard[move->beginPlace], *move) <
                            SearchParameters::CounterMovePruning::ContinuationThreshold;
                const bool seePruningCandidate = pruningContext &&
                    !highConfidenceMove &&
                    ((IsQuietMove(*move) && move->value <
                        -(SearchParameters::SEE::QuietBase -
                          std::min(expectedReducedDepth, SearchParameters::SEE::QuietDepthCap)) *
                         expectedReducedDepth * expectedReducedDepth) ||
                     (!IsQuietMove(*move) && move->value <
                         -SearchParameters::SEE::TacticalDepthMargin * depth));

                boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4, *move);
                GameLogic::DoMove(board4, *move, prevMove, depth, depthGone, missingInfoAboutPrevStateFromMove);
                if (BoardLogic::UnderAttack(
                        board4, board4.pieces[turn * 8 + 6].front(), board4.sideToMove))
                {
                    GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                    delete missingInfoAboutPrevStateFromMove;
                    missingInfoAboutPrevStateFromMove = nullptr;
                    if (UCI::IsTest())
                    {
                        Board::AreBoardsEqual(board4, *boardCopy);
                        delete boardCopy;
                        boardCopy = nullptr;
                    }
                    continue;
                }
                int value;
                bool valueSelective = false;
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                {
                    tempRepeat = true;
                    availMoves++;
                    retValue->AddProvenance(SearchProvenance::Repetition);
                    value = 0;
                    move->value = 0;
                    trustedValue = true;
                }
                else
                {
                    const bool givesCheck = BoardLogic::UnderAttack(
                        board4, board4.pieces[board4.sideToMove * 8 + 6].front(),
                        !board4.sideToMove);
                    if (!givesCheck && (moveCountPruningCandidate ||
                                       valueFutilityCandidate ||
                                       seePruningCandidate ||
                                       counterHistoryPruningCandidate))
                    {
                        allUpperProof = false;
                        retValue->AddProvenance(SearchProvenance::ForwardPruning);
                        GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                        delete missingInfoAboutPrevStateFromMove;
                        missingInfoAboutPrevStateFromMove = nullptr;
                        if (UCI::IsTest())
                        {
                            Board::AreBoardsEqual(board4, *boardCopy);
                            delete boardCopy;
                            boardCopy = nullptr;
                        }
                        continue;
                    }


                    bool tempPVNode = false;
                    if (move->isRefuteWithoutNullMove || (isPVNode && availMoves < 2))
                    {
                        tempPVNode = true;
                    }
                    if (move->endPiece == 0 && move->promotionPiece <= 0)
                    {
                        quietMovesSearched++;
                        searchedQuiets.push_back(move);
                    }
                    else
                        searchedCaptures.push_back(move);

                    const bool reducedSearch = LMRDepth > 0;
                    delete MPValue;
                    PrepareChildStack(depthGone + 1, *move,
                        board4.mainBoard[move->beginPlace], IsQuietMove(*move)
                            ? QuietStatScore(turn, depthGone,
                                board4.mainBoard[move->beginPlace], *move) : 0);
                    MPValue = PVS(reducedSearch ? false : tempPVNode,
                                  -alpha - Option::nullWindowSize, -alpha,
                                  nominalChildDepth - LMRDepth, *move, move2,
                                  move3, prevMove, board4, MAtESearch, true,
                                  depthGone + 1, previousMoveWasCheck, true,
                                  selectiveSearch || reducedSearch,
                                  reducedSearch ? true : (tempPVNode ? false : !cutNode));
                    value = -MPValue->value;
                    moveProof = InvertProof(MPValue->proof);
                    if (value != -160000)
                    {
                        availMoves++;
                    }
                    valueSelective = MPValue->selective || reducedSearch ||
                        selectiveSearch;
                    if (!reducedSearch)
                    {
                        trustedValue = true;
                    }
#if HOWL_CORRECTNESS_TESTING
                    if (reducedSearch)
                    {
                        RecordLMRSearch(i, depth, depthGone, *move, LMRDepth, (value > alpha));
                    }
#endif

                    if (reducedSearch && value > alpha)
                    {
                        const int smallerReduction = LMRDepth / 2;
                        delete MPValue;
                        PrepareChildStack(depthGone + 1, *move,
                            board4.mainBoard[move->beginPlace], IsQuietMove(*move)
                                ? QuietStatScore(turn, depthGone,
                                    board4.mainBoard[move->beginPlace], *move) : 0);
                        MPValue = PVS(false, -alpha - Option::nullWindowSize, -alpha,
                                      nominalChildDepth - smallerReduction,
                                      *move, move2, move3,
                                      prevMove, board4, MAtESearch, true, depthGone + 1,
                                      previousMoveWasCheck, true, true, true);
                        value = -MPValue->value;
                        moveProof = InvertProof(MPValue->proof);
                        valueSelective = true;
                    }

                    // A certified unreduced scout already proves the same
                    // one-cp non-PV cutoff. PV/refutation and LMR recovery
                    // searches retain their distinct confirmation semantics.
                    const bool scoutProvesCutoff = !reducedSearch && !isPVNode &&
                        !tempPVNode && !move->isRefuteWithoutNullMove &&
                        beta - alpha == Option::nullWindowSize &&
                        MPValue->ProvesUpper(-beta);
                    if (value > alpha && !scoutProvesCutoff)
                    {
                        bool confirmationPVNode = isPVNode || move->isRefuteWithoutNullMove;
#if HOWL_CORRECTNESS_TESTING
                        if (reducedSearch)
                            g_lmrStats.fullDepthConfirmations++;
                        int origAlphaForLMR = alpha;
                        int origBetaForLMR = beta;
#endif
                        delete MPValue;
                        PrepareChildStack(depthGone + 1, *move,
                            board4.mainBoard[move->beginPlace], IsQuietMove(*move)
                                ? QuietStatScore(turn, depthGone,
                                    board4.mainBoard[move->beginPlace], *move) : 0);
                        MPValue = PVS(confirmationPVNode, -beta, -alpha,
                                      nominalChildDepth,
                                      *move, move2, move3, prevMove, board4, MAtESearch,
                                      true, depthGone + 1, previousMoveWasCheck,
                                      nullWindowSearch, selectiveSearch,
                                      confirmationPVNode ? false : !cutNode);
                        value = -MPValue->value;
                        moveProof = InvertProof(MPValue->proof);
                        valueSelective = MPValue->selective || selectiveSearch;
                        trustedValue = true;
#if HOWL_CORRECTNESS_TESTING
                        if (reducedSearch)
                        {
                            RecordLMRReSearchResult(i, value, origAlphaForLMR, origBetaForLMR);
                        }
#endif
                    }

                    retValue->provenance |= MPValue->provenance;
                    if (trustedValue)
                        move->value = value;
                }
                if (value != -160000)
                    allUpperProof = allUpperProof && ((moveProof & UpperProof) != 0);
                if (trustedValue && value > alpha)
                {
                    alpha = value;
                }
                GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                delete missingInfoAboutPrevStateFromMove;
                missingInfoAboutPrevStateFromMove = nullptr;
                if (UCI::IsTest())
                {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                if (Search::stopRequested.load(std::memory_order_relaxed))
                {
                    delete MPValue;
                    MPValue = nullptr;
                    retValue->value = 0;
                    retValue->bound = SearchBound::Upper;
                    retValue->MarkSpeculative(SearchProvenance::Aborted);
                    return retValue;
                }
                if (trustedValue && value > bestMoveValue)
                {
                    if (value >= beta)
                    {
#if HOWL_CORRECTNESS_TESTING
                        RecordMoveOrderingCutoff(i, false, *move, depth, depthGone, isPVNode, nullWindowSearch || beta == alpha + 1);
#endif
                        if (!selectiveSearch)
                        {
                            RewardCutoffMove(board4, turn, depthGone, depth, *move,
                                             searchedQuiets, searchedCaptures);
                            if (isPVNode)
                            {
                                move->isRefuteWithoutNullMove = true;
                            }
                            // TODO: delete movelist except first and second move
                            if (value != 0)
                            {
                                uint16_t packed = TTMoveHelper::PackMove(*move);
                                MovePrintValue cutoffResult;
                                cutoffResult.bound = ClassifyBound(value, origAlpha, origBeta);
                                cutoffResult.SetProof((moveProof & LowerProof) != 0, false);
                                cutoffResult.selective = valueSelective;
                                const uint8_t cutoffFlag = TTFlagForResult(cutoffResult);
                                TranspositionTable::Store(ttKey,
                                    MateScore::ToTranspositionTable(move->value, depthGone),
                                    depth, cutoffFlag, packed, ss.staticEval, ttPv);
                            }
                        }
                        retValue->value = move->value;
                        retValue->bound = ClassifyBound(
                            value, origAlpha, origBeta);
                        retValue->selective = valueSelective;
                        retValue->SetProof((moveProof & LowerProof) != 0, false);
                        retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + MPValue->printString;
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                    move->isRefuteWithoutNullMove = false;
                    bestMoveValue = value;
                    bestLowerProof = (moveProof & LowerProof) != 0;
                    bestMoveSelective = valueSelective;
                    SelectedMove = move;
                    selectedMoveRank = i + 1;
                    SelectedPV = MPValue->printString;
                }
            }
        }

        if (availMoves == 0 && ss.excludedMove != 0)
        {
            retValue->value = alpha;
            retValue->bound = SearchBound::Upper;
            retValue->proof = UpperProof;
            delete MPValue;
            return retValue;
        }
        if (availMoves == 0 && !BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
        {
            Move stalemateMove;
            stalemateMove.value = 0;
            stalemateMove.promotionPiece = -2;
            retValue->value = 0;
            retValue->SetProof(allUpperProof, allUpperProof);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else if (availMoves == 0 && BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
        {
            Move mateMove;
            mateMove.value = MateScore::MatedAtPly(depthGone);
            retValue->value = MateScore::MatedAtPly(depthGone);
            retValue->SetProof(allUpperProof, allUpperProof);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else
        {
#if HOWL_CORRECTNESS_TESTING
            if (SelectedMove != nullptr)
            {
                g_orderingQualityStats.recordBestMove(selectedMoveRank, isPVNode, IsQuietMove(*SelectedMove));
            }
#endif
            const bool resultSelective = bestMoveSelective || selectiveSearch;
            const SearchBound resultBound = ClassifyBound(
                bestMoveValue, origAlpha, origBeta);
            if (!selectiveSearch && SelectedMove != nullptr && bestMoveValue != 0)
            {
                if (IsQuietMove(*SelectedMove))
                {
                    UpdateHistory(turn, *SelectedMove, depth, true);
                    UpdateContinuationHistories(depthGone,
                        board4.mainBoard[SelectedMove->beginPlace], *SelectedMove,
                        depth, true);
                }
                else
                    UpdateCaptureHistory(board4, *SelectedMove, depth, true);
                MovePrintValue storedResult;
                storedResult.bound = resultBound;
                storedResult.SetProof(bestLowerProof, allUpperProof);
                storedResult.selective = resultSelective;
                const uint8_t flag = TTFlagForResult(storedResult);
                uint16_t packed = TTMoveHelper::PackMove(*SelectedMove);
                TranspositionTable::Store(ttKey,
                    MateScore::ToTranspositionTable(bestMoveValue, depthGone),
                    depth, flag, packed, ss.staticEval, ttPv);
            }
            retValue->value = bestMoveValue;
            retValue->bound = resultBound;
            retValue->selective = resultSelective;
            retValue->SetProof(bestLowerProof, allUpperProof);
            retValue->printString = ChessStringManipulation::PVToString(*SelectedMove, 0, false, board4) + ' ' + SelectedPV;
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
    }
}

void PVSSearch::deleteMoveList(MoveList moveList)
{
    for (int i = 0; i < moveList.count; ++i)
    {
        delete moveList.moves[i];
    }
}

void PVSSearch::NullMovePruning(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool mAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, MovePrintValue mPValue)
{
    /*if (depth == 1)
    {
        if (Evaluate(board4) + Option.futilityMargin <= alpha)
        {
            double valueRazored = qSearch(alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone);
            if (Evaluate(board4) + Option.futilityMargin > valueRazored)
            {
                return Evaluate(board4) + Option.futilityMargin;
            }
            else
            {
                return valueRazored;
            }
        }
    }
    if (depth == 2)
    {
        if (Evaluate(board4) + Option.extendedFutilityMargin <= alpha)
        {
            double valueRazored = qSearch(alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone);
            if (Evaluate(board4) + Option.extendedFutilityMargin > valueRazored)
            {
                return Evaluate(board4) + Option.extendedFutilityMargin;
            }
            else
            {
                return valueRazored;
            }
        }
    }
    if (depth == 3)
    {
        if (Evaluate(board4) + Option.superExtendedFutilityMargin <= alpha)
        {
            double valueRazored = qSearch(alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone);
            if (Evaluate(board4) + Option.superExtendedFutilityMargin > valueRazored)
            {
                return Evaluate(board4) + Option.superExtendedFutilityMargin;
            }
            else
            {
                return valueRazored;
            }
        }
    }
    int expectedValue = Evaluate(board4) + depth * 450;
    if (expectedValue <= alpha)
    {
        retValue.value = expectedValue;
        return retValue;
        //return expectedValue;
        //int valueRazored = qSearch(isPVNode, alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone, nullWindowSearch).value;
        //if (expectedValue > valueRazored)
        //{
        //    return expectedValue;
        //}
        //else
        //{
        //    return valueRazored;
        //}
    }*/
}

double PVSSearch::NullMoveReduction(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool mateSearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch)
{
    Move nullMove = Move();
    nullMove.promotionPiece = -1;

    Board *boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
    MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
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
    GameLogic::UndoMove(board4, nullMove, *missingInfoAboutPrevStateFromMove);
    if (UCI::IsTest())
    {
        Board::AreBoardsEqual(board4, *boardCopy);
        delete boardCopy;
        boardCopy = nullptr;
    }
    delete missingInfoAboutPrevStateFromMove;
    missingInfoAboutPrevStateFromMove = nullptr;
    return valueReduced;
}

MovePrintValue *PVSSearch::StartQSearch(bool isPVNode, int alpha, int beta, Move &prevMove, int depthGone, Move &move1, Move &move2, Move &move3, Board &board4, bool nullWindowSearch, bool previousMoveWasCheck)
{
    MovePrintValue *res = nullptr;
    if (previousMoveWasCheck)
    {
        res = QSearcher::QSearch(isPVNode, alpha, beta, prevMove, depthGone, 1, false, 1, move1, move2, move3, board4, false, depthGone, nullWindowSearch);
    }
    // else if (evaluate(recDepth) > beta && isNullMoveAllowed)
    //{
    //     return evaluate(recDepth);
    // }
    else if (prevMove.endPiece != 0 || prevMove.promotionPiece > 0)
    {
        res = QSearcher::QSearch(isPVNode, alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone, nullWindowSearch);
    }
    else
    {
        res = QSearcher::QSearch(isPVNode, alpha, beta, prevMove, depthGone, 0, false, 0, move1, move2, move3, board4, false, depthGone, nullWindowSearch);
    }
    return res;
}

void PVSSearch::IGG(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool lastCheck, bool nullWindowSearch, MoveList moveList)
{
    Board* boardCopy = nullptr; // <-- ADD THIS LINE
    MovePrintValue *MPValue = new MovePrintValue();
    int value = -200000;
    int tempDepth = depth;
    int bestMoveValue = -200000;
    depth = moveOrderingDepth[depth];
    {
        if (depth >= 3)
        {
            int availMovesIr = 0;
            bool firstMoveIr = true;
            {
                firstMoveIr = true;
                for (int i = 0; i < moveList.count; ++i)
                {
                    Move *move = moveList.moves[i];
                    if (firstMoveIr)
                    {
                        boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                        MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4, *move);
                        GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone, missingInfoAboutPrevStateFromMove);
                        if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                        {
                            bestMoveValue = 0;
                            move->value = 0;
                        }
                        else
                        {
                            bool tempPVNode = false;
                            if (isPVNode || move->isRefuteWithoutNullMove)
                            {
                                tempPVNode = true;
                            }
                            delete MPValue;
                            MPValue = PVSSearch::PVS(tempPVNode, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, isNullMoveAllowed, depthGone + 1, lastCheck, nullWindowSearch);
                            bestMoveValue = -MPValue->value;
                            if (bestMoveValue != -160000)
                            {
                                availMovesIr++;
                            }
                            move->value = bestMoveValue;
                        }
                        GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                        delete missingInfoAboutPrevStateFromMove;
                        missingInfoAboutPrevStateFromMove = nullptr;
                        if (UCI::IsTest())
                        {
                            Board::AreBoardsEqual(board4, *boardCopy);
                            delete boardCopy;
                            boardCopy = nullptr;
                        }
                        firstMoveIr = false;
                        if (bestMoveValue > alpha)
                        {
                            if (bestMoveValue >= beta)
                            {
                                if (isPVNode)
                                {
                                    move->isRefuteWithoutNullMove = true;
                                }
                            }
                            alpha = bestMoveValue;
                        }
                        move->isRefuteWithoutNullMove = false;
                        // AddToTable(board4, move, depth, false, nullWindowSearch, depthGone);
                    }
                    else
                    {
                        bool tempRepeat = false;
                        boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                        MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4, *move);
                        GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone, missingInfoAboutPrevStateFromMove);
                        if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                        {
                            tempRepeat = true;
                            value = 0;
                            move->value = 0;
                        }
                        else
                        {
                            bool tempPVNode = false;
                            if (move->isRefuteWithoutNullMove || (isPVNode && availMovesIr < 2))
                            {
                                tempPVNode = true;
                            }
                            int LMRDepth = 0;
                            bool exempt = false;
                            if (move->promotionPiece > 0)
                            {
                                exempt = true;
                            }
                            else if (move->endPiece > 0)
                            {
                                static const int pieceValLookup[8] = {0, 100, 320, 330, 500, 900, 20000, 0};
                                int movingPieceType = board4.mainBoard[move->endPlace] % 8;
                                int capturedPieceType = move->endPiece % 8;
                                if (pieceValLookup[capturedPieceType] >= pieceValLookup[movingPieceType])
                                {
                                    exempt = true;
                                }
                            }
                            if (!tempPVNode && !exempt && depth >= 3)
                            {
                                LMRDepth = 1;
                            }
                            delete MPValue;
                            MPValue = PVSSearch::PVS(tempPVNode, -alpha - Option::nullWindowSize, -alpha, depth - 1 - LMRDepth, *move, move2, move3, prevMove, board4, MAtESearch, isNullMoveAllowed, depthGone + 1, lastCheck, true);
                            value = -MPValue->value;
                            if (value != -160000)
                            {
                                availMovesIr++;
                            }
                            move->value = value;
                        }
                        if (value > alpha /* && value < beta */)
                        {
                            if (!tempRepeat)
                            {
                                bool tempPVNode = false;
                                if (isPVNode || move->isRefuteWithoutNullMove)
                                {
                                    tempPVNode = true;
                                }
                                delete MPValue;
                                MPValue = PVSSearch::PVS(tempPVNode, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, isNullMoveAllowed, depthGone + 1, lastCheck, nullWindowSearch);
                                value = -MPValue->value;
                                move->value = value;
                            }
                            if (value > alpha)
                            {
                                alpha = value;
                            }
                        }

                        GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                        delete missingInfoAboutPrevStateFromMove;
                        missingInfoAboutPrevStateFromMove = nullptr;
                        if (UCI::IsTest())
                        {
                            Board::AreBoardsEqual(board4, *boardCopy);
                            delete boardCopy;
                            boardCopy = nullptr;
                        }
                        if (value > bestMoveValue)
                        {
                            if (value >= beta)
                            {
                                if (isPVNode)
                                {
                                    move->isRefuteWithoutNullMove = true;
                                }
                            }
                            move->isRefuteWithoutNullMove = false;
                            // AddToTable(board4, move, depth, false, nullWindowSearch, depthGone);
                            bestMoveValue = value;
                        }
                    }
                }
                std::sort(moveList.moves, moveList.moves + moveList.count, [](Move *a, Move *b)
                          { return b->value > a->value; });
            }
        }
    }
    delete MPValue;
    MPValue = nullptr;
}
