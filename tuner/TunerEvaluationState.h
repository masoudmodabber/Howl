#ifndef HOWL_TUNER_EVALUATION_STATE_H
#define HOWL_TUNER_EVALUATION_STATE_H

#include <cstddef>
#include <unordered_set>
#include <vector>
#include "MobilityV2.h"
#include "PassedPawnV2.h"
#include "PieceSquareModel.h"
#include "tuner/TunerParameter.h"

namespace Tuner
{

struct TunerEvaluationState
{
    // =========================================================================
    // 1. Authored Candidate Evaluation Storage (mapped from TunerRegistry)
    // =========================================================================

    // Family: PieceValue (5)
    int PawnValue = 0;
    int KnightValue = 0;
    int BishopValue = 0;
    int RookValue = 0;
    int QueenValue = 0;

    // Family: PawnStructure (1)
    int DoubledPawnValue = 0;

    // Family: IsolatedPawn (2 parameters)
    int IsolatedPawnMiddleGame = 0;
    int IsolatedPawnEndGame = 0;

    // Family: RookBehindPassedPawn (2 parameters)
    int RookBehindPassedPawnMiddleGame = 0;
    int RookBehindPassedPawnEndGame = 0;

    // Family: PassedPawnV2 (6 MG ranks, 6 EG ranks, 1 MG file amplitude)
    int PassedPawnMiddleGameParameters[6] = {0};
    int PassedPawnEndGameParameters[6] = {0};
    int PassedPawnMiddleGameFileAmplitude = 0;

    // Generated passed-pawn lookup tables consumed by the evaluator.
    int WhitePassedPawnValueMiddleGam[64] = {0};
    int WhitePassedPawnValueEndGame[64] = {0};

    // Family: compact PieceSquare model (96 parameters plus generated tables)
    int PawnPieceSquareMiddleGameParameters[7] = {0};
    int KnightPieceSquareMiddleGameParameters[11] = {0};
    int BishopPieceSquareMiddleGameParameters[11] = {0};
    int RookPieceSquareMiddleGameParameters[6] = {0};
    int QueenPieceSquareMiddleGameParameters[6] = {0};
    int KingPieceSquareMiddleGameParameters[7] = {0};
    int PawnPieceSquareEndGameParameters[7] = {0};
    int KnightPieceSquareEndGameParameters[11] = {0};
    int BishopPieceSquareEndGameParameters[11] = {0};
    int RookPieceSquareEndGameParameters[6] = {0};
    int QueenPieceSquareEndGameParameters[6] = {0};
    int KingPieceSquareEndGameParameters[7] = {0};

    int PawnInValueWhiteMiddleGame[64] = {0};
    int KnightInValueWhiteMiddleGame[64] = {0};
    int BishopInValueWhiteMiddleGame[64] = {0};
    int RookInValueWhiteMiddleGame[64] = {0};
    int QueenInValueWhiteMiddleGame[64] = {0};
    int KingInValueWhiteMiddleGame[64] = {0};

    int PawnInValueWhiteEndGame[64] = {0};
    int KnightInValueWhiteEndGame[64] = {0};
    int BishopInValueWhiteEndGame[64] = {0};
    int RookInValueWhiteEndGame[64] = {0};
    int QueenInValueWhiteEndGame[64] = {0};
    int KingInValueWhiteEndGame[64] = {0};

    // Families: Mobility v2 (base anchor plus non-negative increments)
    int KnightMobilityMiddleGameParameters[4] = {0};
    int KnightMobilityEndGameParameters[4] = {0};
    int BishopMobilityMiddleGameParameters[5] = {0};
    int BishopMobilityEndGameParameters[5] = {0};
    int RookMobilityMiddleGameParameters[5] = {0};
    int RookMobilityEndGameParameters[5] = {0};
    int QueenMobilityMiddleGameParameters[5] = {0};
    int QueenMobilityEndGameParameters[5] = {0};

