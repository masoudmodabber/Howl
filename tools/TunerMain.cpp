#include "AttackPlaces.h"
#include "BoardInitializer.h"
#include "KingSetup.h"
#include "MoveLogic.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "tuner/TunerCoordinateDescent.h"

#include <iomanip>
#include <iostream>
#include <string>
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
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: howl_tuner <ParameterFamily> [ParameterFamily ...]\n";
        return 2;
    }

    std::vector<Tuner::ParameterFamily> families;
    families.reserve(static_cast<std::size_t>(argc - 1));
    for (int i = 1; i < argc; ++i)
    {
        Tuner::ParameterFamily family;
        if (!Tuner::TunerCoordinateDescent::FamilyFromString(argv[i], family))
        {
            std::cerr << "Unknown parameter family: " << argv[i] << '\n';
            return 2;
        }
        families.push_back(family);
    }

    InitializeEngine();
    const auto result = Tuner::TunerCoordinateDescent::RunFamilies(
        "tuner-train.tsv", "tuner-validation.tsv", families);
    CleanupEngine();

    if (result.parametersExamined == 0)
    {
        std::cerr << "No tunable parameters found for selected families.\n";
        return 1;
    }

    std::cout << std::fixed << std::setprecision(9)
              << "Baseline training loss:   " << result.baselineTrainLoss << '\n'
              << "Baseline validation loss: " << result.baselineValLoss << '\n'
              << "Tuned training loss:      " << result.finalTrainLoss << '\n'
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
    return 0;
}
