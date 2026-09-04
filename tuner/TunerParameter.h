#ifndef HOWL_TUNER_PARAMETER_H
#define HOWL_TUNER_PARAMETER_H

#include <string>
#include <vector>
#include "Option.h"

namespace Tuner
{

enum class ParameterFamily
{
    PieceValue,
    PawnStructure,
    PassedPawn,
    PieceSquare,
    CenterPresence,
    CenterMove,
    KingSafety,
    Mobility,
    Attack,
    Inline
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

        // 3. PassedPawn (128 parameters: 64 MG + 64 EG)
        for (int sq = 0; sq < 64; ++sq)
        {
            registry.Add("WhitePassedPawnValueMiddleGam_" + std::to_string(sq),
                         ParameterFamily::PassedPawn, sq,
                         Option::WhitePassedPawnValueMiddleGam[sq]);
        }
        for (int sq = 0; sq < 64; ++sq)
        {
            registry.Add("WhitePassedPawnValueEndGame_" + std::to_string(sq),
                         ParameterFamily::PassedPawn, 64 + sq,
                         Option::WhitePassedPawnValueEndGame[sq]);
        }

        // 4. PieceSquare (768 parameters: 6 pieces * 2 phases * 64 squares)
        struct PstBinding
        {
            const char* prefix;
            const int* table;
        };

        const PstBinding pstTables[12] = {
            {"PawnInValueWhiteMiddleGame_", Option::PawnInValueWhiteMiddleGame},
            {"KnightInValueWhiteMiddleGame_", Option::KnightInValueWhiteMiddleGame},
            {"BishopInValueWhiteMiddleGame_", Option::BishopInValueWhiteMiddleGame},
            {"RookInValueWhiteMiddleGame_", Option::RookInValueWhiteMiddleGame},
            {"QueenInValueWhiteMiddleGame_", Option::QueenInValueWhiteMiddleGame},
            {"KingInValueWhiteMiddleGame_", Option::KingInValueWhiteMiddleGame},
            {"PawnInValueWhiteEndGame_", Option::PawnInValueWhiteEndGame},
            {"KnightInValueWhiteEndGame_", Option::KnightInValueWhiteEndGame},
            {"BishopInValueWhiteEndGame_", Option::BishopInValueWhiteEndGame},
            {"RookInValueWhiteEndGame_", Option::RookInValueWhiteEndGame},
            {"QueenInValueWhiteEndGame_", Option::QueenInValueWhiteEndGame},
            {"KingInValueWhiteEndGame_", Option::KingInValueWhiteEndGame}
        };

        int pstSemanticIndex = 0;
        for (const auto& binding : pstTables)
        {
            for (int sq = 0; sq < 64; ++sq)
            {
                registry.Add(binding.prefix + std::to_string(sq),
                             ParameterFamily::PieceSquare, pstSemanticIndex++,
                             binding.table[sq]);
            }
        }

        // 5. CenterPresence (384 parameters: 6 pieces * 64 squares)
        const PstBinding centerPresenceTables[6] = {
            {"PawnInCenterValueWhite_", Option::PawnInCenterValueWhite},
            {"KnightInCenterValueWhite_", Option::KnightInCenterValueWhite},
            {"BishopInCenterValueWhite_", Option::BishopInCenterValueWhite},
            {"RookInCenterValueWhite_", Option::RookInCenterValueWhite},
            {"QueenInCenterValueWhite_", Option::QueenInCenterValueWhite},
            {"KingInCenterValueWhite_", Option::KingInCenterValueWhite}
        };

        int centerPresenceSemanticIndex = 0;
        for (const auto& binding : centerPresenceTables)
        {
            for (int sq = 0; sq < 64; ++sq)
            {
                registry.Add(binding.prefix + std::to_string(sq),
                             ParameterFamily::CenterPresence, centerPresenceSemanticIndex++,
                             binding.table[sq]);
            }
        }

