#ifndef HOWL_TUNER_LOSS_H
#define HOWL_TUNER_LOSS_H

#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Board.h"
#include "BoardMaker.h"
#include "tuner/TunerEvaluationState.h"
#include "tuner/TunerEvaluator.h"

namespace Tuner
{

struct TunerDatasetEntry
{
    std::string fen;
    double result = 0.5; // 1.0 = White win, 0.5 = draw, 0.0 = Black win
};

struct HybridLossComponents
{
    double result = 0.0;
    double teacher = 0.0;
    double anchor = 0.0;
    double combined = 0.0;
    std::size_t positions = 0;
    std::size_t teacherPositions = 0;

    HybridLossComponents& operator+=(const HybridLossComponents& other)
    {
        result += other.result;
        teacher += other.teacher;
        anchor += other.anchor;
        combined += other.combined;
        positions += other.positions;
        teacherPositions += other.teacherPositions;
        return *this;
    }

    void Divide(double divisor)
    {
        if (divisor == 0.0) return;
        result /= divisor;
        teacher /= divisor;
        anchor /= divisor;
        combined /= divisor;
    }
};

class TunerDataset
{
public:
    void Add(std::string fen, double result)
    {
        entries.push_back({std::move(fen), result});
    }

    std::size_t Size() const
    {
        return entries.size();
    }

    const TunerDatasetEntry& operator[](std::size_t index) const
    {
        return entries[index];
    }

    const std::vector<TunerDatasetEntry>& GetEntries() const
    {
        return entries;
    }

    void Clear()
    {
        entries.clear();
    }

private:
    std::vector<TunerDatasetEntry> entries;
};

class TunerLossEvaluator
{
public:
    static constexpr double FixedLogisticScale = 554.17;

    // Convert centipawns (from White's perspective) to expected White score in [0.0, 1.0]
    static double ScoreToProbability(double sideToMoveScoreCp, double scale = FixedLogisticScale)
    {
        return 1.0 / (1.0 + std::pow(10.0, -sideToMoveScoreCp / scale));
    }

    static double CentipawnsToExpectedWhiteScore(double whiteScoreCp, double scale = FixedLogisticScale)
    {
        return ScoreToProbability(whiteScoreCp, scale);
    }

    static HybridLossComponents PositionLoss(double candidateProbability,
                                             double gameResult,
                                             double baselineProbability,
                                             bool hasTeacherScore,
                                             double teacherProbability)
    {
        const double resultDiff = candidateProbability - gameResult;
        const double anchorDiff = candidateProbability - baselineProbability;
        HybridLossComponents loss;
        loss.result = resultDiff * resultDiff;
        loss.anchor = anchorDiff * anchorDiff;
        loss.positions = 1;
        if (hasTeacherScore)
        {
            const double teacherDiff = candidateProbability - teacherProbability;
            loss.teacher = teacherDiff * teacherDiff;
            loss.combined = 0.50 * loss.result + 0.35 * loss.teacher + 0.15 * loss.anchor;
            loss.teacherPositions = 1;
        }
        else
        {
            constexpr double survivingWeight = 0.50 + 0.15;
            loss.combined = (0.50 / survivingWeight) * loss.result +
                            (0.15 / survivingWeight) * loss.anchor;
        }
        return loss;
    }

    // Convert side-to-move evaluation score to White perspective score
    static int ToWhitePerspective(int sideToMoveScore, bool sideToMove)
    {
        // In Howl Board: sideToMove == false is White, true is Black
        return (!sideToMove) ? sideToMoveScore : -sideToMoveScore;
    }

    // Evaluate single entry expected score from White perspective
    static double EvaluateExpectedWhiteScore(const std::string& fen,
                                             const TunerEvaluationState& state,
                                             double scale = FixedLogisticScale)
    {
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
        if (!board)
        {
            return 0.5;
        }

        int stmScore = TunerEvaluator::Evaluate(*board, state);
        int whiteScore = ToWhitePerspective(stmScore, board->sideToMove);
        return CentipawnsToExpectedWhiteScore(static_cast<double>(whiteScore), scale);
    }

    // Compute Mean Squared Error across the dataset
    static double ComputeLoss(const TunerDataset& dataset,
                              const TunerEvaluationState& state,
                              double scale = FixedLogisticScale)
    {
        if (dataset.Size() == 0)
        {
            return 0.0;
        }

        double totalSquaredError = 0.0;

        for (std::size_t i = 0; i < dataset.Size(); ++i)
        {
            const auto& entry = dataset[i];
            std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(entry.fen));
            if (!board)
            {
                continue;
            }

            int stmScore = TunerEvaluator::Evaluate(*board, state);
            int whiteScore = ToWhitePerspective(stmScore, board->sideToMove);
            double expectedWhite = CentipawnsToExpectedWhiteScore(static_cast<double>(whiteScore), scale);

            double diff = entry.result - expectedWhite;
            totalSquaredError += diff * diff;
        }

        return totalSquaredError / static_cast<double>(dataset.Size());
    }
};

} // namespace Tuner

#endif // HOWL_TUNER_LOSS_H