    // Generated mobility bucket tables consumed by the evaluator.
    int PawnMoveCountValueMiddleGame[3] = {0};
    int PawnMoveCountValueEndGame[3] = {0};
    int KnightMoveCountValueMiddleGame[9] = {0};
    int KnightMoveCountValueEndGame[9] = {0};
    int BishopMoveCountValueMiddleGame[14] = {0};
    int BishopMoveCountValueEndGame[14] = {0};
    int RookMoveCountValueMiddleGame[15] = {0};
    int RookMoveCountValueEndGame[15] = {0};
    int QueenMoveCountValueMiddleGame[28] = {0};
    int QueenMoveCountValueEndGame[28] = {0};
    int KingMoveCountValueMiddleGame[9] = {0};
    int KingMoveCountValueEndGame[9] = {0};

    // Family: Attack (60 semantic values: 6 attackers * 5 victims * 2 phases)
    // Authored victim slots 1..5 in each attacker array
    int PawnAttackValueMiddleGame[16] = {0};
    int PawnAttackValueEndGame[16] = {0};
    int KnightAttackValueMiddleGame[16] = {0};
    int KnightAttackValueEndGame[16] = {0};
    int BishopAttackValueMiddleGame[16] = {0};
    int BishopAttackValueEndGame[16] = {0};
    int RookAttackValueMiddleGame[16] = {0};
    int RookAttackValueEndGame[16] = {0};
    int QueenAttackValueMiddleGame[16] = {0};
    int QueenAttackValueEndGame[16] = {0};
    int KingAttackValueMiddleGame[16] = {0};
    int KingAttackValueEndGame[16] = {0};

    // Family: Inline (11 parameters)
    int BishopPairValue = 0;
    int BishopOpenFilePawnScale = 0;
    int TempoMiddleGame = 0;
    int TempoEndGame = 0;
    int OppositeColorBishopMiddleGameScalePermille = 0;
    int OppositeColorBishopEndGameScalePermille = 0;
    int MaterialBalanceOffset = 0;
    int PawnDeficitZeroPawnMultiplierPermille = 0;
    int PawnDeficitOnePawnMultiplierPermille = 0;
    int EndgamePawnAdvancementRankMultiplier = 0;
    int PieceAttackScalePercent = 0;
    int LoneKingBase = 0;
    int LoneKingEdgeWeight = 0;
    int LoneKingCornerWeight = 0;
    int LoneKingConfinementWeight = 0;
    int LoneKingRestrictedNeighbourWeight = 0;
    int LowMaterialScalePermille = 0;
    int KingAttackerPawnWeight = 0;
    int KingAttackerMinorWeight = 0;
    int KingAttackerRookWeight = 0;
    int KingAttackerQueenWeight = 0;
    int KingDefenderPawnWeight = 0;
    int KingDefenderMinorWeight = 0;
    int KingDefenderRookWeight = 0;
    int KingDefenderQueenWeight = 0;
    int KingShelterSecondRankDanger = 0;
    int KingShelterAdvancedPawnDanger = 0;
    int KingShelterMissingPawnDanger = 0;
    int KingShelterOpenFileDanger = 0;
    int KingUndefendedZoneDanger = 0;
    int KingAdditionalZoneAttackerDanger = 0;
    int KingSemiOpenLineDanger = 0;
    int KingOpenLineDanger = 0;
    int KingDiagonalLineDanger = 0;
    int KingControlledEscapeDanger = 0;
    int KingBlockedEscapeDanger = 0;
    int KingTrappedEscapeDanger = 0;
    int KingHeavyBatteryDanger = 0;
    int CentralKingInnerMinorPressure = 0;
    int CentralKingOuterMinorPressure = 0;
    int CentralKingReadinessLagWeight = 0;
    int CentralKingPressureScale = 0;
    int KingUnreadyCoordinationWeight = 0;
    int KingLatentActivationWeight = 0;
    int KingFutureShelterWingWeight = 0;
    int KingPinnedShelterPawnWeight = 0;
    int KingInfiltratedQueenWeight = 0;

    // Family: KnightOutpost (4 parameters)
    int KnightOutpostMiddleGame = 0;
    int KnightOutpostEndGame = 0;
    int KnightSupportedOutpostMiddleGame = 0;
    int KnightSupportedOutpostEndGame = 0;

    // Family: RookFile (4 parameters)
    int RookOpenFileMiddleGame = 0;
    int RookOpenFileEndGame = 0;
    int RookSemiOpenFileMiddleGame = 0;
    int RookSemiOpenFileEndGame = 0;

