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



    constexpr int HistoryLimit = 16384;
    constexpr size_t ContinuationTableSize = 1u << 16;
    int mainHistory[2][64][64] = {};

    struct ContinuationHistoryEntry
    {
        uint32_t key = 0;
        int score = 0;
        bool occupied = false;
    };

    ContinuationHistoryEntry continuationHistory[ContinuationTableSize] = {};

    bool IsQuietMove(const Move &move)
    {
        return move.endPiece == 0 && move.promotionPiece <= 0;
    }

    int HistoryScore(int side, const Move &move)
    {
        return mainHistory[side][move.beginPlace][move.endPlace];
    }

    bool ContinuationKey(const Move &previousMove, const Move &move, uint32_t &key)
    {
        if (previousMove.beginPlace < 0 || previousMove.beginPlace >= 64 ||
            previousMove.endPlace < 0 || previousMove.endPlace >= 64)
            return false;
        key = static_cast<uint32_t>(previousMove.beginPlace) |
              (static_cast<uint32_t>(previousMove.endPlace) << 6) |
              (static_cast<uint32_t>(move.beginPlace) << 12) |
              (static_cast<uint32_t>(move.endPlace) << 18);
        return true;
    }

    ContinuationHistoryEntry &ContinuationEntry(uint32_t key)
    {
        return continuationHistory[(key * 0x9e3779b1u) & (ContinuationTableSize - 1)];
    }

    int ContinuationHistoryScore(const Move &previousMove, const Move &move)
    {
        uint32_t key;
        if (!ContinuationKey(previousMove, move, key))
            return 0;
        const ContinuationHistoryEntry &entry = ContinuationEntry(key);
        return entry.occupied && entry.key == key ? entry.score : 0;
    }

    void UpdateHistory(int side, const Move &move, int depth, bool improvedAlpha)
    {
        const int magnitude = std::min(1024, depth * depth * 16);
        const int bonus = improvedAlpha ? magnitude : -magnitude;
        int &score = mainHistory[side][move.beginPlace][move.endPlace];
        score += bonus - score * std::abs(bonus) / HistoryLimit;
        score = std::clamp(score, -HistoryLimit, HistoryLimit);
    }

    void UpdateContinuationHistory(const Move &previousMove, const Move &move,
                                   int depth, bool improvedAlpha)
    {
        uint32_t key;
        if (!ContinuationKey(previousMove, move, key))
            return;
        ContinuationHistoryEntry &entry = ContinuationEntry(key);
        if (!entry.occupied || entry.key != key)
        {
            entry.key = key;
            entry.score = 0;
            entry.occupied = true;
        }
        const int magnitude = std::min(1024, depth * depth * 16);
        const int bonus = improvedAlpha ? magnitude : -magnitude;
        entry.score += bonus - entry.score * std::abs(bonus) / HistoryLimit;
        entry.score = std::clamp(entry.score, -HistoryLimit, HistoryLimit);
    }

    int CombinedHistoryScore(int side, const Move &previousMove, const Move &move)
    {
        return HistoryScore(side, move) + ContinuationHistoryScore(previousMove, move);
    }

    int HistoryReductionAdjustment(int side, const Move &previousMove, const Move &move)
    {
        return std::clamp(CombinedHistoryScore(side, previousMove, move) / 4096, -2, 2);
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
              turn(turnValue), previousMove(previousMoveValue), packedTTMove(ttMoveValue) {}

        ~MovePicker()
        {
            for (int i = 0; i < entryCount; ++i)
                delete entries[i].move;
        }

        MovePicker(const MovePicker&) = delete;
        MovePicker& operator=(const MovePicker&) = delete;

        Move* Next()
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
                    stage = Stage::StrongQuiets;
                    if (Move* move = BuildKiller(1))
                        return move;
                    break;
                case Stage::StrongQuiets:
                    ScoreAndClassifyQuiets();
                    if (Move* move = Select(strongQuiets, strongQuietCount, strongQuietCursor))
                        return move;
                    stage = Stage::RemainingQuiets;
                    break;
                case Stage::RemainingQuiets:
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
            entry.move->givesCheck = MoveLogic::MoveGivesCheck(board, *entry.move);
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
                    Score(index);
                    entries[index].key = BasePvsOrderingScore(entries[index].move);
                    if (MatchesTT(*entries[index].move))
                        ttEntry = index;
                    if (entries[index].move->value >= 0)
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
                entries[index].key = 100000 +
                    CombinedHistoryScore(turn, previousMove, *entries[index].move) +
                    BasePvsOrderingScore(entries[index].move);
                if (entries[index].key >= 100000)
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
        const bool sparseMinorOnlyMaterial =
            board.pieces[turn * 8 + 5].size() == 0 &&
            board.pieces[turn * 8 + 4].size() == 0 &&
            minorPieceCount <= 1;
        const bool hasNonPawn = minorPieceCount > 0 ||
            board.pieces[turn * 8 + 4].size() > 0 ||
            board.pieces[turn * 8 + 5].size() > 0;
        return hasNonPawn && !sparseMinorOnlyMaterial;
    }

}

