#ifndef HOWL_TUNER_VERIFIER_H
#define HOWL_TUNER_VERIFIER_H

#include <string>
#include "Option.h"
#include "tuner/TunerEvaluationState.h"

namespace Tuner
{

struct VerificationResult
{
    bool passed = true;
    int totalCompared = 0;
    std::string mismatchFamily = "";
    int mismatchIndex = -1;
    int tunerValue = 0;
    int optionValue = 0;
};

class TunerVerifier
{
public:
    static VerificationResult Verify(const TunerEvaluationState& state)
    {
        VerificationResult res;

        auto check = [&](const std::string& family, int index, int tunerVal, int optionVal) -> bool
        {
            res.totalCompared++;
            if (tunerVal != optionVal)
            {
                res.passed = false;
                res.mismatchFamily = family;
                res.mismatchIndex = index;
                res.tunerValue = tunerVal;
                res.optionValue = optionVal;
                return false;
            }
            return true;
        };

        // 1. Piece values (5)
        if (!check("PieceValue", 0, state.PawnValue, Option::PawnValue)) return res;
        if (!check("PieceValue", 1, state.KnightValue, Option::KnightValue)) return res;
        if (!check("PieceValue", 2, state.BishopValue, Option::BishopValue)) return res;
        if (!check("PieceValue", 3, state.RookValue, Option::RookValue)) return res;
        if (!check("PieceValue", 4, state.QueenValue, Option::QueenValue)) return res;

        // 2. DoubledPawnValue (1)
        if (!check("DoubledPawnValue", 0, state.DoubledPawnValue, Option::DoubledPawnValue)) return res;

        // 3. Passed pawn MG and EG tables (White & Black, 256)
        for (int sq = 0; sq < 64; ++sq)
        {
            if (!check("PassedPawn_White_MG", sq, state.WhitePassedPawnValueMiddleGam[sq], Option::WhitePassedPawnValueMiddleGam[sq])) return res;
            if (!check("PassedPawn_White_EG", sq, state.WhitePassedPawnValueEndGame[sq], Option::WhitePassedPawnValueEndGame[sq])) return res;
            if (!check("PassedPawn_Black_MG", sq, state.BlackPassedPawnValueMiddleGam[sq], Option::BlackPassedPawnValueMiddleGam[sq])) return res;
            if (!check("PassedPawn_Black_EG", sq, state.BlackPassedPawnValueEndGam[sq], Option::BlackPassedPawnValueEndGam[sq])) return res;
        }

        // 4. Piece square MG and EG tables (White, 768)
        const int* whitePstMgTuner[6] = {
            state.PawnInValueWhiteMiddleGame, state.KnightInValueWhiteMiddleGame, state.BishopInValueWhiteMiddleGame,
            state.RookInValueWhiteMiddleGame, state.QueenInValueWhiteMiddleGame, state.KingInValueWhiteMiddleGame
        };
        const int* whitePstMgOption[6] = {
            Option::PawnInValueWhiteMiddleGame, Option::KnightInValueWhiteMiddleGame, Option::BishopInValueWhiteMiddleGame,
            Option::RookInValueWhiteMiddleGame, Option::QueenInValueWhiteMiddleGame, Option::KingInValueWhiteMiddleGame
        };
        const int* whitePstEgTuner[6] = {
            state.PawnInValueWhiteEndGame, state.KnightInValueWhiteEndGame, state.BishopInValueWhiteEndGame,
            state.RookInValueWhiteEndGame, state.QueenInValueWhiteEndGame, state.KingInValueWhiteEndGame
        };
        const int* whitePstEgOption[6] = {
            Option::PawnInValueWhiteEndGame, Option::KnightInValueWhiteEndGame, Option::BishopInValueWhiteEndGame,
            Option::RookInValueWhiteEndGame, Option::QueenInValueWhiteEndGame, Option::KingInValueWhiteEndGame
        };

        for (int p = 0; p < 6; ++p)
        {
            for (int sq = 0; sq < 64; ++sq)
            {
                int idx = p * 64 + sq;
                if (!check("PieceSquare_White_MG", idx, whitePstMgTuner[p][sq], whitePstMgOption[p][sq])) return res;
                if (!check("PieceSquare_White_EG", idx, whitePstEgTuner[p][sq], whitePstEgOption[p][sq])) return res;
            }
        }

        // 5. Derived Black piece square tables (768)
        const int* blackPstMgTuner[6] = {
            state.PawnInValueBlackMiddleGame, state.KnightInValueBlackMiddleGame, state.BishopInValueBlackMiddleGame,
            state.RookInValueBlackMiddleGame, state.QueenInValueBlackMiddleGame, state.KingInValueBlackMiddleGame
        };
        const int* blackPstMgOption[6] = {
            Option::PawnInValueBlackMiddleGame, Option::KnightInValueBlackMiddleGame, Option::BishopInValueBlackMiddleGame,
            Option::RookInValueBlackMiddleGame, Option::QueenInValueBlackMiddleGame, Option::KingInValueBlackMiddleGame
        };
        const int* blackPstEgTuner[6] = {
            state.PawnInValueBlackEndGame, state.KnightInValueBlackEndGame, state.BishopInValueBlackEndGame,
            state.RookInValueBlackEndGame, state.QueenInValueBlackEndGame, state.KingInValueBlackEndGame
        };
        const int* blackPstEgOption[6] = {
            Option::PawnInValueBlackEndGame, Option::KnightInValueBlackEndGame, Option::BishopInValueBlackEndGame,
            Option::RookInValueBlackEndGame, Option::QueenInValueBlackEndGame, Option::KingInValueBlackEndGame
        };

        for (int p = 0; p < 6; ++p)
        {
            for (int sq = 0; sq < 64; ++sq)
            {
                int idx = p * 64 + sq;
                if (!check("PieceSquare_Black_MG", idx, blackPstMgTuner[p][sq], blackPstMgOption[p][sq])) return res;
                if (!check("PieceSquare_Black_EG", idx, blackPstEgTuner[p][sq], blackPstEgOption[p][sq])) return res;
            }
        }

        // 6. CenterPresence White and Black (768)
        const int* cpWhiteTuner[6] = {
            state.PawnInCenterValueWhite, state.KnightInCenterValueWhite, state.BishopInCenterValueWhite,
            state.RookInCenterValueWhite, state.QueenInCenterValueWhite, state.KingInCenterValueWhite
        };
        const int* cpWhiteOption[6] = {
            Option::PawnInCenterValueWhite, Option::KnightInCenterValueWhite, Option::BishopInCenterValueWhite,
            Option::RookInCenterValueWhite, Option::QueenInCenterValueWhite, Option::KingInCenterValueWhite
        };
        const int* cpBlackTuner[6] = {
            state.PawnInCenterValueBlack, state.KnightInCenterValueBlack, state.BishopInCenterValueBlack,
            state.RookInCenterValueBlack, state.QueenInCenterValueBlack, state.KingInCenterValueBlack
        };
        const int* cpBlackOption[6] = {
            Option::PawnInCenterValueBlack, Option::KnightInCenterValueBlack, Option::BishopInCenterValueBlack,
            Option::RookInCenterValueBlack, Option::QueenInCenterValueBlack, Option::KingInCenterValueBlack
        };

        for (int p = 0; p < 6; ++p)
        {
            for (int sq = 0; sq < 64; ++sq)
            {
                int idx = p * 64 + sq;
                if (!check("CenterPresence_White", idx, cpWhiteTuner[p][sq], cpWhiteOption[p][sq])) return res;
                if (!check("CenterPresence_Black", idx, cpBlackTuner[p][sq], cpBlackOption[p][sq])) return res;
            }
        }

        // 7. CenterMove White and Black (768)
        const int* cmWhiteTuner[6] = {
            state.PawnMoveCenterValueWhite, state.KnightMoveCenterValueWhite, state.BishopMoveCenterValueWhite,
            state.RookMoveCenterValueWhite, state.QueenMoveCenterValueWhite, state.KingMoveCenterValueWhite
        };
        const int* cmWhiteOption[6] = {
            Option::PawnMoveCenterValueWhite, Option::KnightMoveCenterValueWhite, Option::BishopMoveCenterValueWhite,
            Option::RookMoveCenterValueWhite, Option::QueenMoveCenterValueWhite, Option::KingMoveCenterValueWhite
        };
        const int* cmBlackTuner[6] = {
            state.PawnMoveCenterValueBlack, state.KnightMoveCenterValueBlack, state.BishopMoveCenterValueBlack,
            state.RookMoveCenterValueBlack, state.QueenMoveCenterValueBlack, state.KingMoveCenterValueBlack
        };
        const int* cmBlackOption[6] = {
            Option::PawnMoveCenterValueBlack, Option::KnightMoveCenterValueBlack, Option::BishopMoveCenterValueBlack,
            Option::RookMoveCenterValueBlack, Option::QueenMoveCenterValueBlack, Option::KingMoveCenterValueBlack
        };

        for (int p = 0; p < 6; ++p)
        {
            for (int sq = 0; sq < 64; ++sq)
            {
                int idx = p * 64 + sq;
                if (!check("CenterMove_White", idx, cmWhiteTuner[p][sq], cmWhiteOption[p][sq])) return res;
                if (!check("CenterMove_Black", idx, cmBlackTuner[p][sq], cmBlackOption[p][sq])) return res;
            }
        }

        // 8. KingSafety White and Black (128)
        for (int sq = 0; sq < 64; ++sq)
        {
            if (!check("KingSafety_White", sq, state.WhiteKingPlaceSafetyMiddleGame[sq], Option::WhiteKingPlaceSafetyMiddleGame[sq])) return res;
            if (!check("KingSafety_Black", sq, state.BlackKingPlaceSafetyMiddleGame[sq], Option::BlackKingPlaceSafetyMiddleGame[sq])) return res;
        }

        // 9. Mobility MG and EG authored tables (156)
        const int* mobMgTuner[6] = {
            state.PawnMoveCountValueMiddleGame, state.KnightMoveCountValueMiddleGame, state.BishopMoveCountValueMiddleGame,
            state.RookMoveCountValueMiddleGame, state.QueenMoveCountValueMiddleGame, state.KingMoveCountValueMiddleGame
        };
        const int* mobMgOption[6] = {
            Option::PawnMoveCountValueMiddleGame, Option::KnightMoveCountValueMiddleGame, Option::BishopMoveCountValueMiddleGame,
            Option::RookMoveCountValueMiddleGame, Option::QueenMoveCountValueMiddleGame, Option::KingMoveCountValueMiddleGame
        };
        const int* mobEgTuner[6] = {
            state.PawnMoveCountValueEndGame, state.KnightMoveCountValueEndGame, state.BishopMoveCountValueEndGame,
            state.RookMoveCountValueEndGame, state.QueenMoveCountValueEndGame, state.KingMoveCountValueEndGame
        };
        const int* mobEgOption[6] = {
            Option::PawnMoveCountValueEndGame, Option::KnightMoveCountValueEndGame, Option::BishopMoveCountValueEndGame,
            Option::RookMoveCountValueEndGame, Option::QueenMoveCountValueEndGame, Option::KingMoveCountValueEndGame
        };
        const int mobCounts[6] = {3, 9, 14, 15, 28, 9};

        int mobIdx = 0;
        for (int p = 0; p < 6; ++p)
        {
            for (int i = 0; i < mobCounts[p]; ++i)
            {
                if (!check("Mobility_MG", mobIdx, mobMgTuner[p][i], mobMgOption[p][i])) return res;
                if (!check("Mobility_EG", mobIdx, mobEgTuner[p][i], mobEgOption[p][i])) return res;
                mobIdx++;
            }
        }

        // 10. Mobility runtime [3][...] copies (156)
        for (int i = 0; i < 3; ++i)
        {
            if (!check("Mobility_Runtime_Pawn_MG", i, state.PawnMoveCountValue[0][i], Option::PawnMoveCountValue[0][i])) return res;
            if (!check("Mobility_Runtime_Pawn_EG", i, state.PawnMoveCountValue[2][i], Option::PawnMoveCountValue[2][i])) return res;
        }
        for (int i = 0; i < 9; ++i)
        {
            if (!check("Mobility_Runtime_Knight_MG", i, state.KnightMoveCountValue[0][i], Option::KnightMoveCountValue[0][i])) return res;
            if (!check("Mobility_Runtime_Knight_EG", i, state.KnightMoveCountValue[2][i], Option::KnightMoveCountValue[2][i])) return res;
        }
        for (int i = 0; i < 14; ++i)
        {
            if (!check("Mobility_Runtime_Bishop_MG", i, state.BishopMoveCountValue[0][i], Option::BishopMoveCountValue[0][i])) return res;
            if (!check("Mobility_Runtime_Bishop_EG", i, state.BishopMoveCountValue[2][i], Option::BishopMoveCountValue[2][i])) return res;
        }
        for (int i = 0; i < 15; ++i)
        {
            if (!check("Mobility_Runtime_Rook_MG", i, state.RookMoveCountValue[0][i], Option::RookMoveCountValue[0][i])) return res;
            if (!check("Mobility_Runtime_Rook_EG", i, state.RookMoveCountValue[2][i], Option::RookMoveCountValue[2][i])) return res;
        }
        for (int i = 0; i < 28; ++i)
        {
            if (!check("Mobility_Runtime_Queen_MG", i, state.QueenMoveCountValue[0][i], Option::QueenMoveCountValue[0][i])) return res;
            if (!check("Mobility_Runtime_Queen_EG", i, state.QueenMoveCountValue[2][i], Option::QueenMoveCountValue[2][i])) return res;
        }
        for (int i = 0; i < 9; ++i)
        {
            if (!check("Mobility_Runtime_King_MG", i, state.KingMoveCountValue[0][i], Option::KingMoveCountValue[0][i])) return res;
            if (!check("Mobility_Runtime_King_EG", i, state.KingMoveCountValue[2][i], Option::KingMoveCountValue[2][i])) return res;
        }

        // 11. Attack MG and EG semantic values (60)
        const int* attMgTuner[6] = {
            state.PawnAttackValueMiddleGame, state.KnightAttackValueMiddleGame, state.BishopAttackValueMiddleGame,
            state.RookAttackValueMiddleGame, state.QueenAttackValueMiddleGame, state.KingAttackValueMiddleGame
        };
        const int* attMgOption[6] = {
            Option::PawnAttackValueMiddleGame, Option::KnightAttackValueMiddleGame, Option::BishopAttackValueMiddleGame,
            Option::RookAttackValueMiddleGame, Option::QueenAttackValueMiddleGame, Option::KingAttackValueMiddleGame
        };
        const int* attEgTuner[6] = {
            state.PawnAttackValueEndGame, state.KnightAttackValueEndGame, state.BishopAttackValueEndGame,
            state.RookAttackValueEndGame, state.QueenAttackValueEndGame, state.KingAttackValueEndGame
        };
        const int* attEgOption[6] = {
            Option::PawnAttackValueEndGame, Option::KnightAttackValueEndGame, Option::BishopAttackValueEndGame,
            Option::RookAttackValueEndGame, Option::QueenAttackValueEndGame, Option::KingAttackValueEndGame
        };

        for (int a = 0; a < 6; ++a)
        {
            for (int v = 1; v <= 5; ++v)
            {
                int idx = a * 5 + (v - 1);
                if (!check("Attack_Semantic_MG", idx, attMgTuner[a][v], attMgOption[a][v])) return res;
                if (!check("Attack_Semantic_EG", idx, attEgTuner[a][v], attEgOption[a][v])) return res;
            }
        }

        // 12. Attack 16 slot runtime victim tables including White and Black victim duplication (192)
        const int (*attRtTuner[6])[16] = {
            state.PawnAttackValue, state.KnightAttackValue, state.BishopAttackValue,
            state.RookAttackValue, state.QueenAttackValue, state.KingAttackValue
        };
        const int (*attRtOption[6])[16] = {
            Option::PawnAttackValue, Option::KnightAttackValue, Option::BishopAttackValue,
            Option::RookAttackValue, Option::QueenAttackValue, Option::KingAttackValue
        };

        for (int a = 0; a < 6; ++a)
        {
            for (int slot = 0; slot < 16; ++slot)
            {
                int idx = a * 16 + slot;
                if (!check("Attack_Runtime_MG", idx, attRtTuner[a][0][slot], attRtOption[a][0][slot])) return res;
                if (!check("Attack_Runtime_EG", idx, attRtTuner[a][2][slot], attRtOption[a][2][slot])) return res;
            }
        }

        return res;
    }
};

} // namespace Tuner

#endif // HOWL_TUNER_VERIFIER_H