    // =========================================================================
    // 2. Tuner-Side Derived Data: Black Mirrored Tables
    // =========================================================================
    int BlackPassedPawnValueMiddleGam[64] = {0};
    int BlackPassedPawnValueEndGam[64] = {0};

    int PawnInValueBlackMiddleGame[64] = {0};
    int KnightInValueBlackMiddleGame[64] = {0};
    int BishopInValueBlackMiddleGame[64] = {0};
    int RookInValueBlackMiddleGame[64] = {0};
    int QueenInValueBlackMiddleGame[64] = {0};
    int KingInValueBlackMiddleGame[64] = {0};

    int PawnInValueBlackEndGame[64] = {0};
    int KnightInValueBlackEndGame[64] = {0};
    int BishopInValueBlackEndGame[64] = {0};
    int RookInValueBlackEndGame[64] = {0};
    int QueenInValueBlackEndGame[64] = {0};
    int KingInValueBlackEndGame[64] = {0};

    // =========================================================================
    // 3. Tuner-Side Derived Data: Runtime Evaluation Layout Copies [3][...]
    //    ([0] = MiddleGame, [1] = Unused/0, [2] = EndGame)
    //    Attack tables also contain victim colour duplication (slots 9..13 <- 1..5)
    // =========================================================================
    int PawnInValueWhite[3][64] = {{0}};
    int KnightInValueWhite[3][64] = {{0}};
    int BishopInValueWhite[3][64] = {{0}};
    int RookInValueWhite[3][64] = {{0}};
    int QueenInValueWhite[3][64] = {{0}};
    int KingInValueWhite[3][64] = {{0}};

    int PawnInValueBlack[3][64] = {{0}};
    int KnightInValueBlack[3][64] = {{0}};
    int BishopInValueBlack[3][64] = {{0}};
    int RookInValueBlack[3][64] = {{0}};
    int QueenInValueBlack[3][64] = {{0}};
    int KingInValueBlack[3][64] = {{0}};

    int PawnMoveCountValue[3][3] = {{0}};
    int KnightMoveCountValue[3][9] = {{0}};
    int BishopMoveCountValue[3][14] = {{0}};
    int RookMoveCountValue[3][15] = {{0}};
    int QueenMoveCountValue[3][28] = {{0}};
    int KingMoveCountValue[3][9] = {{0}};

    int PawnAttackValue[3][16] = {{0}};
    int KnightAttackValue[3][16] = {{0}};
    int BishopAttackValue[3][16] = {{0}};
    int RookAttackValue[3][16] = {{0}};
    int QueenAttackValue[3][16] = {{0}};
    int KingAttackValue[3][16] = {{0}};

