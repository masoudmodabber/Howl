#include "AttackPlaces.h"
#include "BoardInitializer.h"
#include "BoardMaker.h"
#include "EvaluationLogic.h"
#include "KingSetup.h"
#include "MoveLogic.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "tuner/TunerEvaluationState.h"
#include "tuner/TunerEvaluator.h"
#include "tuner/TunerParameter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
constexpr std::size_t DefaultPositionLimit = 10000;
constexpr std::size_t LowOccurrenceCount = 100;
constexpr double LowOccurrenceFraction = 0.001;
constexpr double NearZeroStandardDeviation = 1e-9;
constexpr double EffectiveRankRelativeTolerance = 1e-10;

uint64_t StableHash(const std::string& text)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    for (unsigned char c : text) { hash ^= c; hash *= UINT64_C(1099511628211); }
    return hash;
}

using Tuner::ParameterFamily;

struct Stats
{
    uint64_t nonzero = 0, positive = 0, negative = 0, zero = 0;
    long double sum = 0.0, sumSquares = 0.0;
    int minimum = std::numeric_limits<int>::max();
    int maximum = std::numeric_limits<int>::min();
    uint64_t nonlinearPositions = 0;
};

struct FamilyData
{
    std::vector<std::size_t> indices;
    std::vector<long double> crossProducts;
};

std::string FamilyName(ParameterFamily family)
{
    switch (family)
    {
    case ParameterFamily::PieceValue: return "PieceValue";
    case ParameterFamily::PawnStructure: return "PawnStructure";
    case ParameterFamily::PassedPawnV2: return "PassedPawnV2";
    case ParameterFamily::PieceSquare: return "PieceSquare";
    case ParameterFamily::KnightMobility: return "KnightMobility";
    case ParameterFamily::BishopMobility: return "BishopMobility";
    case ParameterFamily::RookMobility: return "RookMobility";
    case ParameterFamily::QueenMobility: return "QueenMobility";
    case ParameterFamily::Attack: return "Attack";
    case ParameterFamily::Inline: return "Inline";
    case ParameterFamily::RookFile: return "RookFile";
    case ParameterFamily::KnightOutpost: return "KnightOutpost";
    case ParameterFamily::IsolatedPawn: return "IsolatedPawn";
    case ParameterFamily::RookBehindPassedPawn: return "RookBehindPassedPawn";
    case ParameterFamily::EndgameWeights: return "EndgameWeights";
    case ParameterFamily::KingSafety: return "KingSafety";
    default: return "SelectionOnly";
    }
}

std::string PhaseName(const std::string& name)
{
    if (name.find("MiddleGame") != std::string::npos) return "MG";
    if (name.find("EndGame") != std::string::npos) return "EG";
    return "Both/Scalar";
}

std::string ShapeName(ParameterFamily family, const std::string& name)
{
    if (family == ParameterFamily::PieceSquare || family == ParameterFamily::Attack ||
        family == ParameterFamily::PassedPawnV2 ||
        family == ParameterFamily::KnightMobility || family == ParameterFamily::BishopMobility ||
        family == ParameterFamily::RookMobility || family == ParameterFamily::QueenMobility ||
        name.find('_') != std::string::npos)
        return "table_element";
    return "scalar";
}

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