void PVSSearch::ResetHistory()
{
    std::fill(&mainHistory[0][0][0], &mainHistory[0][0][0] + 2 * 64 * 64, 0);
    for (ContinuationHistoryEntry &entry : continuationHistory)
        entry = ContinuationHistoryEntry{};
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
    return ContinuationHistoryScore(prevMove, move);
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

MovePrintValue *PVSSearch::PVS(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, bool selectiveSearch)
{
    MovePrintValue* result = SearchNode(isPVNode, alpha, beta, depth, prevMove,
        move1, move2, move3, board4, MAtESearch, isNullMoveAllowed, depthGone,
        previousMoveWasCheck, nullWindowSearch, selectiveSearch);
    if (selectiveSearch)
        result->MarkSpeculative(SearchProvenance::ReducedSearch);
    return result;
}

MovePrintValue *PVSSearch::SearchNode(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, bool selectiveSearch)
{
    if (Search::stopRequested.load(std::memory_order_relaxed))
    {
        MovePrintValue *abortRet = new MovePrintValue();
        abortRet->value = 0;
        abortRet->bound = SearchBound::Upper;
        abortRet->MarkSpeculative(SearchProvenance::Aborted);
        return abortRet;
    }

    Move *SelectedMove = nullptr;
    Board *boardCopy = nullptr;

    MovePrintValue *retValue = new MovePrintValue();
    retValue->printString = "";

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

    // Frontier collapse at non-PV depth <= 4:
    bool allowFrontierCollapse = selectiveSearch || !isNullWindow;
    if (false && allowFrontierCollapse && !isPVNode && depth <= 4)
    {
        delete retValue;
        retValue = nullptr;
        delete MPValue;
        MPValue = nullptr;
        return StartQSearch(isPVNode, alpha, beta, prevMove, depthGone, move1, move2, move3, board4, nullWindowSearch, previousMoveWasCheck);
    }

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
    TTEntry ttEntry{};
    bool ttHit = TranspositionTable::Probe(board4.ZobristHashCode, ttEntry);
    if (ttHit)
        ttEntry.score = MateScore::FromTranspositionTable(ttEntry.score, depthGone);
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
    if (TranspositionTable::CutoffsEnabled() && !MAtESearch && !isPVNode && ttHit && ttEntry.depth >= depth && TTFlagIsRigorous(ttEntry.flag))
    {
        if (TTBaseFlag(ttEntry.flag) == TT_EXACT)
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
        else if (TTBaseFlag(ttEntry.flag) == TT_LOWER_BOUND && ttEntry.score >= beta)
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
        else if (TTBaseFlag(ttEntry.flag) == TT_UPPER_BOUND && ttEntry.score <= alpha)
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
    if (!isPVNode && !nodeInCheck && !MAtESearch && depth <= 3 &&
        beta < 159800 && alpha > -159800)
    {
        const int staticValue = EvaluationLogic::Evaluate(board4);
        const int reverseFutilityMargin = 90 * depth;
        if (staticValue - reverseFutilityMargin >= beta)
        {
            retValue->value = staticValue;
            retValue->bound = SearchBound::Lower;
            retValue->MarkSpeculative(SearchProvenance::ForwardPruning);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
    }
    if (isNullMoveAllowed && prevMove.promotionPiece != -1 && !isPVNode &&
        !nodeInCheck && !MAtESearch && depth >= 3 &&
        alpha > -159800 && beta < 159800)
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
            int staticEval = EvaluationLogic::Evaluate(board4);
            if (staticEval >= beta)
            {
                const int evalReduction = std::clamp((staticEval - beta) / 250, 0, 2);
                int R = 2 + depth / 2 + evalReduction;
                R = std::min(R, depth - 1);
                Move nullMove{};
                nullMove.promotionPiece = -1;
                MissingInfoAboutPrevStateFromMove undoInfo(board4);
                GameLogic::DoMove(board4, nullMove, prevMove, depthGone, depthGone);

                std::unique_ptr<MovePrintValue> nullRes(PVS(false, -beta, -beta + 1, depth - 1 - R, nullMove, move2, move3, prevMove, board4, MAtESearch, false, depthGone + 1, previousMoveWasCheck, true, selectiveSearch));
                int nullScore = -nullRes->value;

                GameLogic::UndoMove(board4, nullMove, undoInfo);

                const bool nullFailedHigh = nullScore >= beta;
                bool verificationPerformed = false;
                bool cutoffAccepted = nullFailedHigh;
                const int staticMargin = staticEval - beta;
                const int nullMargin = nullScore - beta;
                const bool sparseMaterialRisk =
                    (totalPieceCount <= 10 && totalNonPawnMaterialCount <= 3) ||
                    (totalPieceCount <= 8 && nonPawnMaterialCount == 1);
                int uncertainty = 0;
                uncertainty += staticMargin < 150 ? 1 : 0;
                uncertainty += nullMargin < 100 ? 1 : 0;
                uncertainty += (R * 3 >= depth * 2) ? 1 : 0;
                if (nullFailedHigh && !sparseMaterialRisk && uncertainty == 3)
                {
                    // Certified full-depth TT bounds were resolved before the
                    // null probe. Without that evidence, verify at the requested
                    // depth in this node. A reduced
                    // verification can miss the depth-sensitive false-positive
                    // tail. Reuse the ensuing ordinary result, on either side
                    // of beta, rather than accepting the speculative null score
                    // or starting a duplicate verification/continuation pass.
                    cutoffAccepted = false;
                    retValue->AddProvenance(SearchProvenance::NullVerification);
                }

                const bool verificationRequired = nullFailedHigh && sparseMaterialRisk &&
                    (totalPieceCount <= 6 || uncertainty >= 2);
                if (verificationRequired)
                {
                    verificationPerformed = true;
                    const int verificationReduction = uncertainty >= 3 ? 2 : 3;
                    const int verificationDepth = std::max(0, depth - verificationReduction);
                    std::unique_ptr<MovePrintValue> verification(PVS(
                        false, beta - 1, beta, verificationDepth, prevMove,
                        move1, move2, move3, board4, MAtESearch, false,
                        depthGone, previousMoveWasCheck, true,
                        selectiveSearch));
                    cutoffAccepted = verification->value >= beta;
                }

#if HOWL_CORRECTNESS_TESTING
                RecordNullMoveAttempt(depth, R, staticEval - beta, board4, turn,
                                      nullFailedHigh,
                                      verificationPerformed && cutoffAccepted);
#endif

                if (cutoffAccepted)
                {
                    retValue->value = nullScore;
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
    if constexpr (ProductionIGGEnabled)
    {
        MoveList moveList = MoveLogic::MoveGenerator(board4, depth, depthGone);
        IGG(isPVNode, alpha, beta, depth, prevMove, move1, move2, move3, board4, MAtESearch, isNullMoveAllowed, depthGone, previousMoveWasCheck, nullWindowSearch, moveList);
        deleteMoveList(moveList);
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
    const bool hasTTMove = movePicker.HasTTMove();
#if HOWL_CORRECTNESS_TESTING
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
    {
        bool firstMove = true;
        if (!isPVNode && !nodeInCheck && !MAtESearch && depth >= 5 &&
            beta < 159500 && alpha > -159500)
        {
            const int probBeta = beta + 120;
            int forcingMovesTried = 0;
            MovePicker probPicker(board4, depth, depthGone, turn, prevMove,
                                  ttHit ? ttEntry.bestMove : 0);
            for (int i = 0; i < 6 && forcingMovesTried < 2; ++i)
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
                    std::unique_ptr<MovePrintValue> reducedResult(PVS(
                        false, -probBeta, -probBeta + 1, depth - 4, probMove,
                        move2, move3, prevMove, board4, false, true,
                        depthGone + 1, givesCheck, true, true));
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
            if (Search::stopRequested.load(std::memory_order_relaxed))
            {
                delete MPValue;
                MPValue = nullptr;
                retValue->value = 0;
                retValue->bound = SearchBound::Upper;
                retValue->MarkSpeculative(SearchProvenance::Aborted);
                return retValue;
            }
            const int alphaBeforeMove = alpha;
            int LMRDepth = 0;
            uint8_t moveProof = NoProof;
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
                    bool tempPVNode = false;
                    if (isPVNode || move->isRefuteWithoutNullMove)
                    {
                        tempPVNode = true;
                    }
                    delete MPValue;
                    MPValue = PVS(tempPVNode, -beta, -alpha,
                                  depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, true,
                                  depthGone + 1, previousMoveWasCheck, nullWindowSearch,
                                  selectiveSearch);
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
                    if (!firstMoveWasRepetition && !selectiveSearch &&
                        IsQuietMove(*move) && !IsMateScore(bestMoveValue))
                    {
                        UpdateHistory(turn, *move, depth, true);
                        UpdateContinuationHistory(prevMove, *move, depth, true);
                    }
                    if (bestMoveValue >= beta)
                    {
#if HOWL_CORRECTNESS_TESTING
                        RecordMoveOrderingCutoff(i, hasTTMove, *move, depth, depthGone, isPVNode, nullWindowSearch || beta == alpha + 1);
#endif
                        if (!selectiveSearch)
                        {
                            RecordKiller(depthGone, *move);
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
                                TranspositionTable::Store(board4.ZobristHashCode,
                                    MateScore::ToTranspositionTable(move->value, depthGone),
                                    depth, cutoffFlag, packed);
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
                // Late quiet pruning is enabled for selective searches or at wide-window nodes.
                bool allowLQP = selectiveSearch || !isNullWindow;
                if (false && allowLQP && !isPVNode && depth <= 4 && availMoves > 0 && move->endPiece == 0 && move->promotionPiece <= 0 &&
                    (move->CastleFlag & 15) == 0 &&
                    alpha > -159800 && beta < 159800)
                {
                    if (depth == 1 && quietMovesSearched >= 0) continue;
                    if (depth == 2 && quietMovesSearched >= 1) continue;
                    if (depth == 3 && quietMovesSearched >= 2) continue;
                    if (depth == 4 && quietMovesSearched >= 4) continue;
                }

                if (false && !isPVNode && depth <= 2 && availMoves > 0 && move->endPiece == 0 && move->promotionPiece <= 0 &&
                    (move->CastleFlag & 15) == 0 &&
                    alpha > -159800 && beta < 159800)
                {
                    if (staticEval == -200000)
                    {
                        if (inCheck < 0)
                        {
                            inCheck = BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove) ? 1 : 0;
                        }
                        if (inCheck == 0)
                        {
                            staticEval = EvaluationLogic::Evaluate(board4);
                        }
                    }
                    if (inCheck == 0 && staticEval != -200000)
                    {
                        int futilityMargin = (depth == 1) ? 150 : 300;
                        if (staticEval + futilityMargin <= alpha)
                        {
#if HOWL_CORRECTNESS_TESTING
                            g_futilityPruningSkippedQuietMoves++;

                            // Measure futility pruning safety in instrumentation mode:
                            MissingInfoAboutPrevStateFromMove *diagMissing = new MissingInfoAboutPrevStateFromMove(board4, *move);
                            GameLogic::DoMove(board4, *move, prevMove, depth, depthGone, diagMissing);
                            int diagValue = 0;
                            if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                            {
                                diagValue = 0;
                            }
                            else
                            {
                                bool oppInCheck = BoardLogic::UnderAttack(board4, board4.pieces[(!board4.sideToMove) * 8 + 6].front(), board4.sideToMove);
                                MovePrintValue *diagMP = PVS(false, -alpha - Option::nullWindowSize, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, true, depthGone + 1, oppInCheck, true);
                                diagValue = -diagMP->value;
                                std::string pvStr = diagMP->printString;
                                if (diagValue > alpha)
                                {
                                    delete diagMP;
                                    diagMP = PVS(false, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, true, depthGone + 1, oppInCheck, nullWindowSearch);
                                    diagValue = -diagMP->value;
                                    pvStr = diagMP->printString;
                                }
                                delete diagMP;
                            }
                            bool givesCheck = BoardLogic::UnderAttack(board4, board4.pieces[(!board4.sideToMove) * 8 + 6].front(), board4.sideToMove);
                            GameLogic::UndoMove(board4, *move, *diagMissing);
                            delete diagMissing;

                            RecordFutilityCandidate(i, depth, depthGone, *move, staticEval, alpha, beta, diagValue, givesCheck);
#endif
                            continue;
                        }
                    }
                }

                bool tempRepeat = false;
                bool trustedValue = false;
                const int newDepth = depth - 1;
                if (newDepth >= 1)
                {
                    const int moveNumber = i + 1;
                    LMRDepth = static_cast<int>(std::lround(
                        0.60 * std::log(static_cast<double>(depth)) *
                        std::log(static_cast<double>(moveNumber))));
                    if (IsQuietMove(*move))
                        LMRDepth -= HistoryReductionAdjustment(turn, prevMove, *move);
                    if (move->givesCheck)
                        LMRDepth--;
                    if (move->promotionPiece > 0)
                        LMRDepth--;
                    if (move->endPiece > 0)
                        LMRDepth--;
                    LMRDepth = std::max(0, LMRDepth);
                    const int reducedChildDepth = std::clamp(newDepth - LMRDepth, 1, newDepth);
                    LMRDepth = newDepth - reducedChildDepth;
                }
                if (MAtESearch)
                    LMRDepth = 0;
                if (isPVNode)
                    LMRDepth = 0;
                const bool isTTMove = ttHit && ttEntry.bestMove != 0 &&
                    move->beginPlace == TTMoveHelper::UnpackFrom(ttEntry.bestMove) &&
                    move->endPlace == TTMoveHelper::UnpackTo(ttEntry.bestMove) &&
                    (TTMoveHelper::UnpackPromotion(ttEntry.bestMove) == 0
                         ? move->promotionPiece <= 0
                         : move->promotionPiece == TTMoveHelper::UnpackPromotion(ttEntry.bestMove));
                const int combinedHistory = IsQuietMove(*move)
                    ? CombinedHistoryScore(turn, prevMove, *move)
                    : 0;
                const bool pruningContext = !isPVNode && !nodeInCheck &&
                    !MAtESearch && !isTTMove &&
                    alpha > -159800 && beta < 159800;
                const int negativeHistoryStrength =
                    std::clamp(-combinedHistory / 4096, 0, 3);
                const int positiveHistoryStrength =
                    std::clamp(combinedHistory / 4096, 0, 3);
                const int historyMoveCountAdjustment =
                    positiveHistoryStrength * 3 - negativeHistoryStrength * 3;
                const int allowedQuietMoves = std::max(
                    3, 3 + depth * 2 + historyMoveCountAdjustment);
                const int moveCountDepthLimit =
                    1 + std::min(2, negativeHistoryStrength);
                const bool moveCountPruningCandidate = pruningContext &&
                    IsQuietMove(*move) && depth <= moveCountDepthLimit &&
                    i >= allowedQuietMoves;

                bool valueFutilityCandidate = false;
                const int valueFutilityDepthLimit =
                    2 + (negativeHistoryStrength >= 2 ? 1 : 0);
                if (pruningContext && IsQuietMove(*move) &&
                    depth <= valueFutilityDepthLimit)
                {
                    if (staticEval == -200000)
                        staticEval = EvaluationLogic::Evaluate(board4);
                    const int historyMarginAdjustment =
                        std::clamp(combinedHistory / 64, -240, 180);
                    const int futilityMargin = std::max(
                        40, 100 + 120 * depth + historyMarginAdjustment);
                    valueFutilityCandidate = staticEval + futilityMargin <= alpha;
                }
                const bool seePruningCandidate = pruningContext &&
                    move->endPiece > 0 && move->promotionPiece <= 0 &&
                    depth <= 1 && move->value <= -150;

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
                                       seePruningCandidate))
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
                    }

                    const bool reducedSearch = LMRDepth > 0;
                    delete MPValue;
                    MPValue = PVS(reducedSearch ? false : tempPVNode,
                                  -alpha - Option::nullWindowSize, -alpha,
                                  depth - 1 - LMRDepth, *move, move2, move3, prevMove, board4, MAtESearch, true,
                                  depthGone + 1, previousMoveWasCheck, true,
                                  selectiveSearch || reducedSearch);
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
                        MPValue = PVS(false, -alpha - Option::nullWindowSize, -alpha,
                                      depth - 1 - smallerReduction, *move, move2, move3,
                                      prevMove, board4, MAtESearch, true, depthGone + 1,
                                      previousMoveWasCheck, true, true);
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
                        MPValue = PVS(confirmationPVNode, -beta, -alpha, depth - 1,
                                      *move, move2, move3, prevMove, board4, MAtESearch,
                                      true, depthGone + 1, previousMoveWasCheck,
                                      nullWindowSearch, selectiveSearch);
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
                    if (!tempRepeat && !selectiveSearch && IsQuietMove(*move) &&
                        !IsMateScore(value))
                    {
                        UpdateHistory(turn, *move, depth, true);
                        UpdateContinuationHistory(prevMove, *move, depth, true);
                    }
                    alpha = value;
                }
                else if (trustedValue && !tempRepeat && !selectiveSearch &&
                         IsQuietMove(*move) && SelectedMove != nullptr &&
                         bestMoveValue > value && alphaBeforeMove >= value &&
                         !IsMateScore(value))
                {
                    UpdateHistory(turn, *move, depth, false);
                    UpdateContinuationHistory(prevMove, *move, depth, false);
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
                            RecordKiller(depthGone, *move);
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
                                TranspositionTable::Store(board4.ZobristHashCode,
                                    MateScore::ToTranspositionTable(move->value, depthGone),
                                    depth, cutoffFlag, packed);
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
                MovePrintValue storedResult;
                storedResult.bound = resultBound;
                storedResult.SetProof(bestLowerProof, allUpperProof);
                storedResult.selective = resultSelective;
                const uint8_t flag = TTFlagForResult(storedResult);
                uint16_t packed = TTMoveHelper::PackMove(*SelectedMove);
                TranspositionTable::Store(board4.ZobristHashCode,
                    MateScore::ToTranspositionTable(bestMoveValue, depthGone),
                    depth, flag, packed);
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