    // =========================================================================
    // Deterministic Mapping from Registry to Storage
    // =========================================================================
    int* GetParameterPointer(ParameterFamily family, int semanticIndex)
    {
        switch (family)
        {
        case ParameterFamily::PieceValue:
            switch (semanticIndex)
            {
            case 0: return &PawnValue;
            case 1: return &KnightValue;
            case 2: return &BishopValue;
            case 3: return &RookValue;
            case 4: return &QueenValue;
            default: return nullptr;
            }

        case ParameterFamily::PawnStructure:
            if (semanticIndex == 0) return &DoubledPawnValue;
            return nullptr;

        case ParameterFamily::PassedPawnV2:
            if (semanticIndex >= 0 && semanticIndex < 6)
                return &PassedPawnMiddleGameParameters[semanticIndex];
            if (semanticIndex >= 6 && semanticIndex < 12)
                return &PassedPawnEndGameParameters[semanticIndex - 6];
            if (semanticIndex == 12)
                return &PassedPawnMiddleGameFileAmplitude;
            return nullptr;

        case ParameterFamily::PieceSquare:
        {
            int* groups[12] = {
                PawnPieceSquareMiddleGameParameters, KnightPieceSquareMiddleGameParameters,
                BishopPieceSquareMiddleGameParameters, RookPieceSquareMiddleGameParameters,
                QueenPieceSquareMiddleGameParameters, KingPieceSquareMiddleGameParameters,
                PawnPieceSquareEndGameParameters, KnightPieceSquareEndGameParameters,
                BishopPieceSquareEndGameParameters, RookPieceSquareEndGameParameters,
                QueenPieceSquareEndGameParameters, KingPieceSquareEndGameParameters
            };
            const int counts[12] = {7, 11, 11, 6, 6, 7, 7, 11, 11, 6, 6, 7};
            if (semanticIndex < 0) return nullptr;
            for (int group = 0; group < 12; ++group)
            {
                if (semanticIndex < counts[group]) return &groups[group][semanticIndex];
                semanticIndex -= counts[group];
            }
            return nullptr;
        }

        case ParameterFamily::KnightMobility:
            if (semanticIndex >= 0 && semanticIndex < 4)
                return &KnightMobilityMiddleGameParameters[semanticIndex];
            if (semanticIndex >= 4 && semanticIndex < 8)
                return &KnightMobilityEndGameParameters[semanticIndex - 4];
            return nullptr;

        case ParameterFamily::BishopMobility:
            if (semanticIndex >= 0 && semanticIndex < 5)
                return &BishopMobilityMiddleGameParameters[semanticIndex];
            if (semanticIndex >= 5 && semanticIndex < 10)
                return &BishopMobilityEndGameParameters[semanticIndex - 5];
            return nullptr;

        case ParameterFamily::RookMobility:
            if (semanticIndex >= 0 && semanticIndex < 5)
                return &RookMobilityMiddleGameParameters[semanticIndex];
            if (semanticIndex >= 5 && semanticIndex < 10)
                return &RookMobilityEndGameParameters[semanticIndex - 5];
            return nullptr;

        case ParameterFamily::QueenMobility:
            if (semanticIndex >= 0 && semanticIndex < 5)
                return &QueenMobilityMiddleGameParameters[semanticIndex];
            if (semanticIndex >= 5 && semanticIndex < 10)
                return &QueenMobilityEndGameParameters[semanticIndex - 5];
            return nullptr;

        case ParameterFamily::Attack:
        {
            if (semanticIndex < 0 || semanticIndex >= 60)
                return nullptr;

            int attacker = semanticIndex / 10; // 0..5
            int rem = semanticIndex % 10;
            bool isEg = (rem >= 5);
            int victimId = (rem % 5) + 1; // victim ids 1..5

            int* mgTables[6] = {
                PawnAttackValueMiddleGame, KnightAttackValueMiddleGame, BishopAttackValueMiddleGame,
                RookAttackValueMiddleGame, QueenAttackValueMiddleGame, KingAttackValueMiddleGame
            };
            int* egTables[6] = {
                PawnAttackValueEndGame, KnightAttackValueEndGame, BishopAttackValueEndGame,
                RookAttackValueEndGame, QueenAttackValueEndGame, KingAttackValueEndGame
            };

            return isEg ? &egTables[attacker][victimId] : &mgTables[attacker][victimId];
        }

        case ParameterFamily::Inline:
            switch (semanticIndex)
            {
            case 0: return &BishopPairValue;
            case 1: return &BishopOpenFilePawnScale;
            case 2: return &TempoMiddleGame;
            case 3: return &TempoEndGame;
            case 4: return &OppositeColorBishopMiddleGameScalePermille;
            case 5: return &OppositeColorBishopEndGameScalePermille;
            case 6: return &MaterialBalanceOffset;
            case 7: return &PawnDeficitZeroPawnMultiplierPermille;
            case 8: return &PawnDeficitOnePawnMultiplierPermille;
            case 9: return &EndgamePawnAdvancementRankMultiplier;
            case 10: return &PieceAttackScalePercent;
            default: return nullptr;
            }

        case ParameterFamily::RookFile:
            switch (semanticIndex)
            {
            case 0: return &RookOpenFileMiddleGame;
            case 1: return &RookOpenFileEndGame;
            case 2: return &RookSemiOpenFileMiddleGame;
            case 3: return &RookSemiOpenFileEndGame;
            default: return nullptr;
            }

        case ParameterFamily::KnightOutpost:
            switch (semanticIndex)
            {
            case 0: return &KnightOutpostMiddleGame;
            case 1: return &KnightOutpostEndGame;
            case 2: return &KnightSupportedOutpostMiddleGame;
            case 3: return &KnightSupportedOutpostEndGame;
            default: return nullptr;
            }

        case ParameterFamily::IsolatedPawn:
            switch (semanticIndex)
            {
            case 0: return &IsolatedPawnMiddleGame;
            case 1: return &IsolatedPawnEndGame;
            default: return nullptr;
            }

        case ParameterFamily::RookBehindPassedPawn:
            switch (semanticIndex)
            {
            case 0: return &RookBehindPassedPawnMiddleGame;
            case 1: return &RookBehindPassedPawnEndGame;
            default: return nullptr;
            }

        case ParameterFamily::EndgameWeights:
            switch (semanticIndex)
            {
            case 0: return &LoneKingBase;
            case 1: return &LoneKingEdgeWeight;
            case 2: return &LoneKingCornerWeight;
            case 3: return &LoneKingConfinementWeight;
            case 4: return &LoneKingRestrictedNeighbourWeight;
            case 5: return &LowMaterialScalePermille;
            default: return nullptr;
            }

        case ParameterFamily::KingSafety:
        {
            int* values[30] = {
                &KingAttackerPawnWeight, &KingAttackerMinorWeight,
                &KingAttackerRookWeight, &KingAttackerQueenWeight,
                &KingDefenderPawnWeight, &KingDefenderMinorWeight,
                &KingDefenderRookWeight, &KingDefenderQueenWeight,
                &KingShelterSecondRankDanger, &KingShelterAdvancedPawnDanger,
                &KingShelterMissingPawnDanger, &KingShelterOpenFileDanger,
                &KingUndefendedZoneDanger, &KingAdditionalZoneAttackerDanger,
                &KingSemiOpenLineDanger, &KingOpenLineDanger,
                &KingDiagonalLineDanger, &KingControlledEscapeDanger,
                &KingBlockedEscapeDanger, &KingTrappedEscapeDanger,
                &KingHeavyBatteryDanger, &CentralKingInnerMinorPressure,
                &CentralKingOuterMinorPressure, &CentralKingReadinessLagWeight,
                &CentralKingPressureScale, &KingUnreadyCoordinationWeight,
                &KingLatentActivationWeight, &KingFutureShelterWingWeight,
                &KingPinnedShelterPawnWeight, &KingInfiltratedQueenWeight
            };
            return semanticIndex >= 0 && semanticIndex < 30 ? values[semanticIndex] : nullptr;
        }

        default:
            return nullptr;
        }
    }

