#ifndef HOWL_TUNER_SCALE_CALIBRATOR_H
#define HOWL_TUNER_SCALE_CALIBRATOR_H

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "Board.h"
#include "BoardMaker.h"
#include "Option.h"
#include "tuner/TunerEvaluationState.h"
#include "tuner/TunerEvaluator.h"
#include "tuner/TunerLoss.h"
#include "tuner/TunerParameter.h"

namespace Tuner
{

struct PrecomputedPosition
{
    int whiteScoreCp;
    double result;
};

struct CalibrationReport
{
    bool success = false;
    std::string errorMessage = "";
    double originalScale = 400.0;
    double originalTrainLoss = 0.0;
    double originalValLoss = 0.0;
    double bestScale = 400.0;
    double bestTrainLoss = 0.0;
    double bestValLoss = 0.0;
};

class TunerScaleCalibrator
{
public:
    static double ComputeLoss(const std::vector<PrecomputedPosition>& positions, double scale)
    {
        if (positions.empty())
        {
            return 0.0;
        }
        double totalSqError = 0.0;
        for (const auto& pos : positions)
        {
            double expected = TunerLossEvaluator::CentipawnsToExpectedWhiteScore(pos.whiteScoreCp, scale);
            double diff = pos.result - expected;
            totalSqError += diff * diff;
        }
        return totalSqError / static_cast<double>(positions.size());
    }

    static bool LoadTsv(const std::string& filepath, TunerDataset& dataset)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            return false;
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty()) continue;
            auto tabPos = line.rfind('\t');
            if (tabPos == std::string::npos) continue;

            std::string fen = line.substr(0, tabPos);
            std::string resStr = line.substr(tabPos + 1);
            try
            {
                double res = std::stod(resStr);
                dataset.Add(fen, res);
            }
            catch (...)
            {
                continue;
            }
        }
        return true;
    }

    static std::vector<PrecomputedPosition> PrecomputeScores(const TunerDataset& dataset,
                                                             const TunerEvaluationState& state)
    {
        std::vector<PrecomputedPosition> positions;
        positions.reserve(dataset.Size());

        for (std::size_t i = 0; i < dataset.Size(); ++i)
        {
            const auto& entry = dataset[i];
            std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(entry.fen));
            if (!board)
            {
                continue;
            }
            int stmScore = TunerEvaluator::Evaluate(*board, state);
            int whiteScore = TunerLossEvaluator::ToWhitePerspective(stmScore, board->sideToMove);
            positions.push_back({whiteScore, entry.result});
        }
        return positions;
    }

    static CalibrationReport Calibrate(const std::string& trainPath = "tuner-train.tsv",
                                       const std::string& valPath = "tuner-validation.tsv")
    {
        CalibrationReport report;

        TunerDataset trainDataset;
        if (!LoadTsv(trainPath, trainDataset) || trainDataset.Size() == 0)
        {
            report.success = false;
            report.errorMessage = "Missing or empty dataset file: " + trainPath;
            return report;
        }

        TunerDataset valDataset;
        if (!LoadTsv(valPath, valDataset) || valDataset.Size() == 0)
        {
            report.success = false;
            report.errorMessage = "Missing or empty dataset file: " + valPath;
            return report;
        }

        Option::Initialize();
        TunerRegistry registry = TunerRegistry::CreateRegistry();
        TunerEvaluationState state;
        state.LoadFromRegistry(registry);

        auto trainPositions = PrecomputeScores(trainDataset, state);
        auto valPositions = PrecomputeScores(valDataset, state);

        report.originalScale = 400.0;
        report.originalTrainLoss = ComputeLoss(trainPositions, 400.0);
        report.originalValLoss = ComputeLoss(valPositions, 400.0);

        // Step 1: Grid search to find approximate minimum in [50.0, 1000.0]
        double bestK = 400.0;
        double minLoss = report.originalTrainLoss;

        for (double k = 50.0; k <= 1000.0; k += 1.0)
        {
            double loss = ComputeLoss(trainPositions, k);
            if (loss < minLoss)
            {
                minLoss = loss;
                bestK = k;
            }
        }

        // Step 2: Refine with golden section search around bestK
        double a = std::max(10.0, bestK - 10.0);
        double b = bestK + 10.0;
        const double phi = (1.0 + std::sqrt(5.0)) / 2.0;
        const double resphi = 2.0 - phi;

        double c = a + resphi * (b - a);
        double d = b - resphi * (b - a);
        double fc = ComputeLoss(trainPositions, c);
        double fd = ComputeLoss(trainPositions, d);

        while (std::abs(b - a) > 0.01)
        {
            if (fc < fd)
            {
                b = d;
                d = c;
                fd = fc;
                c = a + resphi * (b - a);
                fc = ComputeLoss(trainPositions, c);
            }
            else
            {
                a = c;
                c = d;
                fc = fd;
                d = b - resphi * (b - a);
                fd = ComputeLoss(trainPositions, d);
            }
        }

        bestK = (a + b) / 2.0;
        report.bestScale = bestK;
        report.bestTrainLoss = ComputeLoss(trainPositions, bestK);
        report.bestValLoss = ComputeLoss(valPositions, bestK);
        report.success = true;

        return report;
    }
};

} // namespace Tuner

#endif // HOWL_TUNER_SCALE_CALIBRATOR_H