std::vector<double> JacobiEigenvalues(std::vector<double> matrix, int n)
{
    if (n == 1) return {matrix[0]};
    for (int iteration = 0; iteration < 100 * n * n; ++iteration)
    {
        int p = 0, q = 1;
        double largest = 0.0;
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
                if (std::abs(matrix[i * n + j]) > largest)
                {
                    largest = std::abs(matrix[i * n + j]); p = i; q = j;
                }
        if (largest < 1e-12) break;
        const double app = matrix[p * n + p], aqq = matrix[q * n + q];
        const double apq = matrix[p * n + q];
        const double angle = 0.5 * std::atan2(2.0 * apq, aqq - app);
        const double c = std::cos(angle), s = std::sin(angle);
        for (int k = 0; k < n; ++k)
        {
            if (k == p || k == q) continue;
            const double akp = matrix[k * n + p], akq = matrix[k * n + q];
            matrix[k * n + p] = matrix[p * n + k] = c * akp - s * akq;
            matrix[k * n + q] = matrix[q * n + k] = s * akp + c * akq;
        }
        matrix[p * n + p] = c * c * app - 2.0 * s * c * apq + s * s * aqq;
        matrix[q * n + q] = s * s * app + 2.0 * s * c * apq + c * c * aqq;
        matrix[p * n + q] = matrix[q * n + p] = 0.0;
    }
    std::vector<double> values(n);
    for (int i = 0; i < n; ++i) values[i] = std::max(0.0, matrix[i * n + i]);
    std::sort(values.begin(), values.end());
    return values;
}
}