    const int* GetParameterPointer(ParameterFamily family, int semanticIndex) const
    {
        return const_cast<TunerEvaluationState*>(this)->GetParameterPointer(family, semanticIndex);
    }

    // =========================================================================
    // Initialization from TunerRegistry currentValue fields
    // =========================================================================
    bool LoadFromRegistry(const TunerRegistry& registry)
    {
        const auto& params = registry.GetParameters();
        for (const auto& param : params)
        {
            int* target = GetParameterPointer(param.family, param.semanticIndex);
            if (!target)
            {
                return false;
            }
            *target = param.currentValue;
        }

        Derive();
        return true;
    }

    // =========================================================================
    // Recreate Tuner-Side Derived Data
    // =========================================================================
    void Derive()
    {
        PassedPawnV2::Generate(PassedPawnMiddleGameParameters,
                               PassedPawnMiddleGameFileAmplitude,
                               PassedPawnEndGameParameters,
                               WhitePassedPawnValueMiddleGam,
                               WhitePassedPawnValueEndGame);

        MobilityV2::GenerateKnight(KnightMobilityMiddleGameParameters, KnightMoveCountValueMiddleGame);
        MobilityV2::GenerateKnight(KnightMobilityEndGameParameters, KnightMoveCountValueEndGame);
        MobilityV2::GenerateBishop(BishopMobilityMiddleGameParameters, BishopMoveCountValueMiddleGame);
        MobilityV2::GenerateBishop(BishopMobilityEndGameParameters, BishopMoveCountValueEndGame);
        MobilityV2::GenerateRook(RookMobilityMiddleGameParameters, RookMoveCountValueMiddleGame);
        MobilityV2::GenerateRook(RookMobilityEndGameParameters, RookMoveCountValueEndGame);
        MobilityV2::GenerateQueenMiddleGame(QueenMobilityMiddleGameParameters, QueenMoveCountValueMiddleGame);
        MobilityV2::GenerateQueenEndGame(QueenMobilityEndGameParameters, QueenMoveCountValueEndGame);
        PieceSquareModel::GeneratePawn(PawnPieceSquareMiddleGameParameters, PawnInValueWhiteMiddleGame);
        PieceSquareModel::GenerateMinor(KnightPieceSquareMiddleGameParameters, KnightInValueWhiteMiddleGame);
        PieceSquareModel::GenerateMinor(BishopPieceSquareMiddleGameParameters, BishopInValueWhiteMiddleGame);
        PieceSquareModel::GenerateMajor(RookPieceSquareMiddleGameParameters, RookInValueWhiteMiddleGame);
        PieceSquareModel::GenerateMajor(QueenPieceSquareMiddleGameParameters, QueenInValueWhiteMiddleGame);
        PieceSquareModel::GenerateKing(KingPieceSquareMiddleGameParameters, KingInValueWhiteMiddleGame);
        PieceSquareModel::GeneratePawn(PawnPieceSquareEndGameParameters, PawnInValueWhiteEndGame);
        PieceSquareModel::GenerateMinor(KnightPieceSquareEndGameParameters, KnightInValueWhiteEndGame);
        PieceSquareModel::GenerateMinor(BishopPieceSquareEndGameParameters, BishopInValueWhiteEndGame);
        PieceSquareModel::GenerateMajor(RookPieceSquareEndGameParameters, RookInValueWhiteEndGame);
        PieceSquareModel::GenerateMajor(QueenPieceSquareEndGameParameters, QueenInValueWhiteEndGame);
        PieceSquareModel::GenerateKing(KingPieceSquareEndGameParameters, KingInValueWhiteEndGame);
        std::fill(PawnMoveCountValueMiddleGame, PawnMoveCountValueMiddleGame + 3, 0);
        std::fill(PawnMoveCountValueEndGame, PawnMoveCountValueEndGame + 3, 0);
        std::fill(KingMoveCountValueMiddleGame, KingMoveCountValueMiddleGame + 9, 0);
        std::fill(KingMoveCountValueEndGame, KingMoveCountValueEndGame + 9, 0);

        auto mirrorSquare = [](int sq) -> int
        {
            return (7 - (sq / 8)) * 8 + (sq % 8);
        };

        // 1. Black mirrored tables from authored White tables
        for (int sq = 0; sq < 64; ++sq)
        {
            int mSq = mirrorSquare(sq);
            BlackPassedPawnValueMiddleGam[sq] = WhitePassedPawnValueMiddleGam[mSq];
            BlackPassedPawnValueEndGam[sq] = WhitePassedPawnValueEndGame[mSq];
        }

        int* whitePstMg[6] = {
            PawnInValueWhiteMiddleGame, KnightInValueWhiteMiddleGame, BishopInValueWhiteMiddleGame,
            RookInValueWhiteMiddleGame, QueenInValueWhiteMiddleGame, KingInValueWhiteMiddleGame
        };
        int* whitePstEg[6] = {
            PawnInValueWhiteEndGame, KnightInValueWhiteEndGame, BishopInValueWhiteEndGame,
            RookInValueWhiteEndGame, QueenInValueWhiteEndGame, KingInValueWhiteEndGame
        };
        int* blackPstMg[6] = {
            PawnInValueBlackMiddleGame, KnightInValueBlackMiddleGame, BishopInValueBlackMiddleGame,
            RookInValueBlackMiddleGame, QueenInValueBlackMiddleGame, KingInValueBlackMiddleGame
        };
        int* blackPstEg[6] = {
            PawnInValueBlackEndGame, KnightInValueBlackEndGame, BishopInValueBlackEndGame,
            RookInValueBlackEndGame, QueenInValueBlackEndGame, KingInValueBlackEndGame
        };

        for (int p = 0; p < 6; ++p)
        {
            for (int sq = 0; sq < 64; ++sq)
            {
                int mSq = mirrorSquare(sq);
                blackPstMg[p][sq] = whitePstMg[p][mSq];
                blackPstEg[p][sq] = whitePstEg[p][mSq];
            }
        }

        // 2. Attack victim colour duplication (White victims 1..5 copied to Black victims 9..13)
        int* attackMg[6] = {
            PawnAttackValueMiddleGame, KnightAttackValueMiddleGame, BishopAttackValueMiddleGame,
            RookAttackValueMiddleGame, QueenAttackValueMiddleGame, KingAttackValueMiddleGame
        };
        int* attackEg[6] = {
            PawnAttackValueEndGame, KnightAttackValueEndGame, BishopAttackValueEndGame,
            RookAttackValueEndGame, QueenAttackValueEndGame, KingAttackValueEndGame
        };

        for (int a = 0; a < 6; ++a)
        {
            for (int v = 1; v <= 5; ++v)
            {
                attackMg[a][v + 8] = attackMg[a][v];
                attackEg[a][v + 8] = attackEg[a][v];
            }
        }

        // 3. Runtime MG/EG source copies required by the evaluation parameter layout
        // PST runtime arrays: [0] = MiddleGame, [2] = EndGame
        int (*whitePstRuntime[6])[64] = {
            PawnInValueWhite, KnightInValueWhite, BishopInValueWhite,
            RookInValueWhite, QueenInValueWhite, KingInValueWhite
        };
        int (*blackPstRuntime[6])[64] = {
            PawnInValueBlack, KnightInValueBlack, BishopInValueBlack,
            RookInValueBlack, QueenInValueBlack, KingInValueBlack
        };

        for (int p = 0; p < 6; ++p)
        {
            for (int sq = 0; sq < 64; ++sq)
            {
                whitePstRuntime[p][0][sq] = whitePstMg[p][sq];
                whitePstRuntime[p][1][sq] = 0;
                whitePstRuntime[p][2][sq] = whitePstEg[p][sq];

                blackPstRuntime[p][0][sq] = blackPstMg[p][sq];
                blackPstRuntime[p][1][sq] = 0;
                blackPstRuntime[p][2][sq] = blackPstEg[p][sq];
            }
        }

        // Mobility runtime arrays: [0] = MiddleGame, [2] = EndGame
        for (int i = 0; i < 3; ++i)
        {
            PawnMoveCountValue[0][i] = PawnMoveCountValueMiddleGame[i];
            PawnMoveCountValue[1][i] = 0;
            PawnMoveCountValue[2][i] = PawnMoveCountValueEndGame[i];
        }
        for (int i = 0; i < 9; ++i)
        {
            KnightMoveCountValue[0][i] = KnightMoveCountValueMiddleGame[i];
            KnightMoveCountValue[1][i] = 0;
            KnightMoveCountValue[2][i] = KnightMoveCountValueEndGame[i];
        }
        for (int i = 0; i < 14; ++i)
        {
            BishopMoveCountValue[0][i] = BishopMoveCountValueMiddleGame[i];
            BishopMoveCountValue[1][i] = 0;
            BishopMoveCountValue[2][i] = BishopMoveCountValueEndGame[i];
        }
        for (int i = 0; i < 15; ++i)
        {
            RookMoveCountValue[0][i] = RookMoveCountValueMiddleGame[i];
            RookMoveCountValue[1][i] = 0;
            RookMoveCountValue[2][i] = RookMoveCountValueEndGame[i];
        }
        for (int i = 0; i < 28; ++i)
        {
            QueenMoveCountValue[0][i] = QueenMoveCountValueMiddleGame[i];
            QueenMoveCountValue[1][i] = 0;
            QueenMoveCountValue[2][i] = QueenMoveCountValueEndGame[i];
        }
        for (int i = 0; i < 9; ++i)
        {
            KingMoveCountValue[0][i] = KingMoveCountValueMiddleGame[i];
            KingMoveCountValue[1][i] = 0;
            KingMoveCountValue[2][i] = KingMoveCountValueEndGame[i];
        }

        // Attack runtime arrays: [0] = MiddleGame, [2] = EndGame
        int (*attackRuntime[6])[16] = {
            PawnAttackValue, KnightAttackValue, BishopAttackValue,
            RookAttackValue, QueenAttackValue, KingAttackValue
        };

        for (int a = 0; a < 6; ++a)
        {
            for (int slot = 0; slot < 16; ++slot)
            {
                attackRuntime[a][0][slot] = attackMg[a][slot];
                attackRuntime[a][1][slot] = 0;
                attackRuntime[a][2][slot] = attackEg[a][slot];
            }
        }
    }

