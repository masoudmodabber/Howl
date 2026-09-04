#ifndef HOWL_TUNER_EVALUATION_STATE_H
#define HOWL_TUNER_EVALUATION_STATE_H

#include <cstddef>
#include <unordered_set>
#include <vector>
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

    // Family: PassedPawn (128: 64 MG + 64 EG)
    int WhitePassedPawnValueMiddleGam[64] = {0};
    int WhitePassedPawnValueEndGame[64] = {0};

    // Family: PieceSquare (768: 6 pieces * 2 phases * 64 squares)
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

    // Family: CenterPresence (384: 6 pieces * 64 squares)
    int PawnInCenterValueWhite[64] = {0};
    int KnightInCenterValueWhite[64] = {0};
    int BishopInCenterValueWhite[64] = {0};
    int RookInCenterValueWhite[64] = {0};
    int QueenInCenterValueWhite[64] = {0};
    int KingInCenterValueWhite[64] = {0};

    // Family: CenterMove (384: 6 pieces * 64 squares)
    int PawnMoveCenterValueWhite[64] = {0};
    int KnightMoveCenterValueWhite[64] = {0};
    int BishopMoveCenterValueWhite[64] = {0};
    int RookMoveCenterValueWhite[64] = {0};
    int QueenMoveCenterValueWhite[64] = {0};
    int KingMoveCenterValueWhite[64] = {0};

    // Family: KingSafety (64 squares)
    int WhiteKingPlaceSafetyMiddleGame[64] = {0};

    // Family: Mobility (156: 6 pieces * 2 phases; counts: P3, N9, B14, R15, Q28, K9)
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

    // Family: Inline (10 parameters)
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

    int PawnInCenterValueBlack[64] = {0};
    int KnightInCenterValueBlack[64] = {0};
    int BishopInCenterValueBlack[64] = {0};
    int RookInCenterValueBlack[64] = {0};
    int QueenInCenterValueBlack[64] = {0};
    int KingInCenterValueBlack[64] = {0};

    int PawnMoveCenterValueBlack[64] = {0};
    int KnightMoveCenterValueBlack[64] = {0};
    int BishopMoveCenterValueBlack[64] = {0};
    int RookMoveCenterValueBlack[64] = {0};
    int QueenMoveCenterValueBlack[64] = {0};
    int KingMoveCenterValueBlack[64] = {0};

    int BlackKingPlaceSafetyMiddleGame[64] = {0};

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

        case ParameterFamily::PassedPawn:
            if (semanticIndex >= 0 && semanticIndex < 64)
                return &WhitePassedPawnValueMiddleGam[semanticIndex];
            if (semanticIndex >= 64 && semanticIndex < 128)
                return &WhitePassedPawnValueEndGame[semanticIndex - 64];
            return nullptr;

        case ParameterFamily::PieceSquare:
        {
            if (semanticIndex < 0 || semanticIndex >= 768)
                return nullptr;
            int table = semanticIndex / 64;
            int sq = semanticIndex % 64;
            int* tables[12] = {
                PawnInValueWhiteMiddleGame, KnightInValueWhiteMiddleGame, BishopInValueWhiteMiddleGame,
                RookInValueWhiteMiddleGame, QueenInValueWhiteMiddleGame, KingInValueWhiteMiddleGame,
                PawnInValueWhiteEndGame, KnightInValueWhiteEndGame, BishopInValueWhiteEndGame,
                RookInValueWhiteEndGame, QueenInValueWhiteEndGame, KingInValueWhiteEndGame
            };
            return &tables[table][sq];
        }

        case ParameterFamily::CenterPresence:
        {
            if (semanticIndex < 0 || semanticIndex >= 384)
                return nullptr;
            int table = semanticIndex / 64;
            int sq = semanticIndex % 64;
            int* tables[6] = {
                PawnInCenterValueWhite, KnightInCenterValueWhite, BishopInCenterValueWhite,
                RookInCenterValueWhite, QueenInCenterValueWhite, KingInCenterValueWhite
            };
            return &tables[table][sq];
        }

        case ParameterFamily::CenterMove:
        {
            if (semanticIndex < 0 || semanticIndex >= 384)
                return nullptr;
            int table = semanticIndex / 64;
            int sq = semanticIndex % 64;
            int* tables[6] = {
                PawnMoveCenterValueWhite, KnightMoveCenterValueWhite, BishopMoveCenterValueWhite,
                RookMoveCenterValueWhite, QueenMoveCenterValueWhite, KingMoveCenterValueWhite
            };
            return &tables[table][sq];
        }

        case ParameterFamily::KingSafety:
            if (semanticIndex >= 0 && semanticIndex < 64)
                return &WhiteKingPlaceSafetyMiddleGame[semanticIndex];
            return nullptr;

        case ParameterFamily::Mobility:
        {
            if (semanticIndex < 0 || semanticIndex >= 156)
                return nullptr;

            struct MobilityLayout
            {
                int* mg;
                int* eg;
                int count;
            };
            const MobilityLayout mob[6] = {
                {PawnMoveCountValueMiddleGame, PawnMoveCountValueEndGame, 3},
                {KnightMoveCountValueMiddleGame, KnightMoveCountValueEndGame, 9},
                {BishopMoveCountValueMiddleGame, BishopMoveCountValueEndGame, 14},
                {RookMoveCountValueMiddleGame, RookMoveCountValueEndGame, 15},
                {QueenMoveCountValueMiddleGame, QueenMoveCountValueEndGame, 28},
                {KingMoveCountValueMiddleGame, KingMoveCountValueEndGame, 9}
            };

            int offset = 0;
            for (int p = 0; p < 6; ++p)
            {
                int mgEnd = offset + mob[p].count;
                if (semanticIndex < mgEnd)
                    return &mob[p].mg[semanticIndex - offset];
                offset = mgEnd;

                int egEnd = offset + mob[p].count;
                if (semanticIndex < egEnd)
                    return &mob[p].eg[semanticIndex - offset];
                offset = egEnd;
            }
            return nullptr;
        }

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
            default: return nullptr;
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
            BlackKingPlaceSafetyMiddleGame[sq] = WhiteKingPlaceSafetyMiddleGame[mSq];
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

        int* whiteCenterPresence[6] = {
            PawnInCenterValueWhite, KnightInCenterValueWhite, BishopInCenterValueWhite,
            RookInCenterValueWhite, QueenInCenterValueWhite, KingInCenterValueWhite
        };
        int* blackCenterPresence[6] = {
            PawnInCenterValueBlack, KnightInCenterValueBlack, BishopInCenterValueBlack,
            RookInCenterValueBlack, QueenInCenterValueBlack, KingInCenterValueBlack
        };

        int* whiteCenterMove[6] = {
            PawnMoveCenterValueWhite, KnightMoveCenterValueWhite, BishopMoveCenterValueWhite,
            RookMoveCenterValueWhite, QueenMoveCenterValueWhite, KingMoveCenterValueWhite
        };
        int* blackCenterMove[6] = {
            PawnMoveCenterValueBlack, KnightMoveCenterValueBlack, BishopMoveCenterValueBlack,
            RookMoveCenterValueBlack, QueenMoveCenterValueBlack, KingMoveCenterValueBlack
        };

        for (int p = 0; p < 6; ++p)
        {
            for (int sq = 0; sq < 64; ++sq)
            {
                int mSq = mirrorSquare(sq);
                blackPstMg[p][sq] = whitePstMg[p][mSq];
                blackPstEg[p][sq] = whitePstEg[p][mSq];
                blackCenterPresence[p][sq] = whiteCenterPresence[p][mSq];
                blackCenterMove[p][sq] = whiteCenterMove[p][mSq];
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
                state.BlackKingPlaceSafetyMiddleGame[sq] != state.WhiteKingPlaceSafetyMiddleGame[mSq] ||
                state.PawnInValueBlackMiddleGame[sq] != state.PawnInValueWhiteMiddleGame[mSq] ||
                state.PawnInCenterValueBlack[sq] != state.PawnInCenterValueWhite[mSq] ||
                state.PawnMoveCenterValueBlack[sq] != state.PawnMoveCenterValueWhite[mSq])
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