int main(int argc, char** argv)
{
    try
    {
        std::string corpus = "tuner-train.tsv";
        std::filesystem::path outputDirectory = ".eval-identifiability";
        std::filesystem::path activeFamilyOutput;
        std::string activeFamilyName;
        std::size_t limit = DefaultPositionLimit;
        bool trainSplit = false;
        for (int i = 1; i < argc; ++i)
        {
            const std::string arg = argv[i];
            if (arg == "--corpus" && i + 1 < argc) corpus = argv[++i];
            else if (arg == "--output-dir" && i + 1 < argc) outputDirectory = argv[++i];
            else if (arg == "--limit" && i + 1 < argc) limit = std::stoull(argv[++i]);
            else if (arg == "--train-split") trainSplit = true;
            else if (arg == "--active-family" && i + 1 < argc) activeFamilyName = argv[++i];
            else if (arg == "--active-family-out" && i + 1 < argc) activeFamilyOutput = argv[++i];
            else if (arg == "--knight-outpost-active-out" && i + 1 < argc)
            {
                activeFamilyName = "KnightOutpost";
                activeFamilyOutput = argv[++i];
            }
            else throw std::runtime_error("Usage: howl_eval_identifiability [--corpus PATH] [--output-dir PATH] [--limit N] [--train-split] [--active-family NAME --active-family-out PATH]");
        }

        InitializeEngine();
        const Tuner::TunerRegistry registry = Tuner::TunerRegistry::CreateRegistry();
        Tuner::TunerEvaluationState baseline;
        if (!baseline.LoadFromRegistry(registry)) throw std::runtime_error("Registry mapping failed");
        const std::size_t parameterCount = registry.Size();
        std::vector<Tuner::TunerEvaluationState> zeroStates(parameterCount, baseline);
        std::vector<Tuner::TunerEvaluationState> oneStates(parameterCount, baseline);
        std::vector<Tuner::TunerEvaluationState> twoStates(parameterCount, baseline);
        for (std::size_t i = 0; i < parameterCount; ++i)
        {
            const auto& parameter = registry[i];
            *zeroStates[i].GetParameterPointer(parameter.family, parameter.semanticIndex) = 0;
            *oneStates[i].GetParameterPointer(parameter.family, parameter.semanticIndex) = 1;
            *twoStates[i].GetParameterPointer(parameter.family, parameter.semanticIndex) = 2;
            zeroStates[i].Derive(); oneStates[i].Derive(); twoStates[i].Derive();
        }

        std::map<std::string, FamilyData> families;
        for (std::size_t i = 0; i < parameterCount; ++i)
            families[FamilyName(registry[i].family)].indices.push_back(i);
        for (auto& item : families)
        {
            const std::size_t n = item.second.indices.size();
            item.second.crossProducts.assign(n * n, 0.0);
        }
        if (!activeFamilyOutput.empty() && families.find(activeFamilyName) == families.end())
            throw std::runtime_error("Unknown active family: " + activeFamilyName);

        std::vector<Stats> stats(parameterCount);
        std::map<std::string, uint64_t> familyActive;
        std::ofstream activeOutput;
        if (!activeFamilyOutput.empty())
        {
            activeOutput.open(activeFamilyOutput);
            if (!activeOutput) throw std::runtime_error("Could not open active-family output");
        }
        std::ifstream input(corpus);
        if (!input) throw std::runtime_error("Could not open corpus: " + corpus);
        std::size_t positions = 0, rejected = 0, parityMismatches = 0;
        std::string line;
        std::vector<int> features(parameterCount);
        bool firstLine = true;
        int fenCol = -1;
        while (positions < limit && std::getline(input, line))
        {
            if (line.empty()) continue;
            if (firstLine)
            {
                firstLine = false;
                if (line.find("fen") != std::string::npos || line.find("position_id") != std::string::npos)
                {
                    std::stringstream ss(line);
                    std::string col;
                    int idx = 0;
                    while (std::getline(ss, col, '\t'))
                    {
                        if (col == "fen") { fenCol = idx; break; }
                        idx++;
                    }
                    continue;
                }
            }
            std::string fen;
            if (fenCol >= 0)
            {
                std::stringstream ss(line);
                std::string col;
                int idx = 0;
                while (std::getline(ss, col, '\t'))
                {
                    if (idx == fenCol) { fen = col; break; }
                    idx++;
                }
            }
            else
            {
                const std::size_t tab = line.find('\t');
                fen = tab == std::string::npos ? line : line.substr(0, tab);
            }
            if (fen.empty()) continue;
            if (trainSplit && StableHash(fen) % 100 >= 70) continue;
            try
            {
                std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
                if (!board) { ++rejected; continue; }
                const int baselineScore = Tuner::TunerEvaluator::Evaluate(*board, baseline);
                if (EvaluationLogic::Evaluate(*board) != baselineScore) ++parityMismatches;
                if (activeOutput.is_open())
                {
                    bool active = false;
                    for (std::size_t i : families[activeFamilyName].indices)
                    {
                        const int zero = Tuner::TunerEvaluator::Evaluate(*board, zeroStates[i]);
                        const int one = Tuner::TunerEvaluator::Evaluate(*board, oneStates[i]);
                        active = active || one != zero;
                    }
                    if (active) activeOutput << fen << '\n';
                    ++positions;
                    continue;
                }
                for (std::size_t i = 0; i < parameterCount; ++i)
                {
                    const int zero = Tuner::TunerEvaluator::Evaluate(*board, zeroStates[i]);
                    const int one = Tuner::TunerEvaluator::Evaluate(*board, oneStates[i]);
                    const int two = Tuner::TunerEvaluator::Evaluate(*board, twoStates[i]);
                    const int feature = one - zero;
                    features[i] = feature;
                    Stats& s = stats[i];
                    s.sum += feature; s.sumSquares += static_cast<long double>(feature) * feature;
                    s.minimum = std::min(s.minimum, feature); s.maximum = std::max(s.maximum, feature);
                    if (feature > 0) { ++s.positive; ++s.nonzero; }
                    else if (feature < 0) { ++s.negative; ++s.nonzero; }
                    else ++s.zero;
                    if (two - one != feature ||
                        baselineScore - zero != registry[i].currentValue * feature)
                        ++s.nonlinearPositions;
                }
                for (auto& familyItem : families)
                {
                    FamilyData& family = familyItem.second;
                    const std::size_t n = family.indices.size();
                    bool active = false;
                    for (std::size_t i = 0; i < n; ++i)
                    {
                        active = active || features[family.indices[i]] != 0;
                        for (std::size_t j = i; j < n; ++j)
                        {
                            const long double product = static_cast<long double>(features[family.indices[i]]) *
                                                        features[family.indices[j]];
                            family.crossProducts[i * n + j] += product;
                            if (i != j) family.crossProducts[j * n + i] += product;
                        }
                    }
                    familyActive[familyItem.first] += active;
                }
                ++positions;
            }
            catch (...) { ++rejected; }
        }
        if (positions == 0) throw std::runtime_error("No valid corpus positions");
        if (activeOutput.is_open())
        {
            std::cout << "positions=" << positions << " rejected=" << rejected
                      << " parity_mismatches=" << parityMismatches << '\n';
            CleanupEngine();
            return parityMismatches == 0 ? 0 : 1;
        }
        std::filesystem::create_directories(outputDirectory);
        std::ofstream inventory(outputDirectory / "parameter-inventory.tsv");
        std::ofstream parameterReport(outputDirectory / "parameter-statistics.tsv");
        std::ofstream correlations(outputDirectory / "family-correlations.tsv");
        std::ofstream familyReport(outputDirectory / "family-summary.tsv");
        std::ofstream activationReport(outputDirectory / "family-activation.tsv");
        activationReport << "family\tactive_positions\tactive_fraction\n";
        for (const auto& item : familyActive)
            activationReport << item.first << '\t' << item.second << '\t'
                             << std::setprecision(12) << static_cast<double>(item.second) / positions << '\n';
        inventory << "name\tfamily\tcurrent_value\tphase\tshape\tsource_location\n";
        parameterReport << "name\tfamily\tcurrent_value\tlinear_exact\tnonlinear_positions\tnonzero_positions\tnonzero_fraction\tmean\tstandard_deviation\tminimum\tmaximum\tpositive\tnegative\tzero\tlow_support\n";
        std::vector<double> means(parameterCount), deviations(parameterCount);
        std::vector<bool> lowSupport(parameterCount);
        std::size_t exactLinear = 0;
        for (std::size_t i = 0; i < parameterCount; ++i)
        {
            const auto& p = registry[i]; const Stats& s = stats[i];
            const double mean = static_cast<double>(s.sum / positions);
            const double variance = std::max(0.0, static_cast<double>(s.sumSquares / positions) - mean * mean);
            const double deviation = std::sqrt(variance);
            means[i] = mean; deviations[i] = deviation;
            lowSupport[i] = s.nonzero < LowOccurrenceCount ||
                static_cast<double>(s.nonzero) / positions < LowOccurrenceFraction ||
                deviation <= NearZeroStandardDeviation;
            exactLinear += s.nonlinearPositions == 0;
            inventory << p.name << '\t' << FamilyName(p.family) << '\t' << p.currentValue << '\t'
                      << PhaseName(p.name) << '\t' << ShapeName(p.family, p.name)
                      << "\tOption.cpp; tuner/TunerParameter.h\n";
            parameterReport << std::setprecision(12) << p.name << '\t' << FamilyName(p.family) << '\t'
                << p.currentValue << '\t' << (s.nonlinearPositions == 0 ? "yes" : "no") << '\t'
                << s.nonlinearPositions << '\t' << s.nonzero << '\t'
                << static_cast<double>(s.nonzero) / positions << '\t' << mean << '\t' << deviation << '\t'
                << s.minimum << '\t' << s.maximum << '\t' << s.positive << '\t' << s.negative << '\t'
                << s.zero << '\t' << (lowSupport[i] ? "yes" : "no") << '\n';
        }

        correlations << "family\tparameter_a\tparameter_b\tcorrelation\tthreshold_band\n";
        familyReport << "family\tparameter_count\teffective_rank\tcondition_number\tlow_support_count\tstrongest_absolute_correlation\tge_0_90_pairs\tge_0_97_pairs\tge_0_995_pairs\tclassification\n";
        for (const auto& familyItem : families)
        {
            const std::string& name = familyItem.first; const FamilyData& family = familyItem.second;
            const int n = static_cast<int>(family.indices.size());
            std::vector<double> correlation(n * n, 0.0);
            double strongest = 0.0; int c90 = 0, c97 = 0, c995 = 0, low = 0;
            for (int i = 0; i < n; ++i)
            {
                const std::size_t globalI = family.indices[i]; low += lowSupport[globalI];
                correlation[i * n + i] = deviations[globalI] > NearZeroStandardDeviation ? 1.0 : 0.0;
                for (int j = i + 1; j < n; ++j)
                {
                    const std::size_t globalJ = family.indices[j];
                    double value = 0.0;
                    if (deviations[globalI] > NearZeroStandardDeviation && deviations[globalJ] > NearZeroStandardDeviation)
                    {
                        const double cross = static_cast<double>(family.crossProducts[i * n + j] / positions);
                        value = (cross - means[globalI] * means[globalJ]) /
                                (deviations[globalI] * deviations[globalJ]);
                        value = std::clamp(value, -1.0, 1.0);
                    }
                    correlation[i * n + j] = correlation[j * n + i] = value;
                    const double absolute = std::abs(value); strongest = std::max(strongest, absolute);
                    if (absolute >= 0.90) ++c90; if (absolute >= 0.97) ++c97; if (absolute >= 0.995) ++c995;
                    if (absolute >= 0.90)
                        correlations << name << '\t' << registry[globalI].name << '\t'
                                     << registry[globalJ].name << '\t' << std::setprecision(12) << value << '\t'
                                     << (absolute >= 0.995 ? ">=0.995" : absolute >= 0.97 ? ">=0.97" : ">=0.90") << '\n';
                }
            }
            const std::vector<double> eigenvalues = JacobiEigenvalues(correlation, n);
            const double largest = eigenvalues.empty() ? 0.0 : eigenvalues.back();
            int rank = 0; double smallestPositive = std::numeric_limits<double>::infinity();
            for (double value : eigenvalues)
                if (value > largest * EffectiveRankRelativeTolerance)
                { ++rank; smallestPositive = std::min(smallestPositive, value); }
            const double condition = rank == n && smallestPositive > 0.0
                ? std::sqrt(largest / smallestPositive) : std::numeric_limits<double>::infinity();
            std::string classification = "well identified";
            if (rank * 2 < n || low * 2 >= n) classification = "poorly identified";
            else if (rank < n || low > 0 || strongest >= 0.97) classification = "partially identified";
            familyReport << name << '\t' << n << '\t' << rank << '\t';
            if (std::isfinite(condition)) familyReport << std::setprecision(12) << condition;
            else familyReport << "rank_deficient";
            familyReport << '\t' << low << '\t' << strongest << '\t' << c90 << '\t' << c97 << '\t'
                         << c995 << '\t' << classification << '\n';
        }
        std::ofstream metadata(outputDirectory / "analysis-metadata.tsv");
        metadata << "positions\t" << positions << '\n' << "rejected\t" << rejected << '\n'
                 << "parameters\t" << parameterCount << '\n' << "exact_linear_parameters\t" << exactLinear << '\n'
                 << "nonlinear_or_conditional_parameters\t" << parameterCount - exactLinear << '\n'
                 << "production_tuner_parity_mismatches\t" << parityMismatches << '\n'
                 << "low_occurrence_count_threshold\t" << LowOccurrenceCount << '\n'
                 << "low_occurrence_fraction_threshold\t" << LowOccurrenceFraction << '\n'
                 << "near_zero_standard_deviation_threshold\t" << NearZeroStandardDeviation << '\n'
                 << "effective_rank_relative_tolerance\t" << EffectiveRankRelativeTolerance << '\n';
        std::cout << "positions=" << positions << " parameters=" << parameterCount
                  << " exact_linear=" << exactLinear
                  << " nonlinear_or_conditional=" << parameterCount - exactLinear
                  << " parity_mismatches=" << parityMismatches << '\n';
        CleanupEngine();
        return parityMismatches == 0 ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n'; return 1;
    }
}