    // =========================================================================
    // Self-Consistency Verification Helper
    // =========================================================================
    struct ConsistencyResult
    {
        std::size_t registryEntryCount = 0;
        std::size_t mappedCount = 0;
        bool everyEntryMappedExactlyOnce = false;
        bool derivedDeterministic = false;
        bool noLostOrDuplicated = false;
    };

    static ConsistencyResult VerifySelfConsistency(const TunerRegistry& registry)
    {
        ConsistencyResult res;
        res.registryEntryCount = registry.Size();

        TunerEvaluationState state;
        std::unordered_set<int*> mappedPointers;
        bool duplicateFound = false;

        for (std::size_t i = 0; i < registry.Size(); ++i)
        {
            const auto& param = registry[i];
            int* ptr = state.GetParameterPointer(param.family, param.semanticIndex);
            if (!ptr)
            {
                continue;
            }
            if (mappedPointers.find(ptr) != mappedPointers.end())
            {
                duplicateFound = true;
            }
            mappedPointers.insert(ptr);
            *ptr = param.currentValue;
            res.mappedCount++;
        }

        res.noLostOrDuplicated = (!duplicateFound) && (mappedPointers.size() == registry.Size());
        res.everyEntryMappedExactlyOnce = (res.mappedCount == registry.Size()) && res.noLostOrDuplicated;

        // Perform derivation and check consistency
        state.Derive();

        // Check Black mirrored table matches
        auto mirrorSquare = [](int sq) -> int
        {
            return (7 - (sq / 8)) * 8 + (sq % 8);
        };
        bool mirrorOk = true;
        for (int sq = 0; sq < 64; ++sq)
        {
            int mSq = mirrorSquare(sq);
            if (state.BlackPassedPawnValueMiddleGam[sq] != state.WhitePassedPawnValueMiddleGam[mSq] ||
                state.BlackPassedPawnValueEndGam[sq] != state.WhitePassedPawnValueEndGame[mSq] ||
                state.PawnInValueBlackMiddleGame[sq] != state.PawnInValueWhiteMiddleGame[mSq])
            {
                mirrorOk = false;
                break;
            }
        }

        // Check attack victim duplication
        bool attackOk = true;
        for (int v = 1; v <= 5; ++v)
        {
            if (state.PawnAttackValueMiddleGame[v + 8] != state.PawnAttackValueMiddleGame[v] ||
                state.KnightAttackValueEndGame[v + 8] != state.KnightAttackValueEndGame[v])
            {
                attackOk = false;
                break;
            }
        }

        // Check runtime MG/EG copies
        bool runtimeOk = true;
        for (int sq = 0; sq < 64; ++sq)
        {
            if (state.PawnInValueWhite[0][sq] != state.PawnInValueWhiteMiddleGame[sq] ||
                state.PawnInValueWhite[2][sq] != state.PawnInValueWhiteEndGame[sq] ||
                state.PawnInValueBlack[0][sq] != state.PawnInValueBlackMiddleGame[sq] ||
                state.PawnInValueBlack[2][sq] != state.PawnInValueBlackEndGame[sq])
            {
                runtimeOk = false;
                break;
            }
        }
        for (int i = 0; i < 3; ++i)
        {
            if (state.PawnMoveCountValue[0][i] != state.PawnMoveCountValueMiddleGame[i] ||
                state.PawnMoveCountValue[2][i] != state.PawnMoveCountValueEndGame[i])
            {
                runtimeOk = false;
                break;
            }
        }
        for (int slot = 0; slot < 16; ++slot)
        {
            if (state.PawnAttackValue[0][slot] != state.PawnAttackValueMiddleGame[slot] ||
                state.PawnAttackValue[2][slot] != state.PawnAttackValueEndGame[slot])
            {
                runtimeOk = false;
                break;
            }
        }

        res.derivedDeterministic = mirrorOk && attackOk && runtimeOk;
        return res;
    }
};

} // namespace Tuner

#endif // HOWL_TUNER_EVALUATION_STATE_H
