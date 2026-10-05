#include "AttackPlaces.h"
#include "BoardInitializer.h"
#include "KingSetup.h"
#include "MoveLogic.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "tuner/TunerCoordinateDescent.h"

#include <iomanip>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
void InitializeEngine()
{
    Option::Initialize();
    AttackPlaces::Initialize();
    BoardInitializer::Initialize();
    PieceMoves::Initialize();
    MoveLogic::Initialize();
    KingSetup::Initialize();
    PassedPawnSetup::Initialize();
}

void CleanupEngine()
{
    AttackPlaces::Cleanup();
    BoardInitializer::Cleanup();
    PieceMoves::Cleanup();
    MoveLogic::Cleanup();
    KingSetup::Cleanup();
    PassedPawnSetup::Cleanup();
}

bool WriteState(const std::string& path, const Tuner::TunerRegistry& registry,
                const Tuner::CoordinateDescentResult* result)
{
    std::unordered_map<std::string, int> finalValues;
    if (result)
        for (const auto& change : result->changedParameters)
            finalValues[change.name] = change.finalValue;

    std::ofstream output(path);
    if (!output) return false;
    for (const auto& parameter : registry.GetParameters())
    {
        const auto it = finalValues.find(parameter.name);
        output << parameter.name << '\t'
               << (it == finalValues.end() ? parameter.currentValue : it->second) << '\n';
    }
    return output.good();
}

bool WriteAnchorScores(const std::string& path)
{
    std::vector<Tuner::TunerPosition> positions;
    if (!Tuner::TunerCoordinateDescent::LoadDataset("tuner-train.tsv", positions) ||
        !Tuner::TunerCoordinateDescent::LoadDataset("tuner-validation.tsv", positions))
        return false;
    std::ofstream output(path);
    if (!output) return false;
    std::unordered_set<std::string> written;
    for (const auto& position : positions)
        if (written.insert(position.positionKey).second)
            output << position.positionKey << '\t'
                   << EvaluationLogic::Evaluate(*position.board) << '\n';
    return output.good();
}
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: howl_tuner [--teacher-scores <path>] [--anchor-scores <path>] <ParameterFamily> [ParameterFamily ...]\n";
        return 2;
    }

    bool listSelection = false;
    std::string stateOut;
    std::string teacherScoresPath;
    std::string anchorScoresPath;
    std::string writeAnchorScoresPath;
    std::vector<Tuner::ParameterFamily> families;
    for (int i = 1; i < argc; ++i)
    {
        const std::string argument = argv[i];
        if (argument == "--list-selection")
        {
            listSelection = true;
            continue;
        }
        if (argument == "--state-out" && i + 1 < argc)
        {
            stateOut = argv[++i];
            continue;
        }
        if (argument == "--teacher-scores" && i + 1 < argc)
        {
            teacherScoresPath = argv[++i];
            continue;
        }
        if (argument == "--anchor-scores" && i + 1 < argc)
        {
            anchorScoresPath = argv[++i];
            continue;
        }
        if (argument == "--write-anchor-scores" && i + 1 < argc)
        {
            writeAnchorScoresPath = argv[++i];
            continue;
        }
        Tuner::ParameterFamily family;
        if (!Tuner::TunerCoordinateDescent::FamilyFromString(argument, family))
        {
            std::cerr << "Unknown parameter family: " << argument << '\n';
            return 2;
        }
        families.push_back(family);
    }

    InitializeEngine();
    const Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
    if (!writeAnchorScoresPath.empty())
    {
        const bool written = WriteAnchorScores(writeAnchorScoresPath);
        CleanupEngine();
        return written ? 0 : 1;
    }
    if (listSelection)
    {
        int selected = 0;
        std::unordered_set<std::string> names;
        bool duplicate = false;
        for (const auto& parameter : registry.GetParameters())
        {
            duplicate = duplicate || !names.insert(parameter.name).second;
            if (Tuner::TunerCoordinateDescent::IsParameterTunable(parameter, families) &&
                Tuner::TunerCoordinateDescent::GetParameterDelta(parameter, families) > 0)
                ++selected;
        }
        if (!stateOut.empty() && !WriteState(stateOut, registry, nullptr)) return 1;
        std::cout << "Canonical parameters: " << registry.Size() << '\n'
                  << "Selected parameters: " << selected << '\n'
                  << "Duplicate parameters: " << (duplicate ? "yes" : "no") << '\n';
        CleanupEngine();
        return duplicate ? 1 : 0;
    }

    const auto result = Tuner::TunerCoordinateDescent::RunFamilies(
        "tuner-train.tsv", "tuner-validation.tsv", families, 554.17, 8,
        teacherScoresPath, anchorScoresPath);
    CleanupEngine();

    if (result.parametersExamined == 0)
    {
        std::cerr << "No tunable parameters found for selected families.\n";
        return 1;
    }

    std::cout << std::fixed << std::setprecision(9)
              << "Training positions:       " << result.baselineTrainComponents.positions << '\n'
              << "Training teacher coverage: " << result.baselineTrainComponents.teacherPositions
              << " / " << result.baselineTrainComponents.positions << " ("
              << (result.baselineTrainComponents.positions == 0 ? 0.0 :
                  100.0 * result.baselineTrainComponents.teacherPositions /
                  result.baselineTrainComponents.positions) << "%)\n"
              << "Validation positions:     " << result.baselineValComponents.positions << '\n'
              << "Validation teacher coverage: " << result.baselineValComponents.teacherPositions
              << " / " << result.baselineValComponents.positions << " ("
              << (result.baselineValComponents.positions == 0 ? 0.0 :
                  100.0 * result.baselineValComponents.teacherPositions /
                  result.baselineValComponents.positions) << "%)\n"
              << "Baseline training result loss: " << result.baselineTrainComponents.result << '\n'
              << "Baseline training teacher loss: " << result.baselineTrainComponents.teacher << '\n'
              << "Baseline training anchor loss: " << result.baselineTrainComponents.anchor << '\n'
              << "Baseline training loss:   " << result.baselineTrainLoss << '\n'
              << "Baseline validation result loss: " << result.baselineValComponents.result << '\n'
              << "Baseline validation teacher loss: " << result.baselineValComponents.teacher << '\n'
              << "Baseline validation anchor loss: " << result.baselineValComponents.anchor << '\n'
              << "Baseline validation loss: " << result.baselineValLoss << '\n'
              << "Tuned training result loss: " << result.finalTrainComponents.result << '\n'
              << "Tuned training teacher loss: " << result.finalTrainComponents.teacher << '\n'
              << "Tuned training anchor loss: " << result.finalTrainComponents.anchor << '\n'
              << "Tuned training loss:      " << result.finalTrainLoss << '\n'
              << "Tuned validation result loss: " << result.finalValComponents.result << '\n'
              << "Tuned validation teacher loss: " << result.finalValComponents.teacher << '\n'
              << "Tuned validation anchor loss: " << result.finalValComponents.anchor << '\n'
              << "Tuned validation loss:    " << result.finalValLoss << '\n'
              << "Sweeps:                   " << result.sweeps << '\n'
              << "Optimizer steps:          " << result.optimizerSteps << '\n'
              << "Termination reason:       " << result.terminationReason << '\n'
              << "Changed parameters:       " << result.parametersChanged << '\n';

    for (const auto& change : result.changedParameters)
    {
        std::cout << change.name << ": " << change.initialValue
                  << " -> " << change.finalValue << '\n';
    }
    if (!stateOut.empty() && !WriteState(stateOut, registry, &result))
    {
        std::cerr << "Could not write tuner state: " << stateOut << '\n';
        return 1;
    }
    return 0;
}