        // 6. CenterMove (384 parameters: 6 pieces * 64 squares)
        const PstBinding centerMoveTables[6] = {
            {"PawnMoveCenterValueWhite_", Option::PawnMoveCenterValueWhite},
            {"KnightMoveCenterValueWhite_", Option::KnightMoveCenterValueWhite},
            {"BishopMoveCenterValueWhite_", Option::BishopMoveCenterValueWhite},
            {"RookMoveCenterValueWhite_", Option::RookMoveCenterValueWhite},
            {"QueenMoveCenterValueWhite_", Option::QueenMoveCenterValueWhite},
            {"KingMoveCenterValueWhite_", Option::KingMoveCenterValueWhite}
        };

        int centerMoveSemanticIndex = 0;
        for (const auto& binding : centerMoveTables)
        {
            for (int sq = 0; sq < 64; ++sq)
            {
                registry.Add(binding.prefix + std::to_string(sq),
                             ParameterFamily::CenterMove, centerMoveSemanticIndex++,
                             binding.table[sq]);
            }
        }

        // 7. KingSafety (64 parameters)
        for (int sq = 0; sq < 64; ++sq)
        {
            registry.Add("WhiteKingPlaceSafetyMiddleGame_" + std::to_string(sq),
                         ParameterFamily::KingSafety, sq,
                         Option::WhiteKingPlaceSafetyMiddleGame[sq]);
        }

        // 8. Mobility (156 parameters: MG & EG for P(3), N(9), B(14), R(15), Q(28), K(9))
        struct MobilityBinding
        {
            const char* namePrefix;
            const int* mgTable;
            const int* egTable;
            int count;
        };

        const MobilityBinding mobilityTables[6] = {
            {"PawnMoveCountValue_", Option::PawnMoveCountValueMiddleGame, Option::PawnMoveCountValueEndGame, 3},
            {"KnightMoveCountValue_", Option::KnightMoveCountValueMiddleGame, Option::KnightMoveCountValueEndGame, 9},
            {"BishopMoveCountValue_", Option::BishopMoveCountValueMiddleGame, Option::BishopMoveCountValueEndGame, 14},
            {"RookMoveCountValue_", Option::RookMoveCountValueMiddleGame, Option::RookMoveCountValueEndGame, 15},
            {"QueenMoveCountValue_", Option::QueenMoveCountValueMiddleGame, Option::QueenMoveCountValueEndGame, 28},
            {"KingMoveCountValue_", Option::KingMoveCountValueMiddleGame, Option::KingMoveCountValueEndGame, 9}
        };

        int mobilitySemanticIndex = 0;
        for (const auto& binding : mobilityTables)
        {
            for (int i = 0; i < binding.count; ++i)
            {
                registry.Add(std::string(binding.namePrefix) + "MiddleGame_" + std::to_string(i),
                             ParameterFamily::Mobility, mobilitySemanticIndex++,
                             binding.mgTable[i]);
            }
            for (int i = 0; i < binding.count; ++i)
            {
                registry.Add(std::string(binding.namePrefix) + "EndGame_" + std::to_string(i),
                             ParameterFamily::Mobility, mobilitySemanticIndex++,
                             binding.egTable[i]);
            }
        }

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

        // 10. Inline (10 parameters)
        registry.Add("BishopPairValue", ParameterFamily::Inline, 0, 50);
        registry.Add("BishopOpenFilePawnScale", ParameterFamily::Inline, 1, 2);
        registry.Add("TempoMiddleGame", ParameterFamily::Inline, 2, 24);
        registry.Add("TempoEndGame", ParameterFamily::Inline, 3, 11);
        registry.Add("OppositeColorBishopMiddleGameScalePermille", ParameterFamily::Inline, 4, 900); // 0.9 * 1000
        registry.Add("OppositeColorBishopEndGameScalePermille", ParameterFamily::Inline, 5, 750);   // 0.75 * 1000
        registry.Add("MaterialBalanceOffset", ParameterFamily::Inline, 6, 1500);
        registry.Add("PawnDeficitZeroPawnMultiplierPermille", ParameterFamily::Inline, 7, 700);    // 0.7 * 1000
        registry.Add("PawnDeficitOnePawnMultiplierPermille", ParameterFamily::Inline, 8, 900);     // 0.9 * 1000
        registry.Add("EndgamePawnAdvancementRankMultiplier", ParameterFamily::Inline, 9, 2);

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
