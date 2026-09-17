#ifndef HOWL_TUNER_PARAMETER_H
#define HOWL_TUNER_PARAMETER_H

#include <string>
#include <vector>
#include "Option.h"
#include "PieceSquareModel.h"

namespace Tuner
{

enum class ParameterFamily
{
    PieceValue,
    PawnStructure,
    PassedPawnV2,
    PieceSquare,
    CenterPresence,
    CenterMove,
    KingSafety,
    KnightMobility,
    BishopMobility,
    RookMobility,
    QueenMobility,
    Attack,
    Inline,
    RookFile,
    KnightOutpost,
    IsolatedPawn,
    RookBehindPassedPawn,
    EndgameWeights,

    // Conceptual selection groups. Registry entries retain their low-level family.
    BaseScalars,
    Pawns,
    Pieces,
    Threats,
    Endgame,
    King,
    PST
};

struct TunerParameter
{
    std::string name;
    ParameterFamily family;
    int semanticIndex;
    int currentValue;
    int minValue;
    int maxValue;
};

class TunerRegistry
{
public:
    static TunerRegistry CreateRegistry()
    {
        TunerRegistry registry;

        // 1. PieceValue (5 parameters)
        registry.Add("PawnValue", ParameterFamily::PieceValue, 0, Option::PawnValue);
        registry.Add("KnightValue", ParameterFamily::PieceValue, 1, Option::KnightValue);
        registry.Add("BishopValue", ParameterFamily::PieceValue, 2, Option::BishopValue);
        registry.Add("RookValue", ParameterFamily::PieceValue, 3, Option::RookValue);
        registry.Add("QueenValue", ParameterFamily::PieceValue, 4, Option::QueenValue);

        // 2. PawnStructure (1 parameter)
        registry.Add("DoubledPawnValue", ParameterFamily::PawnStructure, 0, Option::DoubledPawnValue);

        // 3. PassedPawnV2 (13 parameters: 6 MG ranks, 6 EG ranks, 1 MG file amplitude)
        registry.Add("PassedPawnMiddleGameBase", ParameterFamily::PassedPawnV2, 0,
                     Option::PassedPawnMiddleGameParameters[0]);
        for (int i = 1; i < 6; ++i)
            registry.Add("PassedPawnMiddleGameIncrement_" + std::to_string(i),
                         ParameterFamily::PassedPawnV2, i,
                         Option::PassedPawnMiddleGameParameters[i]);
        registry.Add("PassedPawnEndGameBase", ParameterFamily::PassedPawnV2, 6,
                     Option::PassedPawnEndGameParameters[0]);
        for (int i = 1; i < 6; ++i)
            registry.Add("PassedPawnEndGameIncrement_" + std::to_string(i),
                         ParameterFamily::PassedPawnV2, 6 + i,
                         Option::PassedPawnEndGameParameters[i]);
        registry.Add("PassedPawnMiddleGameFileAmplitude", ParameterFamily::PassedPawnV2, 12,
                     Option::PassedPawnMiddleGameFileAmplitude);

        // 4. Compact PieceSquare model (96 parameters)
        struct PstBinding
        {
            const char* prefix;
            const int* parameters;
            int count;
        };

        const PstBinding pstTables[12] = {
            {"PawnPieceSquareMiddleGame_", Option::PawnPieceSquareMiddleGameParameters, PieceSquareModel::PawnParameterCount},
            {"KnightPieceSquareMiddleGame_", Option::KnightPieceSquareMiddleGameParameters, PieceSquareModel::MinorParameterCount},
            {"BishopPieceSquareMiddleGame_", Option::BishopPieceSquareMiddleGameParameters, PieceSquareModel::MinorParameterCount},
            {"RookPieceSquareMiddleGame_", Option::RookPieceSquareMiddleGameParameters, PieceSquareModel::MajorParameterCount},
            {"QueenPieceSquareMiddleGame_", Option::QueenPieceSquareMiddleGameParameters, PieceSquareModel::MajorParameterCount},
            {"KingPieceSquareMiddleGame_", Option::KingPieceSquareMiddleGameParameters, PieceSquareModel::KingParameterCount},
            {"PawnPieceSquareEndGame_", Option::PawnPieceSquareEndGameParameters, PieceSquareModel::PawnParameterCount},
            {"KnightPieceSquareEndGame_", Option::KnightPieceSquareEndGameParameters, PieceSquareModel::MinorParameterCount},
            {"BishopPieceSquareEndGame_", Option::BishopPieceSquareEndGameParameters, PieceSquareModel::MinorParameterCount},
            {"RookPieceSquareEndGame_", Option::RookPieceSquareEndGameParameters, PieceSquareModel::MajorParameterCount},
            {"QueenPieceSquareEndGame_", Option::QueenPieceSquareEndGameParameters, PieceSquareModel::MajorParameterCount},
            {"KingPieceSquareEndGame_", Option::KingPieceSquareEndGameParameters, PieceSquareModel::KingParameterCount}
        };

        int pstSemanticIndex = 0;
        for (const auto& binding : pstTables)
        {
            for (int index = 0; index < binding.count; ++index)
            {
                registry.Add(binding.prefix + std::to_string(index),
                             ParameterFamily::PieceSquare, pstSemanticIndex++,
                             binding.parameters[index]);
            }
        }

        // 5. Mobility v2 (38 structural parameters: base anchor plus non-negative increments)
        const auto addMobilityGroup = [&registry](ParameterFamily family,
                                                   const char* piece,
                                                   const int* mgParameters,
                                                   const int* egParameters,
                                                   int parameterCount)
        {
            int semanticIndex = 0;
            registry.Add(std::string(piece) + "MobilityMiddleGameBase", family,
                         semanticIndex++, mgParameters[0]);
            for (int i = 1; i < parameterCount; ++i)
                registry.Add(std::string(piece) + "MobilityMiddleGameIncrement_" + std::to_string(i),
                             family, semanticIndex++, mgParameters[i]);
            registry.Add(std::string(piece) + "MobilityEndGameBase", family,
                         semanticIndex++, egParameters[0]);
            for (int i = 1; i < parameterCount; ++i)
                registry.Add(std::string(piece) + "MobilityEndGameIncrement_" + std::to_string(i),
                             family, semanticIndex++, egParameters[i]);
        };

        addMobilityGroup(ParameterFamily::KnightMobility, "Knight",
                         Option::KnightMobilityMiddleGameParameters,
                         Option::KnightMobilityEndGameParameters, 4);
        addMobilityGroup(ParameterFamily::BishopMobility, "Bishop",
                         Option::BishopMobilityMiddleGameParameters,
                         Option::BishopMobilityEndGameParameters, 5);
        addMobilityGroup(ParameterFamily::RookMobility, "Rook",
                         Option::RookMobilityMiddleGameParameters,
                         Option::RookMobilityEndGameParameters, 5);
        addMobilityGroup(ParameterFamily::QueenMobility, "Queen",
                         Option::QueenMobilityMiddleGameParameters,
                         Option::QueenMobilityEndGameParameters, 5);

        // 9. Attack (60 parameters: 6 attackers * 5 victims * 2 phases)
        // Victim piece IDs: 1 (Pawn), 2 (Knight), 3 (Bishop), 4 (Rook), 5 (Queen)
        // Victims 9..13 (Black) are semantic duplicates of 1..5 (White) and are excluded
        struct AttackBinding
        {
            const char* attackerName;
            const int* mgTable;
            const int* egTable;
        };

        const AttackBinding attackTables[6] = {
            {"Pawn", Option::PawnAttackValueMiddleGame, Option::PawnAttackValueEndGame},
            {"Knight", Option::KnightAttackValueMiddleGame, Option::KnightAttackValueEndGame},
            {"Bishop", Option::BishopAttackValueMiddleGame, Option::BishopAttackValueEndGame},
            {"Rook", Option::RookAttackValueMiddleGame, Option::RookAttackValueEndGame},
            {"Queen", Option::QueenAttackValueMiddleGame, Option::QueenAttackValueEndGame},
            {"King", Option::KingAttackValueMiddleGame, Option::KingAttackValueEndGame}
        };

        const char* const victimNames[5] = {"Pawn", "Knight", "Bishop", "Rook", "Queen"};
        const int victimPieceIds[5] = {1, 2, 3, 4, 5};

        int attackSemanticIndex = 0;
        for (const auto& binding : attackTables)
        {
            for (int v = 0; v < 5; ++v)
            {
                int victimId = victimPieceIds[v];
                registry.Add(std::string(binding.attackerName) + "Attack" + victimNames[v] + "_MiddleGame",
                             ParameterFamily::Attack, attackSemanticIndex++,
                             binding.mgTable[victimId]);
            }
            for (int v = 0; v < 5; ++v)
            {
                int victimId = victimPieceIds[v];
                registry.Add(std::string(binding.attackerName) + "Attack" + victimNames[v] + "_EndGame",
                             ParameterFamily::Attack, attackSemanticIndex++,
                             binding.egTable[victimId]);
            }
        }

        // 10. Inline (11 parameters)
        registry.Add("BishopPairValue", ParameterFamily::Inline, 0, Option::BishopPairValue);
        registry.Add("BishopOpenFilePawnScale", ParameterFamily::Inline, 1, Option::BishopOpenFilePawnScale);
        registry.Add("TempoMiddleGame", ParameterFamily::Inline, 2, Option::TempoMiddleGame);
        registry.Add("TempoEndGame", ParameterFamily::Inline, 3, Option::TempoEndGame);
        registry.Add("OppositeColorBishopMiddleGameScalePermille", ParameterFamily::Inline, 4, Option::OppositeColorBishopMiddleGameScalePermille);
        registry.Add("OppositeColorBishopEndGameScalePermille", ParameterFamily::Inline, 5, Option::OppositeColorBishopEndGameScalePermille);
        registry.Add("MaterialBalanceOffset", ParameterFamily::Inline, 6, Option::MaterialBalanceOffset);
        registry.Add("PawnDeficitZeroPawnMultiplierPermille", ParameterFamily::Inline, 7, Option::PawnDeficitZeroPawnMultiplierPermille);
        registry.Add("PawnDeficitOnePawnMultiplierPermille", ParameterFamily::Inline, 8, Option::PawnDeficitOnePawnMultiplierPermille);
        registry.Add("EndgamePawnAdvancementRankMultiplier", ParameterFamily::Inline, 9, Option::EndgamePawnAdvancementRankMultiplier);
        registry.Add("PieceAttackScalePercent", ParameterFamily::Inline, 10, Option::PieceAttackScalePercent);

        // 11. RookFile (4 parameters)
        registry.Add("RookOpenFileMiddleGame", ParameterFamily::RookFile, 0, Option::RookOpenFileMiddleGame);
        registry.Add("RookOpenFileEndGame", ParameterFamily::RookFile, 1, Option::RookOpenFileEndGame);
        registry.Add("RookSemiOpenFileMiddleGame", ParameterFamily::RookFile, 2, Option::RookSemiOpenFileMiddleGame);
        registry.Add("RookSemiOpenFileEndGame", ParameterFamily::RookFile, 3, Option::RookSemiOpenFileEndGame);

        // 12. KnightOutpost (4 parameters)
        registry.Add("KnightOutpostMiddleGame", ParameterFamily::KnightOutpost, 0, Option::KnightOutpostMiddleGame);
        registry.Add("KnightOutpostEndGame", ParameterFamily::KnightOutpost, 1, Option::KnightOutpostEndGame);
        registry.Add("KnightSupportedOutpostMiddleGame", ParameterFamily::KnightOutpost, 2, Option::KnightSupportedOutpostMiddleGame);
        registry.Add("KnightSupportedOutpostEndGame", ParameterFamily::KnightOutpost, 3, Option::KnightSupportedOutpostEndGame);

        // 13. IsolatedPawn (2 parameters)
        registry.Add("IsolatedPawnMiddleGame", ParameterFamily::IsolatedPawn, 0, Option::IsolatedPawnMiddleGame);
        registry.Add("IsolatedPawnEndGame", ParameterFamily::IsolatedPawn, 1, Option::IsolatedPawnEndGame);

        // 14. RookBehindPassedPawn (2 parameters)
        registry.Add("RookBehindPassedPawnMiddleGame", ParameterFamily::RookBehindPassedPawn, 0, Option::RookBehindPassedPawnMiddleGame);
        registry.Add("RookBehindPassedPawnEndGame", ParameterFamily::RookBehindPassedPawn, 1, Option::RookBehindPassedPawnEndGame);

        registry.Add("LoneKingBase", ParameterFamily::EndgameWeights, 0, Option::LoneKingBase);
        registry.Add("LoneKingEdgeWeight", ParameterFamily::EndgameWeights, 1, Option::LoneKingEdgeWeight);
        registry.Add("LoneKingCornerWeight", ParameterFamily::EndgameWeights, 2, Option::LoneKingCornerWeight);
        registry.Add("LoneKingConfinementWeight", ParameterFamily::EndgameWeights, 3, Option::LoneKingConfinementWeight);
        registry.Add("LoneKingRestrictedNeighbourWeight", ParameterFamily::EndgameWeights, 4, Option::LoneKingRestrictedNeighbourWeight);
        registry.Add("LowMaterialScalePermille", ParameterFamily::EndgameWeights, 5, Option::LowMaterialScalePermille);

        const auto addKing = [&registry](const char* name, int index, int value)
        {
            registry.Add(name, ParameterFamily::KingSafety, index, value, 0, 10000);
        };
        addKing("KingAttackerPawnWeight", 0, Option::KingAttackerPawnWeight);
        addKing("KingAttackerMinorWeight", 1, Option::KingAttackerMinorWeight);
        addKing("KingAttackerRookWeight", 2, Option::KingAttackerRookWeight);
        addKing("KingAttackerQueenWeight", 3, Option::KingAttackerQueenWeight);
        addKing("KingDefenderPawnWeight", 4, Option::KingDefenderPawnWeight);
        addKing("KingDefenderMinorWeight", 5, Option::KingDefenderMinorWeight);
        addKing("KingDefenderRookWeight", 6, Option::KingDefenderRookWeight);
        addKing("KingDefenderQueenWeight", 7, Option::KingDefenderQueenWeight);
        addKing("KingShelterSecondRankDanger", 8, Option::KingShelterSecondRankDanger);
        addKing("KingShelterAdvancedPawnDanger", 9, Option::KingShelterAdvancedPawnDanger);
        addKing("KingShelterMissingPawnDanger", 10, Option::KingShelterMissingPawnDanger);
        addKing("KingShelterOpenFileDanger", 11, Option::KingShelterOpenFileDanger);
        addKing("KingUndefendedZoneDanger", 12, Option::KingUndefendedZoneDanger);
        addKing("KingAdditionalZoneAttackerDanger", 13, Option::KingAdditionalZoneAttackerDanger);
        addKing("KingSemiOpenLineDanger", 14, Option::KingSemiOpenLineDanger);
        addKing("KingOpenLineDanger", 15, Option::KingOpenLineDanger);
        addKing("KingDiagonalLineDanger", 16, Option::KingDiagonalLineDanger);
        addKing("KingControlledEscapeDanger", 17, Option::KingControlledEscapeDanger);
        addKing("KingBlockedEscapeDanger", 18, Option::KingBlockedEscapeDanger);
        addKing("KingTrappedEscapeDanger", 19, Option::KingTrappedEscapeDanger);
        addKing("KingHeavyBatteryDanger", 20, Option::KingHeavyBatteryDanger);
        addKing("CentralKingInnerMinorPressure", 21, Option::CentralKingInnerMinorPressure);
        addKing("CentralKingOuterMinorPressure", 22, Option::CentralKingOuterMinorPressure);
        addKing("CentralKingReadinessLagWeight", 23, Option::CentralKingReadinessLagWeight);
        addKing("CentralKingPressureScale", 24, Option::CentralKingPressureScale);
        addKing("KingUnreadyCoordinationWeight", 25, Option::KingUnreadyCoordinationWeight);
        addKing("KingLatentActivationWeight", 26, Option::KingLatentActivationWeight);
        addKing("KingFutureShelterWingWeight", 27, Option::KingFutureShelterWingWeight);
        addKing("KingPinnedShelterPawnWeight", 28, Option::KingPinnedShelterPawnWeight);
        addKing("KingInfiltratedQueenWeight", 29, Option::KingInfiltratedQueenWeight);

        return registry;
    }

    void Add(const TunerParameter& param)
    {
        parameters.push_back(param);
    }

    void Add(std::string name, ParameterFamily family, int semanticIndex, int currentValue)
    {
        parameters.push_back({std::move(name), family, semanticIndex, currentValue, currentValue, currentValue});
    }

    void Add(std::string name, ParameterFamily family, int semanticIndex, int currentValue,
             int minValue, int maxValue)
    {
        parameters.push_back({std::move(name), family, semanticIndex,
                              currentValue, minValue, maxValue});
    }

    const std::vector<TunerParameter>& GetParameters() const
    {
        return parameters;
    }

    std::size_t Size() const
    {
        return parameters.size();
    }

    const TunerParameter& operator[](std::size_t index) const
    {
        return parameters[index];
    }

private:
    std::vector<TunerParameter> parameters;
};

} // namespace Tuner

#endif // HOWL_TUNER_PARAMETER_H
