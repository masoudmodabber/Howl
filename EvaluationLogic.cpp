#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "EvaluationLogic.h"
#include "CentralKingAttackPressure.h"
#include "Option.h"
#include "KingSetup.h"
#include "AttackPlaces.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include <algorithm>
#include <functional>
#include <iostream>

namespace
{
struct EvaluationRays
{
    uint64_t rays[8][64]{};
    uint8_t ranks[8][256]{};
    constexpr EvaluationRays()
    {
        const int dr[8] = {1, 1, -1, -1, 1, 0, -1, 0};
        const int df[8] = {1, -1, 1, -1, 0, 1, 0, -1};
        for (int dir = 0; dir < 8; ++dir)
            for (int sq = 0; sq < 64; ++sq)
                for (int r = sq / 8 + dr[dir], f = sq % 8 + df[dir];
                     r >= 0 && r < 8 && f >= 0 && f < 8; r += dr[dir], f += df[dir])
                    rays[dir][sq] |= 1ULL << (8 * r + f);
        for (int file = 0; file < 8; ++file)
            for (int occupied = 0; occupied < 256; ++occupied)
                for (int step : {-1, 1})
                    for (int to = file + step; to >= 0 && to < 8; to += step)
                    {
                        ranks[file][occupied] |= 1 << to;
                        if (occupied & (1 << to)) break;
                    }
    }
};
constexpr EvaluationRays EvalRays{};

uint64_t EvaluationSliderAttacks(int square, int type, uint64_t occupancy)
{
    // Subtraction reaches the first blocker in each direction. Byte reversal
    // reverses square order on files and diagonals (one square per rank).
    const uint64_t piece = 1ULL << square;
    const auto lineAttacks = [&](uint64_t line) {
        const uint64_t occupied = (occupancy & line) | piece;
        return ((occupied - 2 * piece) ^
                __builtin_bswap64(__builtin_bswap64(occupied) - 2 * __builtin_bswap64(piece))) & line;
    };
    uint64_t attacks = 0;
    if (type != 4)
    {
        attacks = lineAttacks(EvalRays.rays[0][square] | EvalRays.rays[3][square]) |
                  lineAttacks(EvalRays.rays[1][square] | EvalRays.rays[2][square]);
    }
    if (type != 3)
    {
        attacks |= lineAttacks(EvalRays.rays[4][square] | EvalRays.rays[6][square]);
        const int shift = square & ~7;
        attacks |= static_cast<uint64_t>(EvalRays.ranks[square & 7][(occupancy >> shift) & 255]) << shift;
    }
    return attacks;
}
}

EvaluationContext::EvaluationContext(Board& b, int p)
    : board(b), phase(p)
{
    whiteKingSq = b.pieces[6].front();
    blackKingSq = b.pieces[14].front();
    whiteKingFile = whiteKingSq % 8;
    blackKingFile = blackKingSq % 8;
    occupancy = b.whitePieces | b.blackPieces;
    pawnAttacks[0] = ((static_cast<uint64_t>(b.whitePawns) & ~0x8080808080808080ULL) << 9) |
                     ((static_cast<uint64_t>(b.whitePawns) & ~0x0101010101010101ULL) << 7);
    pawnAttacks[1] = ((static_cast<uint64_t>(b.blackPawns) & ~0x0101010101010101ULL) >> 9) |
                     ((static_cast<uint64_t>(b.blackPawns) & ~0x8080808080808080ULL) >> 7);

    for (int sq : b.pieces[1])
    {
        whitePawnFiles |= static_cast<uint8_t>(1 << (sq % 8));
        ++pawnFileCounts[0][sq % 8];
        if ((PassedPawnSetup::WhitePassedMask[sq] & b.blackPawns) == 0)
        {
            whitePassedPawns |= (1ULL << sq);
            whitePassers[whitePasserCount++] = sq;
        }
    }

    for (int sq : b.pieces[9])
    {
        blackPawnFiles |= static_cast<uint8_t>(1 << (sq % 8));
        ++pawnFileCounts[1][sq % 8];
        if ((PassedPawnSetup::BlackPassedMask[sq] & b.whitePawns) == 0)
        {
            blackPassedPawns |= (1ULL << sq);
            blackPassers[blackPasserCount++] = sq;
        }
    }
}

void EvaluationContext::InitializeAttacks()
{
    if (attacksReady) return;
    for (int side = 0; side < 2; ++side)
        for (int type = 1; type <= 6; ++type)
            for (int square : board.pieces[side * 8 + type])
            {
                uint64_t mask;
                if (type == 1)
                    mask = side == 0 ? AttackPlaces::WhitePawnAttackPlaces[square]
                                     : AttackPlaces::BlackPawnAttackPlaces[square];
                else if (type == 2) mask = AttackPlaces::KnightAttackPlaces[square];
                else if (type == 6) mask = AttackPlaces::KingAttackPlaces[square];
                else mask = EvaluationSliderAttacks(square, type, occupancy);
                attacks[square] = mask;
                sideAttacks[side] |= mask;
                if (type != 6) nonKingAttacks[side] |= mask;
                // The existing BoardLogic query requires slider targets to be occupied.
                legacyAttacks[side] |= (type >= 3 && type <= 5) ? mask & occupancy : mask;
            }
    attacksReady = true;
}

namespace
{
int TaperEvaluationValue(int middleGameValue, int endGameValue, int phase)
{
    return (middleGameValue * phase + endGameValue * (24 - phase)) / 24;
}

int TaperGroup1Value(int middleGameValue, int endGameValue, int phase)
{
    static const int weights[25] = {
        10000, 10000, 10000, 10000, 10000, 10000, 10000, 8902, 7804, 6706,
        5609, 4511, 3413, 2844, 2275, 1706, 1138, 569, 0, 0,
        0, 0, 0, 0, 0
    };
    const int clampedPhase = std::clamp(phase, 0, 24);
    const int w = weights[clampedPhase];
    return (middleGameValue * (10000 - w) + endGameValue * w) / 10000;
}

int TaperGroup2Value(int middleGameValue, int endGameValue, int phase)
{
    static const int weights[25] = {
        10000, 10000, 10000, 10000, 10000, 10000, 10000, 9093, 8186, 7279,
        6372, 5465, 4558, 3798, 3039, 2279, 1519, 760, 0, 0,
        0, 0, 0, 0, 0
    };
    const int clampedPhase = std::clamp(phase, 0, 24);
    const int w = weights[clampedPhase];
    return (middleGameValue * (10000 - w) + endGameValue * w) / 10000;
}

int TaperGroup3Value(int middleGameValue, int endGameValue, int phase)
{
    static const int weights[25] = {
        10000, 10000, 10000, 10000, 10000, 10000, 10000, 9177, 8353, 7530,
        6707, 5883, 5060, 4217, 3373, 2530, 1687, 843, 0, 0,
        0, 0, 0, 0, 0
    };
    const int clampedPhase = std::clamp(phase, 0, 24);
    const int w = weights[clampedPhase];
    return (middleGameValue * (10000 - w) + endGameValue * w) / 10000;
}

static constexpr unsigned long long AdjacentFilesMask[8] = {
    0x0101010101010101ULL << 1,
    (0x0101010101010101ULL << 0) | (0x0101010101010101ULL << 2),
    (0x0101010101010101ULL << 1) | (0x0101010101010101ULL << 3),
    (0x0101010101010101ULL << 2) | (0x0101010101010101ULL << 4),
    (0x0101010101010101ULL << 3) | (0x0101010101010101ULL << 5),
    (0x0101010101010101ULL << 4) | (0x0101010101010101ULL << 6),
    (0x0101010101010101ULL << 5) | (0x0101010101010101ULL << 7),
    0x0101010101010101ULL << 6
};

static constexpr unsigned long long RankGeMask[8] = {
    ~0ULL,
    ~0ULL << 8,
    ~0ULL << 16,
    ~0ULL << 24,
    ~0ULL << 32,
    ~0ULL << 40,
    ~0ULL << 48,
    ~0ULL << 56
};

static constexpr unsigned long long RankLeMask[8] = {
    (1ULL << 8) - 1,
    (1ULL << 16) - 1,
    (1ULL << 24) - 1,
    (1ULL << 32) - 1,
    (1ULL << 40) - 1,
    (1ULL << 48) - 1,
    (1ULL << 56) - 1,
    ~0ULL
};

struct PinInfo {
    bool isAligned;
    bool isOrthogonal;
    int stepR;
    int stepC;
};

static constexpr struct PinTable {
    PinInfo data[64][64]{};
    constexpr PinTable() {
        for (int sq1 = 0; sq1 < 64; ++sq1) {
            for (int sq2 = 0; sq2 < 64; ++sq2) {
                if (sq1 == sq2) continue;
                int r1 = sq1 / 8, c1 = sq1 % 8;
                int r2 = sq2 / 8, c2 = sq2 % 8;
                int dr = r2 - r1, dc = c2 - c1;
                if (dr == 0 || dc == 0 || (dr == dc) || (dr == -dc)) {
                    data[sq1][sq2].isAligned = true;
                    data[sq1][sq2].isOrthogonal = (dr == 0 || dc == 0);
                    data[sq1][sq2].stepR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
                    data[sq1][sq2].stepC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
                }
            }
        }
    }
} PinData{};

static long long KnightOutpostChallengeMask[2][64];
static long long KnightOutpostSupportMask[2][64];
static bool KnightOutpostAdvanced[2][64];
static int KnightOutpostFileScale[64];

static void InitializeKnightOutpost()
{
    static bool initialized = false;
    if (!initialized)
    {
        for (int sq = 0; sq < 64; ++sq)
        {
            int r = sq / 8, f = sq % 8;
            KnightOutpostAdvanced[1][sq] = (r >= 3 && r <= 5);
            KnightOutpostAdvanced[0][sq] = (r >= 2 && r <= 4);
            long long ownFile = 0x0101010101010101ULL << f;
            KnightOutpostChallengeMask[1][sq] = PassedPawnSetup::WhitePassedMask[sq] & ~ownFile;
            KnightOutpostChallengeMask[0][sq] = PassedPawnSetup::BlackPassedMask[sq] & ~ownFile;
            KnightOutpostSupportMask[1][sq] = AttackPlaces::BlackPawnAttackPlaces[sq];
            KnightOutpostSupportMask[0][sq] = AttackPlaces::WhitePawnAttackPlaces[sq];
            static const int fileScale[8] = {25, 60, 90, 100, 100, 90, 60, 25};
            KnightOutpostFileScale[sq] = fileScale[f];
        }
        initialized = true;
    }
}

int KnightOutpostValue(const Board& board, int square, bool white, int phase)
{
    InitializeKnightOutpost();
    const int side = white ? 1 : 0;
    if (!KnightOutpostAdvanced[side][square])
        return 0;

    const long long enemyPawns = white ? board.blackPawns : board.whitePawns;
    if ((KnightOutpostChallengeMask[side][square] & enemyPawns) != 0)
        return 0;

    int value = TaperEvaluationValue(
        Option::KnightOutpostMiddleGame,
        Option::KnightOutpostEndGame,
        phase);
    const long long friendlyPawns = white ? board.whitePawns : board.blackPawns;
    if ((KnightOutpostSupportMask[side][square] & friendlyPawns) != 0)
    {
        value += TaperEvaluationValue(
            Option::KnightSupportedOutpostMiddleGame,
            Option::KnightSupportedOutpostEndGame,
            phase);
    }
    return value * KnightOutpostFileScale[square] / 100;
}

bool IsIsolatedPawn(long long friendlyPawns, int square)
{
    return (friendlyPawns & AdjacentFilesMask[square % 8]) == 0;
}

int IsolatedPawnPenalty(int phase)
{
    return TaperEvaluationValue(
        Option::IsolatedPawnMiddleGame,
        Option::IsolatedPawnEndGame,
        phase);
}

inline int ChebyshevDistance(int sq1, int sq2)
{
    return std::max(std::abs((sq1 % 8) - (sq2 % 8)), std::abs((sq1 / 8) - (sq2 / 8)));
}

int EvaluatePassedPawnKingRace(Board &board, int whiteKingSq, int blackKingSq, const EvaluationContext& ctx)
{
    // If either side has queens on the board, direct king race is unrealistic.
    if (board.pieces[5].size() > 0 || board.pieces[13].size() > 0)
    {
        return 0;
    }

    int phase = ctx.phase;
    // King distance influence decreases when remaining material is high.
    if (phase >= 12)
    {
        return 0;
    }

    static const int rankWeight[8] = {0, 0, 0, 1, 5, 8, 14, 22};
    int whiteRaceTempo = (!board.sideToMove) ? 1 : -1;
    int blackRaceTempo = (board.sideToMove) ? 1 : -1;

    int whiteAdjustment = 0;
    for (int i = 0; i < ctx.whitePasserCount; ++i)
    {
        int pawnPlace = ctx.whitePassers[i];
        int blockSq = pawnPlace + 8;
        int promoSq = 56 + (pawnPlace % 8);

        int blockRace = ChebyshevDistance(blackKingSq, blockSq) - ChebyshevDistance(whiteKingSq, blockSq);
        int promotionRace = ChebyshevDistance(blackKingSq, promoSq) - ChebyshevDistance(whiteKingSq, promoSq);

        int mult = rankWeight[(pawnPlace / 8) + 1];
        int adj = (blockRace + promotionRace + whiteRaceTempo) * mult;
        if (adj > 0)
        {
            whiteAdjustment += adj;
        }
    }

    int blackAdjustment = 0;
    for (int i = 0; i < ctx.blackPasserCount; ++i)
    {
        int pawnPlace = ctx.blackPassers[i];
        int blockSq = pawnPlace - 8;
        int promoSq = pawnPlace % 8;

        int blockRace = ChebyshevDistance(whiteKingSq, blockSq) - ChebyshevDistance(blackKingSq, blockSq);
        int promotionRace = ChebyshevDistance(whiteKingSq, promoSq) - ChebyshevDistance(blackKingSq, promoSq);

        int mult = rankWeight[8 - (pawnPlace / 8)];
        int adj = (blockRace + promotionRace + blackRaceTempo) * mult;
        if (adj > 0)
        {
            blackAdjustment += adj;
        }
    }

    int netRace = whiteAdjustment - blackAdjustment;
    int raceValue = (netRace * (12 - phase)) / 12;

    // King blockade of enemy passed pawns
    int whiteBlockade = 0;
    for (int i = 0; i < ctx.blackPasserCount; ++i)
    {
        int pawnPlace = ctx.blackPassers[i];
        int adv = 7 - (pawnPlace / 8);
        int blockSq = pawnPlace - 8;
        if (whiteKingSq == blockSq)
        {
            int bonus = (adv == 3) ? 25 : (adv == 4 ? 220 : (adv == 5 ? 175 : 215));
            whiteBlockade += (bonus * (12 - phase)) / 12;
        }
        else if (ChebyshevDistance(whiteKingSq, blockSq) <= 1 &&
                 ChebyshevDistance(whiteKingSq, blockSq) < ChebyshevDistance(blackKingSq, blockSq))
        {
            int bonus = (adv == 3) ? 20 : (adv == 4 ? 115 : (adv == 5 ? 150 : 180));
            whiteBlockade += (bonus * (12 - phase)) / 12;
        }

        // Isolated blockaded passer containment
        int pFile = pawnPlace % 8;
        bool isIsolated = true;
        for (int pSq : board.pieces[9])
        {
            if (pSq != pawnPlace && std::abs((pSq % 8) - pFile) == 1)
            {
                isIsolated = false;
                break;
            }
        }
        if (isIsolated && (whiteKingSq == blockSq ||
            (ChebyshevDistance(whiteKingSq, blockSq) <= 1 && ChebyshevDistance(whiteKingSq, blockSq) < ChebyshevDistance(blackKingSq, blockSq))))
        {
            int isoBonus = (adv == 3 ? 20 : (adv == 4 ? 45 : 65));
            whiteBlockade += (isoBonus * (12 - phase)) / 12;
        }
    }

    int blackBlockade = 0;
    for (int i = 0; i < ctx.whitePasserCount; ++i)
    {
        int pawnPlace = ctx.whitePassers[i];
        int adv = pawnPlace / 8;
        int blockSq = pawnPlace + 8;
        if (blackKingSq == blockSq)
        {
            int bonus = (adv == 3) ? 25 : (adv == 4 ? 220 : (adv == 5 ? 175 : 215));
            blackBlockade += (bonus * (12 - phase)) / 12;
        }
        else if (ChebyshevDistance(blackKingSq, blockSq) <= 1 &&
                 ChebyshevDistance(blackKingSq, blockSq) < ChebyshevDistance(whiteKingSq, blockSq))
        {
            int bonus = (adv == 3) ? 20 : (adv == 4 ? 115 : (adv == 5 ? 150 : 180));
            blackBlockade += (bonus * (12 - phase)) / 12;
        }

        int pFile = pawnPlace % 8;
        bool isIsolated = true;
        for (int pSq : board.pieces[1])
        {
            if (pSq != pawnPlace && std::abs((pSq % 8) - pFile) == 1)
            {
                isIsolated = false;
                break;
            }
        }
        if (isIsolated && (blackKingSq == blockSq ||
            (ChebyshevDistance(blackKingSq, blockSq) <= 1 && ChebyshevDistance(blackKingSq, blockSq) < ChebyshevDistance(whiteKingSq, blockSq))))
        {
            int isoBonus = (adv == 3 ? 20 : (adv == 4 ? 45 : 65));
            blackBlockade += (isoBonus * (12 - phase)) / 12;
        }
    }

    // --- Coherent Endgame Evaluator Dynamics ---
    int whiteExtra = 0;
    int blackExtra = 0;

    int whiteAdv = 0, blackAdv = 0;
    for (int i = 0; i < ctx.whitePasserCount; ++i) {
        int sq = ctx.whitePassers[i];
        int d = 7 - (sq / 8);
        if (d <= 2) whiteAdv++;
    }
    for (int i = 0; i < ctx.blackPasserCount; ++i) {
        int sq = ctx.blackPassers[i];
        int d = sq / 8;
        if (d <= 2) blackAdv++;
    }

    // 1. Decisive Passed Pawn Conversion
    // A. Dual advanced passers: unstoppable split passer threat (EG1) + sparse material conversion dampening
    if (whiteAdv >= 2 && blackAdv < 2) {
        int bonus = (board.pieces[9].size() > board.pieces[1].size()) ? 510 : 350;
        whiteExtra += (bonus * (12 - phase)) / 12;
    } else if (blackAdv >= 2 && whiteAdv < 2) {
        int bonus = (board.pieces[1].size() > board.pieces[9].size()) ? 510 : 350;
        blackExtra += (bonus * (12 - phase)) / 12;
    }
    // B. Single advanced passer dominance with king interception (Rule of the Square)
    else if (whiteAdv >= 1 && blackAdv == 0) {
        int bonus = 245;
        for (int i = 0; i < ctx.whitePasserCount; ++i) {
            int sq = ctx.whitePassers[i];
            int d = 7 - (sq / 8);
            if (d <= 2) {
                int promoSq = 56 + (sq % 8);
                int bKingDist = ChebyshevDistance(blackKingSq, promoSq);
                int wKingDist = ChebyshevDistance(whiteKingSq, promoSq);
                if (bKingDist <= d && bKingDist < wKingDist) {
                    bonus = 105;
                    break;
                }
            }
        }
        whiteExtra += (bonus * (12 - phase)) / 12;
    } else if (blackAdv >= 1 && whiteAdv == 0) {
        int bonus = 245;
        for (int i = 0; i < ctx.blackPasserCount; ++i) {
            int sq = ctx.blackPassers[i];
            int d = sq / 8;
            if (d <= 2) {
                int promoSq = sq % 8;
                int wKingDist = ChebyshevDistance(whiteKingSq, promoSq);
                int bKingDist = ChebyshevDistance(blackKingSq, promoSq);
                if (wKingDist <= d && wKingDist < bKingDist) {
                    bonus = 105;
                    break;
                }
            }
        }
        blackExtra += (bonus * (12 - phase)) / 12;
    }

    // C. Outside passer on 5th rank (d == 3) when opponent has NO advanced passers
    if (whiteAdv == 0 && blackAdv == 0) {
        int whiteD3Outside = 0, blackD3Outside = 0;
        for (int i = 0; i < ctx.whitePasserCount; ++i) {
            int sq = ctx.whitePassers[i];
            int d = 7 - (sq / 8);
            int f = sq % 8;
            if (d == 3 && (f <= 1 || f >= 5)) whiteD3Outside++;
        }
        for (int i = 0; i < ctx.blackPasserCount; ++i) {
            int sq = ctx.blackPassers[i];
            int d = sq / 8;
            int f = sq % 8;
            if (d == 3 && (f <= 1 || f >= 5)) blackD3Outside++;
        }
        if (blackD3Outside >= 1 && whiteD3Outside == 0) {
            blackExtra += (75 * (12 - phase)) / 12;
        } else if (whiteD3Outside >= 1 && blackD3Outside == 0) {
            whiteExtra += (75 * (12 - phase)) / 12;
        }
    }

    // 2. Rook Restraint & Blockade of enemy passers
    for (int i = 0; i < ctx.blackPasserCount; ++i) {
        int sq = ctx.blackPassers[i];
        int pFile = sq % 8;
        int pRank = sq / 8;
        for (int rSq : board.pieces[4]) {
            if ((rSq % 8) == pFile) {
                int rRank = rSq / 8;
                if (rRank > pRank) {
                    whiteExtra += (65 * (12 - phase)) / 12;
                } else if (rRank < pRank) {
                    whiteExtra += (40 * (12 - phase)) / 12;
                }
            }
        }
    }
    for (int i = 0; i < ctx.whitePasserCount; ++i) {
        int sq = ctx.whitePassers[i];
        int pFile = sq % 8;
        int pRank = sq / 8;
        for (int rSq : board.pieces[12]) {
            if ((rSq % 8) == pFile) {
                int rRank = rSq / 8;
                if (rRank < pRank) {
                    blackExtra += (65 * (12 - phase)) / 12;
                } else if (rRank > pRank) {
                    blackExtra += (40 * (12 - phase)) / 12;
                }
            }
        }
    }

    // 3. Check by rook against enemy king in sparse endgame
    if (phase <= 6) {
        long long occ = board.whitePieces | board.blackPieces;
        for (int rSq : board.pieces[12]) {
            if ((AttackPlaces::RookAttack[rSq][whiteKingSq] & occ) == Option::PowerTwo[whiteKingSq]) {
                blackExtra += (85 * (12 - phase)) / 12;
            }
        }
        for (int rSq : board.pieces[4]) {
            if ((AttackPlaces::RookAttack[rSq][blackKingSq] & occ) == Option::PowerTwo[blackKingSq]) {
                whiteExtra += (85 * (12 - phase)) / 12;
            }
        }
    }

    // 4. Active minor restraint / support in endings:
    if (phase <= 6) {
        if (blackAdv >= 1) {
            for (int bSq : board.pieces[11]) {
                int r = bSq / 8, f = bSq % 8;
                if ((r == 3 || r == 4) && (f >= 2 && f <= 6)) {
                    blackExtra += (110 * (12 - phase)) / 12;
                }
            }
            for (int kSq : board.pieces[10]) {
                int r = kSq / 8, f = kSq % 8;
                if ((r == 3 || r == 4) && (f >= 2 && f <= 5)) {
                    blackExtra += (45 * (12 - phase)) / 12;
                }
            }
        }
        if (whiteAdv >= 1) {
            for (int bSq : board.pieces[3]) {
                int r = bSq / 8, f = bSq % 8;
                if ((r == 3 || r == 4) && (f >= 2 && f <= 6)) {
                    whiteExtra += (110 * (12 - phase)) / 12;
                }
            }
            for (int kSq : board.pieces[2]) {
                int r = kSq / 8, f = kSq % 8;
                if ((r == 3 || r == 4) && (f >= 2 && f <= 5)) {
                    whiteExtra += (45 * (12 - phase)) / 12;
                }
            }
        }
    }

    // 5. Near-endgame king centralization
    if (phase <= 8) {
        int wKf = whiteKingSq % 8, wKr = whiteKingSq / 8;
        if ((wKr == 2 || wKr == 3) && (wKf >= 2 && wKf <= 5)) {
            whiteExtra += (8 * (8 - phase)) / 8;
        }
        int bKf = blackKingSq % 8, bKr = blackKingSq / 8;
        if ((bKr == 5 || bKr == 4) && (bKf >= 2 && bKf <= 5)) {
            blackExtra += (8 * (8 - phase)) / 8;
        }
    }

    return raceValue + (whiteBlockade - blackBlockade) + (whiteExtra - blackExtra);
}

int EvaluatePassedPawnKingRace(Board &board, int whiteKingSq, int blackKingSq)
{
    EvaluationContext ctx(board, EvaluationLogic::CalculatePhase(board));
    return EvaluatePassedPawnKingRace(board, whiteKingSq, blackKingSq, ctx);
}

static int KnightDistance[64][64];
static bool knightDistanceInitialized = false;

static void InitializeKnightDistance()
{
    if (knightDistanceInitialized) return;

    for (int src = 0; src < 64; ++src)
    {
        for (int dst = 0; dst < 64; ++dst)
        {
            KnightDistance[src][dst] = -1;
        }

        int queue[64];
        int head = 0, tail = 0;

        KnightDistance[src][src] = 0;
        queue[tail++] = src;

        while (head < tail)
        {
            int curr = queue[head++];
            int currDist = KnightDistance[src][curr];

            int cf = curr % 8;
            int cr = curr / 8;

            static const int dx[8] = {1, 2, 2, 1, -1, -2, -2, -1};
            static const int dy[8] = {2, 1, -1, -2, -2, -1, 1, 2};

            for (int i = 0; i < 8; ++i)
            {
                int nf = cf + dx[i];
                int nr = cr + dy[i];
                if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8)
                {
                    int nxt = nr * 8 + nf;
                    if (KnightDistance[src][nxt] == -1)
                    {
                        KnightDistance[src][nxt] = currDist + 1;
                        queue[tail++] = nxt;
                    }
                }
            }
        }
    }
    knightDistanceInitialized = true;
}

int EvaluatePassedPawnMinorAccessibility(Board &board, const EvaluationContext* ctx = nullptr)
{
    if (board.pieces[2].empty() && board.pieces[10].empty())
        return 0;
    if (ctx != nullptr && ctx->whitePasserCount == 0 && ctx->blackPasserCount == 0)
        return 0;

    InitializeKnightDistance();

    static const int rankScalePercent[8] = {0, 0, 0, 25, 50, 75, 100, 100};
    const long long occupiedSquares = board.whitePieces | board.blackPieces;

    auto evaluatePasserKnightAccessibility = [&](int pawnPlace, bool passerIsWhite) -> int
    {
        const int sideOffset = passerIsWhite ? 0 : 8;
        const int kingSquare = ctx ? (passerIsWhite ? ctx->whiteKingSq : ctx->blackKingSq)
                                   : board.pieces[sideOffset + 6].front();
        const long long kingAttacks = AttackPlaces::KingAttackPlaces[kingSquare];
        const auto& bishops = board.pieces[sideOffset + 3];

        int8_t controlledCache[64];
        std::fill(std::begin(controlledCache), std::end(controlledCache), -1);

        auto controlledByPasserKingOrBishop = [&](int square) -> bool
        {
            if (controlledCache[square] != -1)
                return controlledCache[square] != 0;

            if ((kingAttacks & Option::PowerTwo[square]) != 0)
            {
                controlledCache[square] = 1;
                return true;
            }

            for (int bishopSquare : bishops)
            {
                if ((AttackPlaces::BishopPseudoAttacks[bishopSquare] & Option::PowerTwo[square]) != 0 &&
                    (AttackPlaces::BetweenMask[bishopSquare][square] & occupiedSquares) == 0)
                {
                    controlledCache[square] = 1;
                    return true;
                }
            }
            controlledCache[square] = 0;
            return false;
        };

        auto knightCorridorValue = [&](int knightSq, bool knightIsWhite) -> int
        {
            if (knightIsWhite == passerIsWhite)
            {
                int minDist = 99;
                const int direction = passerIsWhite ? 8 : -8;
                for (int square = pawnPlace + direction;
                     square >= 0 && square < 64;
                     square += direction)
                {
                    minDist = std::min(minDist, KnightDistance[knightSq][square]);
                }
                if (minDist == 0) return 20;
                if (minDist == 1) return 0;
                if (minDist == 2) return 10;
                if (minDist == 3) return 5;
                return 0;
            }

            int bestValue = 0;
            bool hasTimelyUsefulDestination = false;
            bool hasMaintainableDestination = false;
            const int direction = passerIsWhite ? 8 : -8;
            int pawnMoves = 1;
            for (int square = pawnPlace + direction;
                 square >= 0 && square < 64;
                 square += direction, pawnMoves++)
            {
                const int distance = KnightDistance[knightSq][square];
                const int knightArrivalPly = 2 * distance -
                    ((board.sideToMove == !knightIsWhite) ? 1 : 0);
                const int pawnArrivalPly = 2 * pawnMoves -
                    ((board.sideToMove == !passerIsWhite) ? 1 : 0);

                int value = 0;
                if (distance == 0) value = 20;
                else if (distance == 1) value = 15;
                else if (distance == 2) value = 10;
                else if (distance == 3) value = 5;

                if (knightArrivalPly > pawnArrivalPly)
                    value = 0;

                if (value > 0)
                {
                    hasTimelyUsefulDestination = true;
                    if (!controlledByPasserKingOrBishop(square))
                        hasMaintainableDestination = true;

                    int memo[64];
                    std::fill(std::begin(memo), std::end(memo), -1);
                    auto minimumControlledLandings = [&](auto& self, int from) -> int
                    {
                        if (from == square)
                            return 0;
                        int &cached = memo[from];
                        if (cached >= 0)
                            return cached;
                        cached = 64;
                        const int remainingDistance = KnightDistance[from][square];
                        long long hops = AttackPlaces::KnightAttackPlaces[from];
                        while (hops)
                        {
                            int hop = __builtin_ctzll(hops);
                            hops &= hops - 1;
                            if (KnightDistance[hop][square] != remainingDistance - 1)
                                continue;
                            const int controlledLanding =
                                controlledByPasserKingOrBishop(hop) ? 1 : 0;
                            cached = std::min(cached,
                                controlledLanding + self(self, hop));
                        }
                        return cached;
                    };
                    const int controlledLandings = minimumControlledLandings(minimumControlledLandings, knightSq);
                    value = value * std::max(1, 4 - controlledLandings) / 4;
                }
                bestValue = std::max(bestValue, value);
            }
            if (hasTimelyUsefulDestination && !hasMaintainableDestination)
                return 0;
            return bestValue;
        };

        int accessibility = 0;
        if (passerIsWhite)
        {
            for (int kSq : board.pieces[2])
            {
                accessibility += knightCorridorValue(kSq, true);
            }
            for (int kSq : board.pieces[10])
            {
                accessibility -= knightCorridorValue(kSq, false);
            }
        }
        else
        {
            for (int kSq : board.pieces[10])
            {
                accessibility += knightCorridorValue(kSq, false);
            }
            for (int kSq : board.pieces[2])
            {
                accessibility -= knightCorridorValue(kSq, true);
            }
        }
        return accessibility;
    };

    int whiteNet = 0;
    if (ctx != nullptr)
    {
        for (int i = 0; i < ctx->whitePasserCount; ++i)
        {
            int pawnPlace = ctx->whitePassers[i];
            int accessibility = evaluatePasserKnightAccessibility(pawnPlace, true);
            int relRank = (pawnPlace / 8) + 1;
            whiteNet += (accessibility * rankScalePercent[relRank]) / 100;
        }
    }
    else
    {
        for (int pawnPlace : board.pieces[1])
        {
            if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & board.blackPawns) == 0)
            {
                int accessibility = evaluatePasserKnightAccessibility(pawnPlace, true);
                int relRank = (pawnPlace / 8) + 1;
                whiteNet += (accessibility * rankScalePercent[relRank]) / 100;
            }
        }
    }

    int blackNet = 0;
    if (ctx != nullptr)
    {
        for (int i = 0; i < ctx->blackPasserCount; ++i)
        {
            int pawnPlace = ctx->blackPassers[i];
            int accessibility = evaluatePasserKnightAccessibility(pawnPlace, false);
            int relRank = 8 - (pawnPlace / 8);
            blackNet += (accessibility * rankScalePercent[relRank]) / 100;
        }
    }
    else
    {
        for (int pawnPlace : board.pieces[9])
        {
            if ((PassedPawnSetup::BlackPassedMask[pawnPlace] & board.whitePawns) == 0)
            {
                int accessibility = evaluatePasserKnightAccessibility(pawnPlace, false);
                int relRank = 8 - (pawnPlace / 8);
                blackNet += (accessibility * rankScalePercent[relRank]) / 100;
            }
        }
    }

    return whiteNet - blackNet;
}

static bool IsSquareAttackedBySide(Board &board, int sq, bool byWhite)
{
    long long wholeBoardWithTarget = (board.whitePieces | board.blackPieces) | Option::PowerTwo[sq];
    int pawnPiece = byWhite ? 1 : 9;
    int knightPiece = byWhite ? 2 : 10;
    int bishopPiece = byWhite ? 3 : 11;
    int rookPiece = byWhite ? 4 : 12;
    int queenPiece = byWhite ? 5 : 13;
    int kingPiece = byWhite ? 6 : 14;

    if (byWhite)
    {
        for (int pSq : board.pieces[pawnPiece])
        {
            if ((AttackPlaces::WhitePawnAttackPlaces[pSq] & Option::PowerTwo[sq]) != 0)
            {
                return true;
            }
        }
    }
    else
    {
        for (int pSq : board.pieces[pawnPiece])
        {
            if ((AttackPlaces::BlackPawnAttackPlaces[pSq] & Option::PowerTwo[sq]) != 0)
            {
                return true;
            }
        }
    }

    for (int kSq : board.pieces[knightPiece])
    {
        if ((AttackPlaces::KnightAttackPlaces[kSq] & Option::PowerTwo[sq]) != 0)
        {
            return true;
        }
    }

    for (int kSq : board.pieces[kingPiece])
    {
        if ((AttackPlaces::KingAttackPlaces[kSq] & Option::PowerTwo[sq]) != 0)
        {
            return true;
        }
    }

    for (int bSq : board.pieces[bishopPiece])
    {
        if ((AttackPlaces::BishopAttack[bSq][sq] & wholeBoardWithTarget) == Option::PowerTwo[sq])
        {
            return true;
        }
    }

    for (int rSq : board.pieces[rookPiece])
    {
        if ((AttackPlaces::RookAttack[rSq][sq] & wholeBoardWithTarget) == Option::PowerTwo[sq])
        {
            return true;
        }
    }

    for (int qSq : board.pieces[queenPiece])
    {
        if ((AttackPlaces::QueenAttack[qSq][sq] & wholeBoardWithTarget) == Option::PowerTwo[sq])
        {
            return true;
        }
    }

    return false;
}

static int CountSquareAttacksBySide(Board &board, int sq, bool byWhite, bool friendlySliderBehindFile, int pawnFile)
{
    long long wholeBoardWithTarget = (board.whitePieces | board.blackPieces) | Option::PowerTwo[sq];
    int pawnPiece = byWhite ? 1 : 9;
    int knightPiece = byWhite ? 2 : 10;
    int bishopPiece = byWhite ? 3 : 11;
    int rookPiece = byWhite ? 4 : 12;
    int queenPiece = byWhite ? 5 : 13;
    int kingPiece = byWhite ? 6 : 14;

    int attacks = 0;

    if (byWhite)
    {
        for (int pSq : board.pieces[pawnPiece])
        {
            if ((AttackPlaces::WhitePawnAttackPlaces[pSq] & Option::PowerTwo[sq]) != 0)
            {
                attacks++;
            }
        }
    }
    else
    {
        for (int pSq : board.pieces[pawnPiece])
        {
            if ((AttackPlaces::BlackPawnAttackPlaces[pSq] & Option::PowerTwo[sq]) != 0)
            {
                attacks++;
            }
        }
    }

    for (int kSq : board.pieces[knightPiece])
    {
        if ((AttackPlaces::KnightAttackPlaces[kSq] & Option::PowerTwo[sq]) != 0)
        {
            attacks++;
        }
    }

    for (int kSq : board.pieces[kingPiece])
    {
        if ((AttackPlaces::KingAttackPlaces[kSq] & Option::PowerTwo[sq]) != 0)
        {
            attacks++;
        }
    }

    for (int bSq : board.pieces[bishopPiece])
    {
        if ((AttackPlaces::BishopAttack[bSq][sq] & wholeBoardWithTarget) == Option::PowerTwo[sq])
        {
            attacks++;
        }
    }

    for (int rSq : board.pieces[rookPiece])
    {
        if ((AttackPlaces::RookAttack[rSq][sq] & wholeBoardWithTarget) == Option::PowerTwo[sq])
        {
            attacks++;
        }
        else if (friendlySliderBehindFile && (rSq % 8 == pawnFile) && (sq % 8 == pawnFile))
        {
            // Ray through friendly passer along the same file
            int rRank = rSq / 8;
            int sqRank = sq / 8;
            bool unobstructed = true;
            int step = (sqRank > rRank) ? 8 : -8;
            for (int scanSq = rSq + step; scanSq != sq; scanSq += step)
            {
                int occ = board.mainBoard[scanSq];
                if (occ != 0 && occ != (byWhite ? 1 : 9))
                {
                    unobstructed = false;
                    break;
                }
            }
            if (unobstructed)
            {
                attacks++;
            }
        }
    }

    for (int qSq : board.pieces[queenPiece])
    {
        if ((AttackPlaces::QueenAttack[qSq][sq] & wholeBoardWithTarget) == Option::PowerTwo[sq])
        {
            attacks++;
        }
        else if (friendlySliderBehindFile && (qSq % 8 == pawnFile) && (sq % 8 == pawnFile))
        {
            int qRank = qSq / 8;
            int sqRank = sq / 8;
            bool unobstructed = true;
            int step = (sqRank > qRank) ? 8 : -8;
            for (int scanSq = qSq + step; scanSq != sq; scanSq += step)
            {
                int occ = board.mainBoard[scanSq];
                if (occ != 0 && occ != (byWhite ? 1 : 9))
                {
                    unobstructed = false;
                    break;
                }
            }
            if (unobstructed)
            {
                attacks++;
            }
        }
    }

    return attacks;
}

int EvaluatePassedPawnCorridorSafety(Board &board, const EvaluationContext* ctx = nullptr)
{
    static const int rankScalePercent[8] = {0, 0, 0, 25, 50, 75, 100, 100};

    auto evalPasserWhite = [&](int pawnPlace) -> int {
        int pFile = pawnPlace % 8;
        int pRank = pawnPlace / 8;

        // Check if friendly heavy piece is behind passer on the same file
        bool sliderBehind = false;
        for (int r = pRank - 1; r >= 0; r--)
        {
            int occ = board.mainBoard[r * 8 + pFile];
            if (occ != 0)
            {
                if (occ == 4 || occ == 5) sliderBehind = true;
                break;
            }
        }

        int corridorControl = 0;
        for (int sq = pawnPlace + 8; sq < 64; sq += 8)
        {
            bool isPromo = (sq >= 56);
            int friendly = CountSquareAttacksBySide(board, sq, true, sliderBehind, pFile);
            int enemy = CountSquareAttacksBySide(board, sq, false, false, pFile);

            if (friendly > enemy)
            {
                int base = isPromo ? 24 : 10;
                int extra = (friendly - enemy - 1) * (isPromo ? 14 : 6);
                corridorControl += (base + extra);
            }
            else if (enemy > friendly)
            {
                int base = isPromo ? 20 : 10;
                int extra = (enemy - friendly - 1) * (isPromo ? 10 : 6);
                corridorControl -= (base + extra);
            }
            else if (friendly > 0 && enemy > 0)
            {
                corridorControl -= (isPromo ? 4 : 2);
            }
        }

        if (corridorControl < 0) corridorControl = 0;
        int relRank = pRank + 1;
        int scaled = (corridorControl * rankScalePercent[relRank]) / 100;
        if (scaled < -80) scaled = -80;
        if (scaled > 80) scaled = 80;
        return scaled;
    };

    int whiteTotal = 0;
    if (ctx != nullptr)
    {
        for (int i = 0; i < ctx->whitePasserCount; ++i)
        {
            whiteTotal += evalPasserWhite(ctx->whitePassers[i]);
        }
    }
    else
    {
        for (int pawnPlace : board.pieces[1])
        {
            if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & board.blackPawns) == 0)
            {
                whiteTotal += evalPasserWhite(pawnPlace);
            }
        }
    }

    auto evalPasserBlack = [&](int pawnPlace) -> int {
        int pFile = pawnPlace % 8;
        int pRank = pawnPlace / 8;

        // Check if friendly heavy piece is behind passer on the same file
        bool sliderBehind = false;
        for (int r = pRank + 1; r < 8; r++)
        {
            int occ = board.mainBoard[r * 8 + pFile];
            if (occ != 0)
            {
                if (occ == 12 || occ == 13) sliderBehind = true;
                break;
            }
        }

        int corridorControl = 0;
        for (int sq = pawnPlace - 8; sq >= 0; sq -= 8)
        {
            bool isPromo = (sq < 8);
            int friendly = CountSquareAttacksBySide(board, sq, false, sliderBehind, pFile);
            int enemy = CountSquareAttacksBySide(board, sq, true, false, pFile);

            if (friendly > enemy)
            {
                int base = isPromo ? 24 : 10;
                int extra = (friendly - enemy - 1) * (isPromo ? 14 : 6);
                corridorControl += (base + extra);
            }
            else if (enemy > friendly)
            {
                int base = isPromo ? 20 : 10;
                int extra = (enemy - friendly - 1) * (isPromo ? 10 : 6);
                corridorControl -= (base + extra);
            }
            else if (friendly > 0 && enemy > 0)
            {
                corridorControl -= (isPromo ? 4 : 2);
            }
        }

        if (corridorControl < 0) corridorControl = 0;
        int relRank = 8 - pRank;
        int scaled = (corridorControl * rankScalePercent[relRank]) / 100;
        if (scaled < -80) scaled = -80;
        if (scaled > 80) scaled = 80;
        return scaled;
    };

    int blackTotal = 0;
    if (ctx != nullptr)
    {
        for (int i = 0; i < ctx->blackPasserCount; ++i)
        {
            blackTotal += evalPasserBlack(ctx->blackPassers[i]);
        }
    }
    else
    {
        for (int pawnPlace : board.pieces[9])
        {
            if ((PassedPawnSetup::BlackPassedMask[pawnPlace] & board.whitePawns) == 0)
            {
                blackTotal += evalPasserBlack(pawnPlace);
            }
        }
    }

    return whiteTotal - blackTotal;
}

bool RooksAreConnected(int firstRook, int secondRook, long long occupiedSquares)
{
    return (AttackPlaces::RookAttack[firstRook][secondRook] & occupiedSquares) ==
        Option::PowerTwo[secondRook];
}

bool HasConnectedRooks(const MyList& rooks, long long occupiedSquares)
{
    const int rookCount = rooks.size();
    if (rookCount < 2)
    {
        return false;
    }

    if (RooksAreConnected(rooks[0], rooks[1], occupiedSquares))
    {
        return true;
    }
    if (rookCount == 2)
    {
        return false;
    }

    for (int first = 0; first < rookCount - 1; first++)
    {
        const int firstUncheckedSecond = first == 0 ? 2 : first + 1;
        for (int second = firstUncheckedSecond; second < rookCount; second++)
        {
            if (RooksAreConnected(rooks[first], rooks[second], occupiedSquares))
            {
                return true;
            }
        }
    }
    return false;
}

int RookConnectionValue(MyList (&pieces)[15], long long occupiedSquares)
{
    int value = 0;
    if (HasConnectedRooks(pieces[4], occupiedSquares))
    {
        value += 30;
    }
    if (HasConnectedRooks(pieces[12], occupiedSquares))
    {
        value -= 30;
    }
    return value;
}

int RookBehindPassedPawnValue(Board& board, int phase, const EvaluationContext* ctx = nullptr)
{
    const long long occupiedSquares = board.whitePieces | board.blackPieces;
    const int bonus = TaperEvaluationValue(
        Option::RookBehindPassedPawnMiddleGame,
        Option::RookBehindPassedPawnEndGame,
        phase);
    int value = 0;

    if (ctx != nullptr)
    {
        for (int i = 0; i < ctx->whitePasserCount; ++i)
        {
            int pawn = ctx->whitePassers[i];
            for (int rook : board.pieces[4])
                if (rook % 8 == pawn % 8 && rook < pawn && RooksAreConnected(rook, pawn, occupiedSquares))
                    value += bonus;
            for (int rook : board.pieces[12])
                if (rook % 8 == pawn % 8 && rook < pawn && RooksAreConnected(rook, pawn, occupiedSquares))
                    value -= bonus;
        }
        for (int i = 0; i < ctx->blackPasserCount; ++i)
        {
            int pawn = ctx->blackPassers[i];
            for (int rook : board.pieces[12])
                if (rook % 8 == pawn % 8 && rook > pawn && RooksAreConnected(rook, pawn, occupiedSquares))
                    value -= bonus;
            for (int rook : board.pieces[4])
                if (rook % 8 == pawn % 8 && rook > pawn && RooksAreConnected(rook, pawn, occupiedSquares))
                    value += bonus;
        }
    }
    else
    {
        for (int pawn : board.pieces[1])
        {
            if ((PassedPawnSetup::WhitePassedMask[pawn] & board.blackPawns) != 0)
                continue;
            for (int rook : board.pieces[4])
                if (rook % 8 == pawn % 8 && rook < pawn && RooksAreConnected(rook, pawn, occupiedSquares))
                    value += bonus;
            for (int rook : board.pieces[12])
                if (rook % 8 == pawn % 8 && rook < pawn && RooksAreConnected(rook, pawn, occupiedSquares))
                    value -= bonus;
        }
        for (int pawn : board.pieces[9])
        {
            if ((PassedPawnSetup::BlackPassedMask[pawn] & board.whitePawns) != 0)
                continue;
            for (int rook : board.pieces[12])
                if (rook % 8 == pawn % 8 && rook > pawn && RooksAreConnected(rook, pawn, occupiedSquares))
                    value -= bonus;
            for (int rook : board.pieces[4])
                if (rook % 8 == pawn % 8 && rook > pawn && RooksAreConnected(rook, pawn, occupiedSquares))
                    value += bonus;
        }
    }
    return value;
}

struct KingDangerResult
{
    int danger = 0;
    int attackerWeight = 0;
    int defenderWeight = 0;
    int escapeSafety = 0;
    int filePressure = 0;
    int diagonalPressure = 0;
    int pawnShelter = 0;
    int phaseScale = 0;
};

bool IsInsideBoard(int rank, int file)
{
    return rank >= 0 && rank < 8 && file >= 0 && file < 8;
}

struct PrecomputedKingZone {
    int count = 0;
    int squares[9]{};
    long long mask = 0;
};

struct KingZonesTable {
    PrecomputedKingZone zones[64];
    constexpr KingZonesTable() {
        for (int ksq = 0; ksq < 64; ksq++) {
            const int kingRank = ksq / 8;
            const int kingFile = ksq % 8;
            zones[ksq].squares[0] = ksq;
            zones[ksq].count = 1;
            zones[ksq].mask = (1ULL << ksq);
            for (int rankOffset = -1; rankOffset <= 1; rankOffset++) {
                for (int fileOffset = -1; fileOffset <= 1; fileOffset++) {
                    if (rankOffset == 0 && fileOffset == 0) continue;
                    const int rank = kingRank + rankOffset;
                    const int file = kingFile + fileOffset;
                    if (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
                        const int target = rank * 8 + file;
                        zones[ksq].squares[zones[ksq].count++] = target;
                        zones[ksq].mask |= (1ULL << target);
                    }
                }
            }
        }
    }
};

static constexpr KingZonesTable KingZonesData{};

struct PawnAttacksTable {
    long long data[2][64]{};
    constexpr PawnAttacksTable() {
        for (int sq = 0; sq < 64; sq++) {
            const int r = sq / 8, f = sq % 8;
            // White (attacks rank + 1)
            if (r < 7) {
                if (f > 0) data[1][sq] |= (1ULL << ((r + 1) * 8 + (f - 1)));
                if (f < 7) data[1][sq] |= (1ULL << ((r + 1) * 8 + (f + 1)));
            }
            // Black (attacks rank - 1)
            if (r > 0) {
                if (f > 0) data[0][sq] |= (1ULL << ((r - 1) * 8 + (f - 1)));
                if (f < 7) data[0][sq] |= (1ULL << ((r - 1) * 8 + (f + 1)));
            }
        }
    }
};

static constexpr PawnAttacksTable PawnAttacksData{};

inline bool PieceAttacksSquareFast(long long occupiedSquares, int pieceType, bool white,
                                   int from, int target)
{
    if (pieceType == 1)
    {
        return (PawnAttacksData.data[white ? 1 : 0][from] & Option::PowerTwo[target]) != 0;
    }
    if (pieceType == 2)
    {
        return (AttackPlaces::KnightAttackPlaces[from] & Option::PowerTwo[target]) != 0;
    }
    if (pieceType == 6)
    {
        return (AttackPlaces::KingAttackPlaces[from] & Option::PowerTwo[target]) != 0;
    }
    long long ray = 0;
    if (pieceType == 3)
    {
        ray = AttackPlaces::BishopAttack[from][target];
    }
    else if (pieceType == 4)
    {
        ray = AttackPlaces::RookAttack[from][target];
    }
    else if (pieceType == 5)
    {
        ray = AttackPlaces::QueenAttack[from][target];
    }
    if (ray == 0)
    {
        return false;
    }
    return (AttackPlaces::BetweenMask[from][target] & occupiedSquares) == 0;
}

bool PieceAttacksSquare(Board& board, int pieceType, bool white,
                        int from, int target)
{
    const long long occupiedSquares = board.whitePieces | board.blackPieces;
    return PieceAttacksSquareFast(occupiedSquares, pieceType, white, from, target);
}

inline bool HasSideAttack(Board& board, bool white, int target, long long occupiedSquares)
{
    const int firstPiece = white ? 1 : 9;
    for (int boardPiece = firstPiece; boardPiece < firstPiece + 6; boardPiece++)
    {
        const int pieceType = white ? boardPiece : boardPiece - 8;
        for (int from : board.pieces[boardPiece])
        {
            if (PieceAttacksSquareFast(occupiedSquares, pieceType, white, from, target))
            {
                return true;
            }
        }
    }
    return false;
}

int CountSideAttacks(Board& board, bool white, int target)
{
    int attackers = 0;
    const long long occupiedSquares = board.whitePieces | board.blackPieces;
    const int firstPiece = white ? 1 : 9;
    for (int boardPiece = firstPiece; boardPiece < firstPiece + 6; boardPiece++)
    {
        const int pieceType = white ? boardPiece : boardPiece - 8;
        for (int from : board.pieces[boardPiece])
        {
            if (PieceAttacksSquareFast(occupiedSquares, pieceType, white, from, target))
            {
                attackers++;
            }
        }
    }
    return attackers;
}

int CountNonKingSideAttacks(Board& board, bool white, int target)
{
    int attackers = 0;
    const long long occupiedSquares = board.whitePieces | board.blackPieces;
    const int firstPiece = white ? 1 : 9;
    for (int boardPiece = firstPiece; boardPiece < firstPiece + 5; boardPiece++)
    {
        const int pieceType = white ? boardPiece : boardPiece - 8;
        for (int from : board.pieces[boardPiece])
            attackers += PieceAttacksSquareFast(occupiedSquares, pieceType, white, from, target) ? 1 : 0;
    }
    return attackers;
}

int UndefendedKingZoneDanger(Board& board, bool whiteKing, int kingSquare)
{
    constexpr int UndefendedSquareDanger = 2;
    constexpr int AdditionalAttackerDanger = 1;
    const bool attackingWhite = !whiteKing;
    const PrecomputedKingZone& zone = KingZonesData.zones[kingSquare];
    int danger = 0;
    for (int i = 1; i < zone.count; ++i)
    {
        const int target = zone.squares[i];
        const int enemyAttacks = CountSideAttacks(board, attackingWhite, target);
        const int friendlyDefenses = CountNonKingSideAttacks(board, whiteKing, target);
        if (enemyAttacks > 0 && friendlyDefenses == 0)
        {
            danger += UndefendedSquareDanger;
            danger += (enemyAttacks - 1) * AdditionalAttackerDanger;
        }
    }
    return danger;
}

std::vector<int> KingZone(int kingSquare)
{
    const auto& z = KingZonesData.zones[kingSquare];
    return std::vector<int>(z.squares, z.squares + z.count);
}

inline bool PieceParticipatesInZoneFast(long long occupiedSquares, int boardPiece, int square,
                                       const PrecomputedKingZone& zone)
{
    const bool white = boardPiece < 8;
    const int pieceType = white ? boardPiece : boardPiece - 8;
    if (pieceType == 1)
    {
        return (PawnAttacksData.data[white ? 1 : 0][square] & zone.mask) != 0;
    }
    if (pieceType == 2)
    {
        return (AttackPlaces::KnightAttackPlaces[square] & zone.mask) != 0;
    }
    for (int i = 0; i < zone.count; i++)
    {
        if (PieceAttacksSquareFast(occupiedSquares, pieceType, white, square, zone.squares[i]))
        {
            return true;
        }
    }
    return false;
}

bool PieceParticipatesInZone(Board& board, int boardPiece, int square,
                             const std::vector<int>& zone)
{
    const bool white = boardPiece < 8;
    const int pieceType = white ? boardPiece : boardPiece - 8;
    const long long occupiedSquares = board.whitePieces | board.blackPieces;
    for (int target : zone)
    {
        if (PieceAttacksSquareFast(occupiedSquares, pieceType, white, square, target))
        {
            return true;
        }
    }
    return false;
}

int ShelterDanger(Board& board, bool whiteKing, int kingSquare, const EvaluationContext* ctx = nullptr)
{
    const int direction = whiteKing ? 1 : -1;
    const int kingRank = kingSquare / 8;
    const int kingFile = kingSquare % 8;
    MyList& friendlyPawns = board.pieces[whiteKing ? 1 : 9];
    MyList& enemyPawns = board.pieces[whiteKing ? 9 : 1];
    const uint8_t enemyPawnFiles = ctx ? (whiteKing ? ctx->blackPawnFiles : ctx->whitePawnFiles) : 0;
    int danger = 0;

    for (int file = std::max(0, kingFile - 1); file <= std::min(7, kingFile + 1); file++)
    {
        bool firstRankPawn = false;
        bool secondRankPawn = false;
        bool fartherPawn = false;
        for (int pawn : friendlyPawns)
        {
            if (pawn % 8 != file)
            {
                continue;
            }
            const int advance = (pawn / 8 - kingRank) * direction;
            firstRankPawn |= advance == 1;
            secondRankPawn |= advance == 2;
            fartherPawn |= advance > 2;
        }
        if (firstRankPawn)
        {
            continue;
        }
        danger += secondRankPawn ? 5 : (fartherPawn ? 10 : 16);

        bool fileHasEnemyPawn = false;
        if (ctx)
        {
            fileHasEnemyPawn = (enemyPawnFiles & (1 << file)) != 0;
        }
        else
        {
            for (int pawn : enemyPawns)
            {
                fileHasEnemyPawn |= pawn % 8 == file;
            }
        }
        if (!secondRankPawn && !fartherPawn && !fileHasEnemyPawn)
        {
            danger += 7;
        }
    }
    return danger;
}

KingDangerResult EvaluateKingDanger(Board& board, bool whiteKing, const EvaluationContext* ctx = nullptr,
                                    bool needDetails = true)
{
    if (ctx == nullptr || !ctx->attacksReady)
    {
        EvaluationContext local(board, ctx ? ctx->phase : EvaluationLogic::CalculatePhase(board));
        local.InitializeAttacks();
        return EvaluateKingDanger(board, whiteKing, &local, needDetails);
    }
    static constexpr int attackerWeight[7] = {0, 2, 5, 5, 8, 12, 0};
    static constexpr int defenderWeight[7] = {0, 2, 4, 4, 5, 7, 0};
    const int kingSquare = whiteKing ? ctx->whiteKingSq : ctx->blackKingSq;
    const bool attackingWhite = !whiteKing;
    const PrecomputedKingZone& zone = KingZonesData.zones[kingSquare];
    // With no participating attacker, the old escalation returns zero. Only
    // shelter is consumed by other score terms; detailed reporting still runs fully.
    if (!needDetails && (ctx->nonKingAttacks[attackingWhite ? 0 : 1] & zone.mask) == 0)
    {
        KingDangerResult result;
        result.pawnShelter = ShelterDanger(board, whiteKing, kingSquare, ctx);
        return result;
    }
    const int kingFile = kingSquare % 8;
    const int minFile = std::max(0, kingFile - 1);
    const int maxFile = std::min(7, kingFile + 1);
    const uint8_t friendlyPawnFiles = whiteKing ? ctx->whitePawnFiles : ctx->blackPawnFiles;
    const uint8_t enemyPawnFiles = whiteKing ? ctx->blackPawnFiles : ctx->whitePawnFiles;
    int fileOpenness[8] = {};
    for (int file = minFile; file <= maxFile; ++file)
        fileOpenness[file] = (friendlyPawnFiles & (1 << file)) ? 0 :
                            ((enemyPawnFiles & (1 << file)) ? 1 : 2);

    int attackerParticipation = 0, defenderParticipation = 0;
    int attackerCount = 0, defenderCount = 0, loneAttackerSq = -1;
    int filePressure = 0, diagonalPressure = 0;
    const int enemySide = attackingWhite ? 0 : 1;
    const int ownSide = whiteKing ? 0 : 1;
    const uint64_t neighbours = zone.mask & ~Option::PowerTwo[kingSquare];
    const uint64_t enemyControl = ctx->sideAttacks[enemySide];
    const uint64_t undefended = neighbours & ~ctx->nonKingAttacks[ownSide];
    int undefendedKingZoneDanger = __builtin_popcountll(undefended & enemyControl);
    const uint64_t attackingPawnAttacks = ctx->pawnAttacks[enemySide];
    uint64_t restrictedBetweenSquares = 0;
    uint64_t queenHits = 0, rookHits = 0;

    for (int type = 1; type <= 5; ++type)
    {
        for (int square : board.pieces[enemySide * 8 + type])
        {
            const uint64_t hits = ctx->attacks[square] & zone.mask;
            if (!hits) continue;
            attackerParticipation += attackerWeight[type];
            ++attackerCount;
            loneAttackerSq = square;
            if (hits & undefended)
                undefendedKingZoneDanger += __builtin_popcountll(hits & undefended);
            if (type >= 3)
            {
                uint64_t targets = hits & neighbours;
                while (targets)
                {
                    const int target = __builtin_ctzll(targets);
                    targets &= targets - 1;
                    restrictedBetweenSquares |= AttackPlaces::BetweenMask[square][target] & attackingPawnAttacks;
                }
                if ((type == 3 || type == 5) && (hits & Option::PowerTwo[kingSquare]))
                    diagonalPressure += 9;
            }
            if (type >= 4)
            {
                for (int file = minFile; file <= maxFile; ++file)
                    if (fileOpenness[file] && (hits & (0x0101010101010101ULL << file)))
                        filePressure += fileOpenness[file] == 2 ? 10 : 6;
                if (type == 4) rookHits |= hits;
                else queenHits |= hits;
            }
        }
        for (int square : board.pieces[ownSide * 8 + type])
            if (ctx->attacks[square] & zone.mask)
            {
                defenderParticipation += defenderWeight[type];
                ++defenderCount;
            }
    }
    const int enemyKingSquare = attackingWhite ? ctx->whiteKingSq : ctx->blackKingSq;
    undefendedKingZoneDanger += __builtin_popcountll(ctx->attacks[enemyKingSquare] & undefended);
    // On an undefended square, the old 2 + (attacks - 1) is attacks + 1.
    // The union supplies the +1; each attacking piece supplies its own count.
    const uint64_t ownOccupancy = whiteKing ? board.whitePieces : board.blackPieces;
    const int occupiedEscapes = __builtin_popcountll(neighbours & ownOccupancy);
    const int controlledEscapes = __builtin_popcountll(neighbours & ~ownOccupancy & enemyControl);
    const int safeEscapes = zone.count - 1 - occupiedEscapes - controlledEscapes;
    const int edgeDirections = 9 - zone.count;
    const int phaseVal = ctx->phase;
    const bool hasHeavyMatingBattery = phaseVal >= 12 && (queenHits & rookHits & undefended) != 0;
    const int escapeDanger = controlledEscapes * 6 + occupiedEscapes * 2 +
                             edgeDirections * 2 + std::max(0, 3 - safeEscapes) * 8;
    const int balanceDanger = std::max(0, attackerParticipation - defenderParticipation) +
                              std::max(0, attackerCount - defenderCount) * 4;
    const int shelterDanger = ShelterDanger(board, whiteKing, kingSquare, ctx);
    const int lineDanger = filePressure + diagonalPressure;
    const int defensiveRestriction = __builtin_popcountll(restrictedBetweenSquares) * 6;
    int rawDanger = attackerParticipation * 2 + escapeDanger +
                    lineDanger + shelterDanger + balanceDanger + undefendedKingZoneDanger +
                    defensiveRestriction + (hasHeavyMatingBattery ? 140 : 0);

    const int queenCount = board.pieces[attackingWhite ? 5 : 13].size();
    const int rookCount = board.pieces[attackingWhite ? 4 : 12].size();
    const int minorCount = board.pieces[attackingWhite ? 2 : 10].size() +
                           board.pieces[attackingWhite ? 3 : 11].size();
    const int base = (queenCount > 0) ? 20 : (20 * phaseVal / 24);
    int attackingMaterialScale = std::min(100, base + queenCount * 45 +
                                                     rookCount * 12 + minorCount * 5);
    if (queenCount == 0)
    {
        attackingMaterialScale = attackingMaterialScale * phaseVal / 24;
    }
    rawDanger = rawDanger * attackingMaterialScale / 100;
    int escalatedDanger = 0;
    if (attackerCount >= 2)
    {
        const int divisor = 180 + defenderParticipation * 4;
        escalatedDanger = rawDanger + (rawDanger * rawDanger) / divisor;
    }
    else if (attackerCount == 1)
    {
        const int rank = kingSquare / 8;
        const int file = kingSquare % 8;
        const bool centralKing = (whiteKing ? rank <= 1 : rank >= 6) && file >= 2 && file <= 5;
        if (attackerParticipation >= 8)
        {
            // Lone major piece creating forcing pressure
            const int attackerRank = loneAttackerSq >= 0 ? loneAttackerSq / 8 : -1;
            const bool infiltrated = whiteKing ? (attackerRank <= 1) : (attackerRank >= 6);
            if (infiltrated && (centralKing || shelterDanger >= 10 || undefendedKingZoneDanger > 0))
            {
                escalatedDanger = (rawDanger * 7) / 8;
            }
            else
            {
                escalatedDanger = rawDanger / 2;
            }
        }
        else if (attackerParticipation >= 4)
        {
            // Single minor piece creating latent pressure against an exposed/central king, weak shelter, or undefended zone
            const bool attackerContested = loneAttackerSq >= 0 &&
                (ctx->legacyAttacks[whiteKing ? 0 : 1] & Option::PowerTwo[loneAttackerSq]);
            if (!attackerContested)
            {
                if (centralKing || shelterDanger >= 10 || undefendedKingZoneDanger > 0)
                {
                    escalatedDanger = (rawDanger * (defensiveRestriction > 0 ? 3 : 2)) / 8;
                }
            }
        }
    }
    KingDangerResult result{std::min(escalatedDanger, 450), attackerParticipation,
            defenderParticipation, escapeDanger, filePressure,
            diagonalPressure, shelterDanger, attackingMaterialScale};
    return result;
}
}

EvaluationChessCache EvaluationLogic::EvalCache;
PawnCache EvaluationLogic::PawnEvalCache;

int EvaluationLogic::CentralKingReadinessPenalty(Board& board, bool whiteKing, int phase,
                                                const EvaluationContext* ctx)
{
    const int king = board.pieces[whiteKing ? 6 : 14].front();
    const int home = whiteKing ? 0 : 56;
    const int forward = whiteKing ? 8 : -8;
    // Restrict this opening interaction to a king still in the central files.
    if (phase <= 12 || king % 8 < 3 || king % 8 > 4 ||
        (whiteKing ? king / 8 > 1 : king / 8 < 6))
        return 0;

    const bool enemyWhite = !whiteKing;
    const int own = whiteKing ? 0 : 8;
    const int enemy = enemyWhite ? 0 : 8;
    const long long occupied = board.whitePieces | board.blackPieces;
    bool activeQueen = false;
    for (int queen : board.pieces[enemy + 5])
        activeQueen |= ChebyshevDistance(queen, king) <= 4 &&
                       (enemyWhite ? queen / 8 >= 3 : queen / 8 <= 4);
    bool centralSpace = false;
    for (int pawn : board.pieces[enemy + 1])
        centralSpace |= pawn % 8 >= 3 && pawn % 8 <= 4 &&
                        (enemyWhite ? pawn / 8 >= 4 : pawn / 8 <= 3);
    if (!activeQueen || !centralSpace)
        return 0;

    int unready = 0;
    int blockedBishops = 0;
    for (int knight : board.pieces[own + 2])
        unready += knight == home + 1 || knight == home + 6;
    for (int bishop : board.pieces[own + 3])
    {
        if (bishop != home + 2 && bishop != home + 5)
            continue;
        ++unready;
        const int left = board.mainBoard[bishop + forward - 1];
        const int right = board.mainBoard[bishop + forward + 1];
        const auto friendly = [&](int piece) { return piece != 0 && (piece < 8) == whiteKing; };
        if (friendly(left) && friendly(right) &&
            (left == own + 2 || left == own + 3 || right == own + 2 || right == own + 3))
        {
            // A minor blocking a home bishop's last exit is not useful development.
            ++unready;
            ++blockedBishops;
        }
    }
    if (unready < 2)
        return 0;

    // Include the nearby shelter squares, not just squares giving immediate check.
    const uint64_t area = CentralKingAttackPressure::KingLayers.inner[king] |
                          CentralKingAttackPressure::KingLayers.outer[king];
    const bool reuseAttacks = ctx != nullptr && ctx->attacksReady;
    const auto pieceAttacks = [&](int type, int from, long long occupancy) -> uint64_t {
        if (type == 2) return AttackPlaces::KnightAttackPlaces[from];
        return EvaluationSliderAttacks(from, type, occupancy);
    };
    bool queenPressure = false;
    for (int queen : board.pieces[enemy + 5])
        queenPressure |= ChebyshevDistance(queen, king) <= 4 &&
            ((reuseAttacks ? ctx->attacks[queen] : pieceAttacks(5, queen, occupied)) & area) != 0;
    if (!queenPressure)
        return 0;

    bool reinforcement = false;
    bool developedCover = false;
    for (int type : {2, 3})
    {
        for (int from : board.pieces[enemy + type])
        {
            const uint64_t attacks = reuseAttacks ? ctx->attacks[from] : pieceAttacks(type, from, occupied);
            if (attacks & area)
                continue;
            uint64_t destinations = attacks & ~occupied;
            while (destinations && !(reinforcement && developedCover))
            {
                const int to = __builtin_ctzll(destinations);
                destinations &= destinations - 1;
                const long long after = (occupied & ~Option::PowerTwo[from]) | Option::PowerTwo[to];
                if ((pieceAttacks(type, to, after) & area) == 0 ||
                    HasSideAttack(board, whiteKing, board.pieces[enemy + 6].front(), after))
                    continue;
                // Useful developed defenders contest the actual reinforcement routes.
                for (int defenderType : {2, 3, 5})
                    for (int defender : board.pieces[own + defenderType])
                        if ((whiteKing ? defender / 8 >= 2 : defender / 8 <= 5) &&
                            PieceAttacksSquareFast(after, defenderType, whiteKing, defender, to))
                            developedCover = true;
                if (!HasSideAttack(board, whiteKing, to, after))
                    reinforcement = true;
            }
        }
    }

    // Direct centipawn terms: coordination <=64, latent reinforcement <=24,
    // future shelter <=48. Developed coverage reduces the unresolved liability.
    const int coordination = 16 * std::min(4, unready + blockedBishops);
    const int latent = reinforcement ? 24 : 0;
    int shelter = 0;
    if (reinforcement && (whiteKing ? board.whiteSmallCastle : board.blackSmallCastle) &&
        board.mainBoard[home + 7] == own + 4)
    {
        for (int file = 5; file <= 7; ++file)
            if (board.mainBoard[home + forward + file] != own + 1)
                shelter += file == 6 ? 32 : 16;
        shelter = std::min(48, shelter);
    }
    const int penalty = (coordination + latent + shelter) / (developedCover ? 4 : 1);
    return penalty * std::min(8, phase - 12) / 8;
}

std::size_t EvaluationLogic::EvalCacheSize()
{
    return EvalCache.size();
}

std::size_t EvaluationLogic::PawnEvalCacheSize()
{
    return PawnEvalCache.size();
}

std::size_t EvaluationLogic::EvalCacheCapacityBytes()
{
    return EvalCache.capacityBytes();
}

std::size_t EvaluationLogic::EvalCacheEntryCapacity()
{
    return EvalCache.entryCapacity();
}

std::size_t EvaluationLogic::EvalCacheClusterCount()
{
    return EvalCache.clusterCount();
}

EvaluationCacheStatistics EvaluationLogic::EvalCacheStats()
{
    return EvalCache.statistics();
}

void EvaluationLogic::ResetEvalCacheStats()
{
    EvalCache.resetStatistics();
}

bool EvaluationLogic::ResizeEvalCache(std::size_t capacityBytes)
{
    return EvalCache.resize(capacityBytes);
}

#if HOWL_CORRECTNESS_TESTING
void EvaluationLogic::SetEvalCacheAllocationFailureThresholdForTesting(
    std::size_t capacityBytes)
{
    EvaluationChessCache::SetAllocationFailureThresholdForTesting(capacityBytes);
}
#endif

#if HOWL_CORRECTNESS_TESTING
void EvaluationLogic::ClearEvalCacheForTesting()
{
    EvalCache.clear();
}

int EvaluationLogic::RookConnectionValueForTesting(Board& board)
{
    return RookConnectionValue(board.pieces, board.whitePieces | board.blackPieces);
}

int EvaluationLogic::RookBehindPassedPawnValueForTesting(Board& board, int phase)
{
    return RookBehindPassedPawnValue(board, phase);
}

int EvaluationLogic::UndefendedKingZoneDangerForTesting(Board& board, bool whiteKing)
{
    const int kingSquare = board.pieces[whiteKing ? 6 : 14].front();
    return UndefendedKingZoneDanger(board, whiteKing, kingSquare);
}

int EvaluationLogic::TaperEvaluationValueForTesting(int middleGameValue,
                                                     int endGameValue, int phase)
{
    return TaperEvaluationValue(middleGameValue, endGameValue, phase);
}

int EvaluationLogic::TaperGroup1ValueForTesting(int middleGameValue,
                                               int endGameValue, int phase)
{
    return TaperGroup1Value(middleGameValue, endGameValue, phase);
}

int EvaluationLogic::TaperGroup2ValueForTesting(int middleGameValue,
                                               int endGameValue, int phase)
{
    return TaperGroup2Value(middleGameValue, endGameValue, phase);
}

int EvaluationLogic::TaperGroup3ValueForTesting(int middleGameValue,
                                               int endGameValue, int phase)
{
    return TaperGroup3Value(middleGameValue, endGameValue, phase);
}

int EvaluationLogic::KnightOutpostValueForTesting(Board& board, int phase)
{
    int value = 0;
    for (int square : board.pieces[2])
        value += KnightOutpostValue(board, square, true, phase);
    for (int square : board.pieces[10])
        value -= KnightOutpostValue(board, square, false, phase);
    return value;
}

int EvaluationLogic::PassedPawnMinorAccessibilityValueForTesting(Board& board)
{
    return EvaluatePassedPawnMinorAccessibility(board);
}

int EvaluationLogic::IsolatedPawnValueForTesting(Board& board, int phase)
{
    int value = 0;
    const int penalty = IsolatedPawnPenalty(phase);
    for (int square : board.pieces[1])
        value += IsIsolatedPawn(board.whitePawns, square) ? penalty : 0;
    for (int square : board.pieces[9])
        value -= IsIsolatedPawn(board.blackPawns, square) ? penalty : 0;
    return value;
}
#endif

int EvaluationLogic::CalculatePhase(const Board &thisBoard)
{
    const auto &pieces = thisBoard.pieces;
    int phase = pieces[2].size() * 1 + pieces[3].size() * 1 + pieces[4].size() * 2 + pieces[5].size() * 4
              + pieces[10].size() * 1 + pieces[11].size() * 1 + pieces[12].size() * 2 + pieces[13].size() * 4;
    return std::clamp(phase, 0, 24);
}

namespace
{
int LoneKingMateGuidance(Board &board)
{
    const auto materialCount = [&board](int offset)
    {
        int count = 0;
        for (int piece = 1; piece <= 5; ++piece)
            count += static_cast<int>(board.pieces[offset + piece].size());
        return count;
    };

    const bool blackIsLone = materialCount(8) == 0 &&
        (board.pieces[4].size() > 0 || board.pieces[5].size() > 0);
    const bool whiteIsLone = materialCount(0) == 0 &&
        (board.pieces[12].size() > 0 || board.pieces[13].size() > 0);
    if (blackIsLone == whiteIsLone)
        return 0;

    const bool whiteWinning = blackIsLone;
    const int loneKingSquare = board.pieces[whiteWinning ? 14 : 6].front();
    const int winningKingSquare = board.pieces[whiteWinning ? 6 : 14].front();
    const int file = loneKingSquare % 8;
    const int rank = loneKingSquare / 8;
    const int edgeDistance = std::min({file, 7 - file, rank, 7 - rank});
    const int edgeSteps = 3 - edgeDistance;
    const int nearestCornerDistance = std::min(file, 7 - file) + std::min(rank, 7 - rank);
    const int cornerSteps = 6 - nearestCornerDistance;

    // 1. Determine the area in which winning rook(s) or queen(s) confine the lone king.
    const int winningOffset = whiteWinning ? 0 : 8;
    int minCutFile = -1, maxCutFile = 8;
    int minCutRank = -1, maxCutRank = 8;
    auto updateCuts = [&](int sq)
    {
        const int f = sq % 8;
        const int r = sq / 8;
        if (f < file && f > minCutFile) minCutFile = f;
        if (f > file && f < maxCutFile) maxCutFile = f;
        if (r < rank && r > minCutRank) minCutRank = r;
        if (r > rank && r < maxCutRank) maxCutRank = r;
    };
    for (int sq : board.pieces[winningOffset + 4]) updateCuts(sq);
    for (int sq : board.pieces[winningOffset + 5]) updateCuts(sq);

    const int westSpace = file - std::max(0, minCutFile + 1);
    const int eastSpace = std::min(7, maxCutFile - 1) - file;
    const int southSpace = rank - std::max(0, minCutRank + 1);
    const int northSpace = std::min(7, maxCutRank - 1) - rank;

    const auto squareAttacked = [&board, loneKingSquare](int target, bool whiteAttacks)
    {
        const long long targetBit = Option::PowerTwo[target];
        const long long occupancy =
            ((board.whitePieces | board.blackPieces) & ~Option::PowerTwo[loneKingSquare]) |
            targetBit;
        const int offset = whiteAttacks ? 0 : 8;
        for (int square : board.pieces[offset + 1])
            if (((whiteAttacks ? AttackPlaces::WhitePawnAttackPlaces[square]
                                : AttackPlaces::BlackPawnAttackPlaces[square]) & targetBit) != 0)
                return true;
        for (int square : board.pieces[offset + 2])
            if ((AttackPlaces::KnightAttackPlaces[square] & targetBit) != 0)
                return true;
        for (int square : board.pieces[offset + 3])
            if ((AttackPlaces::BishopAttack[square][target] & occupancy) == targetBit)
                return true;
        for (int square : board.pieces[offset + 4])
            if ((AttackPlaces::RookAttack[square][target] & occupancy) == targetBit)
                return true;
        for (int square : board.pieces[offset + 5])
            if ((AttackPlaces::QueenAttack[square][target] & occupancy) == targetBit)
                return true;
        for (int square : board.pieces[offset + 6])
            if ((AttackPlaces::KingAttackPlaces[square] & targetBit) != 0)
                return true;
        return false;
    };

    int safeNeighbours = 0;
    int confinementSupport = 0;
    for (int rankDelta = -1; rankDelta <= 1; ++rankDelta)
    {
        for (int fileDelta = -1; fileDelta <= 1; ++fileDelta)
        {
            if (rankDelta == 0 && fileDelta == 0)
                continue;
            const int targetRank = rank + rankDelta;
            const int targetFile = file + fileDelta;
            if (targetRank < 0 || targetRank > 7 || targetFile < 0 || targetFile > 7)
                continue;
            const int target = targetRank * 8 + targetFile;
            if (!squareAttacked(target, whiteWinning))
                ++safeNeighbours;

            // 2. Identify boundary squares through which the lone king could expand back into a larger area.
            const bool isBoundaryOrEscape =
                (maxCutFile <= 7 && targetFile >= maxCutFile) ||
                (minCutFile >= 0 && targetFile <= minCutFile) ||
                (maxCutRank <= 7 && targetRank >= maxCutRank) ||
                (minCutRank >= 0 && targetRank <= minCutRank) ||
                (eastSpace > westSpace && targetFile > file) ||
                (westSpace > eastSpace && targetFile < file) ||
                (northSpace > southSpace && targetRank > rank) ||
                (southSpace > northSpace && targetRank < rank);

            // 3. Reward the winning king for controlling those boundary / escape squares.
            if (isBoundaryOrEscape &&
                (AttackPlaces::KingAttackPlaces[winningKingSquare] & Option::PowerTwo[target]) != 0)
            {
                ++confinementSupport;
            }
        }
    }

    const int guidance = 200 + 12 * edgeSteps + 6 * cornerSteps + 8 * confinementSupport +
                         10 * (8 - safeNeighbours);
    return whiteWinning ? guidance : -guidance;
}

int EvaluateInternal(Board &thisBoard, EvaluationBreakdown *breakdown)
{
    long long piecesBinary = thisBoard.whitePieces | thisBoard.blackPieces;
    MyList (&pieces)[15] = thisBoard.pieces;

    int whitePieceEvaluation = pieces[1].size() * Option::PawnValue + pieces[2].size() * Option::KnightValue + pieces[3].size() * (Option::BishopValue + (8 - (pieces[1].size() + pieces[9].size())) * 2) + pieces[4].size() * Option::RookValue + pieces[5].size() * Option::QueenValue;

    int blackPieceEvaluation = pieces[9].size() * Option::PawnValue + pieces[10].size() * Option::KnightValue + pieces[11].size() * (Option::BishopValue + (8 - (pieces[1].size() + pieces[9].size())) * 2) + pieces[12].size() * Option::RookValue + pieces[13].size() * Option::QueenValue;
    double pieceBalance = 1;
    if (whitePieceEvaluation > blackPieceEvaluation)
    {
        pieceBalance = static_cast<double>(whitePieceEvaluation + 1500) / (blackPieceEvaluation + 1500);
    }
    else if (whitePieceEvaluation < blackPieceEvaluation)
    {
        pieceBalance = static_cast<double>(blackPieceEvaluation + 1500) / (whitePieceEvaluation + 1500);
    }

    if (whitePieceEvaluation > blackPieceEvaluation)
    {
        if (pieces[1].size() == 0)
        {
            pieceBalance *= .7;
        }
        if (pieces[1].size() == 1)
        {
            pieceBalance *= .9;
        }
    }
    else if (whitePieceEvaluation < blackPieceEvaluation)
    {
        if (pieces[9].size() == 0)
        {
            pieceBalance *= .7;
        }
        if (pieces[9].size() == 1)
        {
            pieceBalance *= .9;
        }
    }

    int pieceEvaluation = (int)((whitePieceEvaluation - blackPieceEvaluation) * pieceBalance);
    // Bishop pair
    int whiteBishopPair = 0;
    int blackBishopPair = 0;
    const int totalPawns = pieces[1].size() + pieces[9].size();
    const int bpBonus = std::max(0, 48 - totalPawns * 3);
    if (pieces[3].size() == 2 && ((pieces[3][0] / 8 + pieces[3][0] % 8) % 2) != ((pieces[3][1] / 8 + pieces[3][1] % 8) % 2))    
    {
        int dev = 0;
        for (int sq : pieces[3]) if (sq != 2 && sq != 5) dev++;
        whiteBishopPair = (dev >= 2) ? bpBonus : (dev == 1 ? (bpBonus / 2) : 0);
    }
    if (pieces[11].size() == 2 && ((pieces[11][0] / 8 + pieces[11][0] % 8) % 2) != ((pieces[11][1] / 8 + pieces[11][1] % 8) % 2))
    {
        int dev = 0;
        for (int sq : pieces[11]) if (sq != 58 && sq != 61) dev++;
        blackBishopPair = (dev >= 2) ? bpBonus : (dev == 1 ? (bpBonus / 2) : 0);
    }
    int bishopPairVaue = whiteBishopPair - blackBishopPair;

    int phase = EvaluationLogic::CalculatePhase(thisBoard);
    EvaluationContext ctx(thisBoard, phase);
    ctx.InitializeAttacks();

    // Movement
    MovementResult moveRes = EvaluationLogic::PieceMoveCountFast(thisBoard, phase, ctx);
    int movement = moveRes.movement;
    int attackNet = moveRes.attackNet;
    int center = moveRes.center;
    int rookFileNet = moveRes.rookFileNet;
    int whiteRookFile = moveRes.whiteRookFile;
    int blackRookFile = moveRes.blackRookFile;

    KingDangerResult whiteKingDanger = EvaluateKingDanger(thisBoard, true, &ctx, breakdown != nullptr);
    KingDangerResult blackKingDanger = EvaluateKingDanger(thisBoard, false, &ctx, breakdown != nullptr);
    int kingDangerNet = blackKingDanger.danger - whiteKingDanger.danger;

    int whiteKingSq = ctx.whiteKingSq;
    int blackKingSq = ctx.blackKingSq;
    int whiteKingPlacement = (Option::WhiteKingPlaceSafetyMiddleGame[whiteKingSq] * phase
                              + Option::KingInValueWhiteEndGame[whiteKingSq] * (24 - phase)) / 24;
    int blackKingPlacement = (Option::BlackKingPlaceSafetyMiddleGame[blackKingSq] * phase
                              + Option::KingInValueBlackEndGame[blackKingSq] * (24 - phase)) / 24;
    int kingPlacementNet = whiteKingPlacement - blackKingPlacement;
    const CentralKingAttackPressure::Result whiteCentralPressure =
        CentralKingAttackPressure::Evaluate(thisBoard, true, ctx.whitePawnFiles, ctx.blackPawnFiles,
                                           ctx.attacks, ctx.legacyAttacks);
    const CentralKingAttackPressure::Result blackCentralPressure =
        CentralKingAttackPressure::Evaluate(thisBoard, false, ctx.whitePawnFiles, ctx.blackPawnFiles,
                                           ctx.attacks, ctx.legacyAttacks);
    const int centralPressureNet =
        whiteCentralPressure.contribution - blackCentralPressure.contribution;

    const bool whiteCastled = (!thisBoard.whiteSmallCastle && !thisBoard.whiteBigCastle && (whiteKingSq == 6 || whiteKingSq == 2));
    const bool blackCastled = (!thisBoard.blackSmallCastle && !thisBoard.blackBigCastle && (blackKingSq == 62 || blackKingSq == 58));
    const int whiteKingR = whiteKingSq / 8, whiteKingC = whiteKingSq % 8;
    const int blackKingR = blackKingSq / 8, blackKingC = blackKingSq % 8;
    const bool whiteCentralKing = (!whiteCastled && whiteKingR <= 1 && whiteKingC >= 2 && whiteKingC <= 5);
    const bool blackCentralKing = (!blackCastled && blackKingR >= 6 && blackKingC >= 2 && blackKingC <= 5);

    int whitePawnShield = 0;
    int blackPawnShield = 0;
    if (phase >= 12)
    {
        if (whiteCastled)
        {
            if (whiteKingDanger.pawnShelter <= 8)
                whitePawnShield = (15 * phase) / 24;
            else
                whitePawnShield = - ((whiteKingDanger.pawnShelter - 8) * phase) / 24;
        }
        else if (whiteCentralKing && whiteKingDanger.pawnShelter > 8)
        {
            whitePawnShield = - ((whiteKingDanger.pawnShelter - 8) * phase) / 48;
        }

        if (blackCastled)
        {
            if (blackKingDanger.pawnShelter <= 8)
                blackPawnShield = (15 * phase) / 24;
            else
                blackPawnShield = - ((blackKingDanger.pawnShelter - 8) * phase) / 24;
        }
        else if (blackCentralKing && blackKingDanger.pawnShelter > 8)
        {
            blackPawnShield = - ((blackKingDanger.pawnShelter - 8) * phase) / 48;
        }
    }
    int pawnShieldNet = whitePawnShield - blackPawnShield;

    int castledSecurityNet = 0;
    if (phase >= 14)
    {
        if (whiteCastled && blackCentralKing)
            castledSecurityNet += (45 * phase) / 24;
        else if (blackCastled && whiteCentralKing)
            castledSecurityNet -= (45 * phase) / 24;
    }

    int kingSafety = kingDangerNet + kingPlacementNet + centralPressureNet + pawnShieldNet + castledSecurityNet;
    kingSafety += EvaluationLogic::CentralKingReadinessPenalty(thisBoard, false, phase, &ctx) -
                  EvaluationLogic::CentralKingReadinessPenalty(thisBoard, true, phase, &ctx);

    // Pawn Structure
    int pawnBase = EvaluationLogic::GetPawnStructureValue(thisBoard, phase, &ctx);
    int passedPawnKingRace = EvaluatePassedPawnKingRace(thisBoard, whiteKingSq, blackKingSq, ctx);
    int passedPawnMinorAccessibility = EvaluatePassedPawnMinorAccessibility(thisBoard, &ctx);
    int passedPawnCorridorSafety = EvaluatePassedPawnCorridorSafety(thisBoard, &ctx);
    int rookBehindPassedPawn = RookBehindPassedPawnValue(thisBoard, phase, &ctx);
    int pawnStructure = pawnBase + passedPawnKingRace + passedPawnMinorAccessibility +
                        passedPawnCorridorSafety + rookBehindPassedPawn;

    // Rook Connection
    int rookValue = RookConnectionValue(pieces, piecesBinary);
    // Temp
    const int taperedTempo = TaperGroup1Value(24, 11, phase);
    int temp = (!thisBoard.sideToMove) ? taperedTempo : -taperedTempo;

    double oppositeColorBishop = 1.0;
    if (pieces[3].size() == 1 && pieces[11].size() == 1 && ((pieces[3].front() / 8 + pieces[3].front() % 8) % 2) != ((pieces[11].front() / 8 + pieces[11].front() % 8) % 2))
    {
        oppositeColorBishop = (0.9 * phase + 0.75 * (24 - phase)) / 24;
    }

    const int loneKingMateGuidance = LoneKingMateGuidance(thisBoard);
    int unscaled = pieceEvaluation + bishopPairVaue + movement + pawnStructure + kingSafety +
                   rookValue + center + temp + loneKingMateGuidance;

    double endgameScaleFactor = oppositeColorBishop;
    if (unscaled > 0)
    {
        if (pieces[1].size() == 0 && pieces[4].size() == 0 && pieces[5].size() == 0 &&
            (pieces[2].size() + pieces[3].size() == 1) &&
            pieces[9].size() >= 1)
        {
            endgameScaleFactor *= 0.25;
        }
    }
    else if (unscaled < 0)
    {
        if (pieces[9].size() == 0 && pieces[12].size() == 0 && pieces[13].size() == 0 &&
            (pieces[10].size() + pieces[11].size() == 1) &&
            pieces[1].size() >= 1)
        {
            endgameScaleFactor *= 0.25;
        }
    }

    int evaluation = (int)(unscaled * endgameScaleFactor); 
    bool drawAdjustment = false;
    bool noPawns = (pieces[1].size() == 0 && pieces[9].size() == 0);
    bool noMajorPieces = (pieces[4].size() == 0 && pieces[12].size() == 0 &&
                          pieces[5].size() == 0 && pieces[13].size() == 0);

    if (noPawns && noMajorPieces)
    {
        int whiteKnights = pieces[2].size();
        int whiteBishops = pieces[3].size();
        int blackKnights = pieces[10].size();
        int blackBishops = pieces[11].size();

        int whiteMinors = whiteKnights + whiteBishops;
        int blackMinors = blackKnights + blackBishops;

        if (whiteMinors == 0 && blackMinors == 0)
        {
            evaluation = 0;
            drawAdjustment = true;
        }
        else if (blackMinors == 0 && ((whiteBishops == 0 && whiteKnights <= 2) || (whiteKnights == 0 && whiteBishops < 2)))
        {
            evaluation = 0;
            drawAdjustment = true;
        }
        else if (whiteMinors == 0 && ((blackBishops == 0 && blackKnights <= 2) || (blackKnights == 0 && blackBishops < 2)))
        {
            evaluation = 0;
            drawAdjustment = true;
        }
    }

    if (breakdown != nullptr)
    {
        breakdown->phase = phase;
        breakdown->whiteMaterial = whitePieceEvaluation;
        breakdown->blackMaterial = blackPieceEvaluation;
        breakdown->materialNet = whitePieceEvaluation - blackPieceEvaluation;
        breakdown->pieceBalance = pieceBalance;
        breakdown->pieceEvaluation = pieceEvaluation;
        breakdown->loneKingMateGuidance = loneKingMateGuidance;

        breakdown->whiteBishopPair = whiteBishopPair;
        breakdown->blackBishopPair = blackBishopPair;
        breakdown->bishopPairNet = bishopPairVaue;

        breakdown->mobilityNet = movement;
        breakdown->pieceAttacksNet = attackNet;
        breakdown->whiteRookFileBonus = whiteRookFile;
        breakdown->blackRookFileBonus = blackRookFile;
        breakdown->rookFileBonusNet = rookFileNet;
        breakdown->centerNet = center;

        breakdown->kingAttackNet = kingDangerNet;
        breakdown->whiteKingPlacement = whiteKingPlacement;
        breakdown->blackKingPlacement = blackKingPlacement;
        breakdown->kingPlacementNet = kingPlacementNet;
        breakdown->whitePawnShield = whitePawnShield;
        breakdown->blackPawnShield = blackPawnShield;
        breakdown->pawnShieldNet = pawnShieldNet;
        breakdown->whiteCentralKingExposure = 0;
        breakdown->blackCentralKingExposure = 0;
        breakdown->centralKingExposureNet = 0;
        breakdown->whiteCentralKingAttackPressure = whiteCentralPressure.contribution;
        breakdown->blackCentralKingAttackPressure = blackCentralPressure.contribution;
        breakdown->centralKingAttackPressureNet = centralPressureNet;
        breakdown->centralGeneralOpenness = whiteCentralPressure.generalCentreOpenness;
        breakdown->centralEffectiveOpenness = whiteCentralPressure.effectiveOpenness;
        breakdown->centralDFileExposure = whiteCentralPressure.dFileExposure;
        breakdown->centralEFileExposure = whiteCentralPressure.eFileExposure;
        breakdown->centralCentreLocked = whiteCentralPressure.centreLocked;
        breakdown->centralCentreOpen = whiteCentralPressure.centreOpen;
        breakdown->whiteCentralKingActive = whiteCentralPressure.centralKingActive;
        breakdown->blackCentralKingActive = blackCentralPressure.centralKingActive;
        breakdown->whiteHeavyLinePressure = whiteCentralPressure.heavyLinePressure;
        breakdown->blackHeavyLinePressure = blackCentralPressure.heavyLinePressure;
        breakdown->whiteBishopDiagonalPressure = whiteCentralPressure.bishopDiagonalPressure;
        breakdown->blackBishopDiagonalPressure = blackCentralPressure.bishopDiagonalPressure;
        breakdown->whiteDirectHeavyLines = whiteCentralPressure.directHeavyLines;
        breakdown->blackDirectHeavyLines = blackCentralPressure.directHeavyLines;
        breakdown->whiteOneBlockerHeavyLines = whiteCentralPressure.oneBlockerHeavyLines;
        breakdown->blackOneBlockerHeavyLines = blackCentralPressure.oneBlockerHeavyLines;
        breakdown->whiteMultiBlockerHeavyLines = whiteCentralPressure.multiBlockerHeavyLines;
        breakdown->blackMultiBlockerHeavyLines = blackCentralPressure.multiBlockerHeavyLines;
        breakdown->whiteDirectBishopLines = whiteCentralPressure.directBishopLines;
        breakdown->blackDirectBishopLines = blackCentralPressure.directBishopLines;
        breakdown->whiteOneBlockerBishopLines = whiteCentralPressure.oneBlockerBishopLines;
        breakdown->blackOneBlockerBishopLines = blackCentralPressure.oneBlockerBishopLines;
        breakdown->whiteMultiBlockerBishopLines = whiteCentralPressure.multiBlockerBishopLines;
        breakdown->blackMultiBlockerBishopLines = blackCentralPressure.multiBlockerBishopLines;
        breakdown->whiteInnerAttackers = whiteCentralPressure.innerAttackers;
        breakdown->blackInnerAttackers = blackCentralPressure.innerAttackers;
        breakdown->whiteOuterAttackers = whiteCentralPressure.outerAttackers;
        breakdown->blackOuterAttackers = blackCentralPressure.outerAttackers;
        breakdown->whiteInnerAttackContribution = whiteCentralPressure.innerAttackContribution;
        breakdown->blackInnerAttackContribution = blackCentralPressure.innerAttackContribution;
        breakdown->whiteOuterAttackContribution = whiteCentralPressure.outerAttackContribution;
        breakdown->blackOuterAttackContribution = blackCentralPressure.outerAttackContribution;
        breakdown->whiteNonlinearEscalation = whiteCentralPressure.nonlinearEscalation;
        breakdown->blackNonlinearEscalation = blackCentralPressure.nonlinearEscalation;
        breakdown->whiteCastlingMitigation = whiteCentralPressure.castlingMitigation;
        breakdown->blackCastlingMitigation = blackCentralPressure.castlingMitigation;
        breakdown->whiteImmediateCastling = whiteCentralPressure.immediateCastling;
        breakdown->blackImmediateCastling = blackCentralPressure.immediateCastling;
        breakdown->kingSafetyTotal = kingSafety;
        breakdown->whiteKingDanger = whiteKingDanger.danger;
        breakdown->blackKingDanger = blackKingDanger.danger;
        breakdown->whiteAttackerWeight = whiteKingDanger.attackerWeight;
        breakdown->blackAttackerWeight = blackKingDanger.attackerWeight;
        breakdown->whiteDefenderWeight = whiteKingDanger.defenderWeight;
        breakdown->blackDefenderWeight = blackKingDanger.defenderWeight;
        breakdown->whiteEscapeSafety = whiteKingDanger.escapeSafety;
        breakdown->blackEscapeSafety = blackKingDanger.escapeSafety;
        breakdown->whiteFilePressure = whiteKingDanger.filePressure;
        breakdown->blackFilePressure = blackKingDanger.filePressure;
        breakdown->whiteDiagonalPressure = whiteKingDanger.diagonalPressure;
        breakdown->blackDiagonalPressure = blackKingDanger.diagonalPressure;
        breakdown->whitePawnShelter = whiteKingDanger.pawnShelter;
        breakdown->blackPawnShelter = blackKingDanger.pawnShelter;
        breakdown->whitePhaseScale = whiteKingDanger.phaseScale;
        breakdown->blackPhaseScale = blackKingDanger.phaseScale;

        breakdown->pawnStructureNet = pawnStructure;
        breakdown->pawnBaseNet = pawnBase;
        breakdown->passedPawnKingRaceNet = passedPawnKingRace;
        breakdown->passedPawnMinorAccessibilityNet = passedPawnMinorAccessibility;
        breakdown->passedPawnCorridorSafetyNet = passedPawnCorridorSafety;
        breakdown->rookBehindPassedPawnNet = rookBehindPassedPawn;
        breakdown->rookConnectionNet = rookValue;
        breakdown->tempoNet = temp;

        breakdown->oppositeColorBishopScale = endgameScaleFactor;
        breakdown->unscaledTotal = unscaled;
        breakdown->scaledTotal = (int)(unscaled * endgameScaleFactor);
        breakdown->drawAdjustmentApplied = drawAdjustment;
        breakdown->whitePerspectiveTotal = evaluation;
        breakdown->sideToMoveTotal = thisBoard.sideToMove ? -evaluation : evaluation;
    }

    return evaluation;
}
}

int EvaluationLogic::Evaluate(Board &thisBoard)
{
    const std::uint64_t evaluationKey =
        static_cast<std::uint64_t>(thisBoard.ZobristHashCode);
    std::optional<std::int32_t> cacheEvalResult = EvalCache.getFromCache(evaluationKey);

    if (cacheEvalResult.has_value())
    {
        return cacheEvalResult.value();
    }

    int evaluation = EvaluateInternal(thisBoard, nullptr);
    int finalScore = (!thisBoard.sideToMove) ? evaluation : -evaluation;
    EvalCache.addToCache(evaluationKey, static_cast<std::int32_t>(finalScore));
    return finalScore;
}

EvaluationBreakdown EvaluationLogic::EvaluateDetailed(Board &thisBoard)
{
    EvaluationBreakdown breakdown{};
    EvaluateInternal(thisBoard, &breakdown);
    return breakdown;
}

int EvaluationLogic::GetPawnStructureValue(Board &thisBoard, int phase, const EvaluationContext* ctx)
{
    const long long whitePawns = thisBoard.whitePawns;
    const long long blackPawns = thisBoard.blackPawns;
    // File counts are shared with the context; connected doubled pawns can
    // be detected for a whole file without materializing per-file square lists.
    const auto doubledPenalty = [&](uint64_t pawns, int side) {
        const uint64_t diagonalNeighbours =
            ((pawns & ~0x8080808080808080ULL) << 9) |
            ((pawns & ~0x0101010101010101ULL) << 7) |
            ((pawns & ~0x0101010101010101ULL) >> 9) |
            ((pawns & ~0x8080808080808080ULL) >> 7);
        const uint64_t connected = pawns & diagonalNeighbours;
        int total = 0;
        for (int file = 0; file < 8; ++file)
        {
            const uint64_t fileMask = 0x0101010101010101ULL << file;
            const int count = ctx ? ctx->pawnFileCounts[side][file]
                                  : __builtin_popcountll(pawns & fileMask);
            if (count > 1)
            {
                int penalty = Option::DoubledPawnValue * (count - 1);
                if (connected & fileMask) penalty /= 2;
                total += penalty;
            }
        }
        return total;
    };
    const int doubledPawnValueWhite = doubledPenalty(whitePawns, 0);
    int singlePastWhite = 0;
    if (ctx != nullptr)
    {
        for (int i = 0; i < ctx->whitePasserCount; ++i)
        {
            int pawnPlace = ctx->whitePassers[i];
            int mgVal = Option::WhitePassedPawnValueMiddleGam[pawnPlace];
            int egVal = Option::WhitePassedPawnValueEndGame[pawnPlace];
            singlePastWhite += TaperGroup3Value(mgVal, egVal, phase);
        }
    }
    else
    {
        for (int pawnPlace : thisBoard.pieces[1])
        {
            if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & blackPawns) == 0)
            {
                int mgVal = Option::WhitePassedPawnValueMiddleGam[pawnPlace];
                int egVal = Option::WhitePassedPawnValueEndGame[pawnPlace];
                singlePastWhite += TaperGroup3Value(mgVal, egVal, phase);
            }
        }
    }
    int isolatedPawnValueWhite = 0;
    int goForwardPawnWhite = 0;
    int pawnChainWhite = 0;
    const int isolatedPenalty = IsolatedPawnPenalty(phase);
    const int whiteKingSq = ctx ? ctx->whiteKingSq : thisBoard.pieces[6].front();
    const int blackKingSq = ctx ? ctx->blackKingSq : thisBoard.pieces[14].front();
    for (int pawnPlace : thisBoard.pieces[1])
    {
        if (IsIsolatedPawn(whitePawns, pawnPlace))
            isolatedPawnValueWhite += isolatedPenalty;

        const int endGameValue = (pawnPlace / 8) * 2;
        goForwardPawnWhite += TaperGroup3Value(0, endGameValue, phase);

        if ((AttackPlaces::BlackPawnAttackPlaces[pawnPlace] & whitePawns) != 0)
        {
            pawnChainWhite += 6;
            int r = pawnPlace / 8, f = pawnPlace % 8;
            if (f >= 2 && f <= 5)
            {
                if (r >= 3) pawnChainWhite += 6;
                if (r >= 4) pawnChainWhite += 6;
                if ((r == 4 || r == 5) && ChebyshevDistance(pawnPlace, blackKingSq) <= 3)
                {
                    pawnChainWhite += TaperGroup1Value(14, 2, phase);
                }
            }
        }
    }
    int whitePawnSum = doubledPawnValueWhite + singlePastWhite + isolatedPawnValueWhite + goForwardPawnWhite + pawnChainWhite;

    const int doubledPawnValueBlack = doubledPenalty(blackPawns, 1);
    int singlePastBlack = 0;
    if (ctx != nullptr)
    {
        for (int i = 0; i < ctx->blackPasserCount; ++i)
        {
            int pawnPlace = ctx->blackPassers[i];
            int mgVal = Option::BlackPassedPawnValueMiddleGam[pawnPlace];
            int egVal = Option::BlackPassedPawnValueEndGam[pawnPlace];
            singlePastBlack += TaperGroup3Value(mgVal, egVal, phase);
        }
    }
    else
    {
        for (int pawnPlace : thisBoard.pieces[9])
        {
            if ((PassedPawnSetup::BlackPassedMask[pawnPlace] & whitePawns) == 0)
            {
                int mgVal = Option::BlackPassedPawnValueMiddleGam[pawnPlace];
                int egVal = Option::BlackPassedPawnValueEndGam[pawnPlace];
                singlePastBlack += TaperGroup3Value(mgVal, egVal, phase);
            }
        }
    }
    int isolatedPawnValueBlack = 0;
    int goForwardPawnBlack = 0;
    int pawnChainBlack = 0;
    const int isolatedPenaltyBlack = IsolatedPawnPenalty(phase);
    for (int pawnPlace : thisBoard.pieces[9])
    {
        if (IsIsolatedPawn(blackPawns, pawnPlace))
            isolatedPawnValueBlack += isolatedPenaltyBlack;

        const int endGameValue = (7 - (pawnPlace / 8)) * 2;
        goForwardPawnBlack += TaperGroup3Value(0, endGameValue, phase);

        if ((AttackPlaces::WhitePawnAttackPlaces[pawnPlace] & blackPawns) != 0)
        {
            pawnChainBlack += 6;
            int r = pawnPlace / 8, f = pawnPlace % 8;
            if (f >= 2 && f <= 5)
            {
                if (r <= 4) pawnChainBlack += 6;
                if (r <= 3) pawnChainBlack += 6;
                if ((r == 3 || r == 2) && ChebyshevDistance(pawnPlace, whiteKingSq) <= 3)
                {
                    pawnChainBlack += TaperGroup1Value(14, 2, phase);
                }
            }
        }
    }
    int blackPawnSum = doubledPawnValueBlack + singlePastBlack + isolatedPawnValueBlack + goForwardPawnBlack + pawnChainBlack;

    if (phase >= 12) {
        // Space advance for c4 when d4 is present
        if (thisBoard.mainBoard[26] == 1 && thisBoard.mainBoard[27] == 1 && thisBoard.mainBoard[34] != 9) {
            whitePawnSum += 20;
        }
        if (thisBoard.mainBoard[34] == 9 && thisBoard.mainBoard[35] == 9 && thisBoard.mainBoard[26] != 1) {
            blackPawnSum += 20;
        }

        // Flank passer with Rook support
        for (int sq : thisBoard.pieces[1]) {
            int f = sq % 8, r = sq / 8;
            if (r >= 5 && (f <= 1 || f >= 6)) {
                if ((PassedPawnSetup::WhitePassedMask[sq] & blackPawns) == 0) {
                    bool rookBehind = false;
                    for (int rsq : thisBoard.pieces[4]) {
                        if (rsq % 8 == f && rsq / 8 < r) { rookBehind = true; break; }
                    }
                    whitePawnSum += (rookBehind ? 60 : 35);
                }
            }
        }
        for (int sq : thisBoard.pieces[9]) {
            int f = sq % 8, r = sq / 8;
            if (r <= 2 && (f <= 1 || f >= 6)) {
                if ((PassedPawnSetup::BlackPassedMask[sq] & whitePawns) == 0) {
                    bool rookBehind = false;
                    for (int rsq : thisBoard.pieces[12]) {
                        if (rsq % 8 == f && rsq / 8 > r) { rookBehind = true; break; }
                    }
                    blackPawnSum += (rookBehind ? 60 : 35);
                }
            }
        }
    }

    return whitePawnSum - blackPawnSum;
}

MovementResult EvaluationLogic::PieceMoveCountFast(Board &thisBoard, int phase)
{
    EvaluationContext ctx(thisBoard, phase);
    ctx.InitializeAttacks();
    return PieceMoveCountFast(thisBoard, phase, ctx);
}

MovementResult EvaluationLogic::PieceMoveCountFast(Board &thisBoard, int phase, const EvaluationContext& ctx)
{
    long long whitePieces = thisBoard.whitePieces;
    long long blackPieces = thisBoard.blackPieces;
    int *mainBoard = thisBoard.mainBoard;
    long long wholeBoard = whitePieces | blackPieces;
    int movement = 0;
    int centerValue = 0;
    int whiteAttackValue = 0;
    int blackAttackValue = 0;
    int whiteRookFileBonus = 0;
    int blackRookFileBonus = 0;
    int moveCount;
    const long long bpa = ctx.pawnAttacks[1];
    const long long wpa = ctx.pawnAttacks[0];
    const auto centerSum = [](uint64_t attacks, bool black, int type, const int* values) {
        attacks &= Option::MoveCenterNonzero[black ? 1 : 0][type];
        int sum = 0;
        while (attacks)
        {
            const int square = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            sum += values[square];
        }
        return sum;
    };
    const auto taperedTable = [phase](const auto& values, int index)
    {
        return TaperEvaluationValue(values[0][index], values[2][index], phase);
    };
    const auto taperedGroup1Table = [phase](const auto& values, int index)
    {
        return TaperGroup1Value(values[0][index], values[2][index], phase);
    };
    const auto taperedGroup2Table = [phase](const auto& values, int index)
    {
        return TaperGroup2Value(values[0][index], values[2][index], phase);
    };
    {
        for (int piece = 1; piece < 7; piece++)
        {
            switch (piece)
            {
            case 1:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement += taperedTable(Option::PawnInValueWhite, piecePoisiion);
                    centerValue += Option::PawnInCenterValueWhite[piecePoisiion];
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][0] != nullptr)
                    {
                        if ((PieceMoves::pawnTwoMove[piecePoisiion] & wholeBoard) == 0)
                        {
                            // pieceMovePosition[0][1][piecePoisiion + 16][piece]++;
                            moveCount++;
                            // movement += taperedTable(Option::PawnMoveValueWhite, piecePoisiion + 16);
                            // whitePieceAttack
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][1] != nullptr && (Option::PowerTwo[piecePoisiion + 8] & wholeBoard) == 0)
                    {
                        // pieceMovePosition[0][1][piecePoisiion + 8][piece]++;
                        moveCount++;
                        // movement += taperedTable(Option::PawnMoveValueWhite, piecePoisiion + 8);
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][6] != nullptr && piecePoisiion + 7 == thisBoard.unpassentPlace)
                    {
                        // pieceMovePosition[0][2][piecePoisiion + 7][piece]++;
                        whiteAttackValue += taperedGroup1Table(Option::PawnAttackValue, 9);
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][7] != nullptr && piecePoisiion + 9 == thisBoard.unpassentPlace)
                    {
                        // pieceMovePosition[0][2][piecePoisiion + 9][piece]++;
                        whiteAttackValue += taperedGroup1Table(Option::PawnAttackValue, 9);
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][8] != nullptr && (Option::PowerTwo[piecePoisiion + 7] & blackPieces) != 0)
                    {
                        // pieceMovePosition[0][2][piecePoisiion + 7][piece]++;
                        whiteAttackValue += taperedGroup1Table(Option::PawnAttackValue, mainBoard[piecePoisiion + 7]);
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][13] != nullptr && (Option::PowerTwo[piecePoisiion + 9] & blackPieces) != 0)
                    {
                        // pieceMovePosition[0][2][piecePoisiion + 9][piece]++;
                        whiteAttackValue += taperedGroup1Table(Option::PawnAttackValue, mainBoard[piecePoisiion + 9]);
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][8] != nullptr)
                    {
                        centerValue += Option::PawnMoveCenterValueWhite[piecePoisiion + 7];
                    }
                    if (PieceMoves::WhitePawnMoves[piecePoisiion][13] != nullptr)
                    {
                        centerValue += Option::PawnMoveCenterValueWhite[piecePoisiion + 9];
                    }
                    movement += taperedTable(Option::PawnMoveCountValue, moveCount);
                }
                break;
            case 2:
            {
                uint64_t whiteOutpostHolesAwarded = 0;
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement += taperedTable(Option::KnightInValueWhite, piecePoisiion);
                    if (phase >= 16 && (piecePoisiion % 8 == 0 || piecePoisiion % 8 == 7)) movement -= 15;
                    movement += KnightOutpostValue(thisBoard, piecePoisiion, true, phase);
                    centerValue += Option::KnightInCenterValueWhite[piecePoisiion];
                    const uint64_t attacks = ctx.attacks[piecePoisiion];
                    centerValue += centerSum(attacks, false, 2, Option::KnightMoveCenterValueWhite);
                    moveCount = __builtin_popcountll(attacks & ~wholeBoard & ~bpa);
                    uint64_t holes = attacks & ~wholeBoard & ~whiteOutpostHolesAwarded;
                    while (holes)
                    {
                        const int endPlace = __builtin_ctzll(holes);
                        holes &= holes - 1;
                        if (KnightOutpostAdvanced[1][endPlace] &&
                            (KnightOutpostChallengeMask[1][endPlace] & thisBoard.blackPawns) == 0 &&
                            (KnightOutpostSupportMask[1][endPlace] & thisBoard.whitePawns) != 0)
                        {
                            whiteOutpostHolesAwarded |= Option::PowerTwo[endPlace];
                            movement += TaperGroup1Value(10, 2, phase) * KnightOutpostFileScale[endPlace] / 100;
                        }
                    }
                    uint64_t captures = attacks & blackPieces;
                    while (captures)
                    {
                        const int endPlace = __builtin_ctzll(captures);
                        captures &= captures - 1;
                        whiteAttackValue += taperedGroup1Table(Option::KnightAttackValue, mainBoard[endPlace]);
                        if (mainBoard[endPlace] == 9 &&
                            (ctx.legacyAttacks[1] & Option::PowerTwo[endPlace]) == 0)
                        {
                            const int f = endPlace % 8, r = endPlace / 8;
                            whiteAttackValue += (f >= 2 && f <= 5 && r >= 2 && r <= 5) ? 32 : 16;
                        }
                    }
                    movement += taperedGroup1Table(Option::KnightMoveCountValue, moveCount);
                }
                break;
            }
            case 3:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement += taperedTable(Option::BishopInValueWhite, piecePoisiion);
                    centerValue += Option::BishopInCenterValueWhite[piecePoisiion];
                    const uint64_t attacks = ctx.attacks[piecePoisiion];
                    moveCount = __builtin_popcountll(attacks & ~wholeBoard & ~bpa);
                    centerValue += centerSum(attacks, false, 3, Option::BishopMoveCenterValueWhite);
                    uint64_t captures = attacks & blackPieces;
                    while (captures)
                    {
                        const int endPos = __builtin_ctzll(captures);
                        captures &= captures - 1;
                        const int endPiece = mainBoard[endPos];
                        whiteAttackValue += taperedGroup1Table(Option::BishopAttackValue, endPiece);
                        if (endPiece == 9 && (ctx.legacyAttacks[1] & Option::PowerTwo[endPos]) == 0)
                        {
                            const int f = endPos % 8, r = endPos / 8;
                            whiteAttackValue += (f >= 2 && f <= 5 && r >= 2 && r <= 5) ? 32 : 16;
                        }
                    }
                    movement += taperedGroup1Table(Option::BishopMoveCountValue, moveCount);
                }
                break;
            case 4:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    int file = piecePoisiion % 8;
                    unsigned long long fileMask = 0x0101010101010101ULL << file;
                    bool friendlyPawn = (thisBoard.whitePawns & fileMask) != 0;
                    if (!friendlyPawn)
                    {
                        bool enemyPawn = (thisBoard.blackPawns & fileMask) != 0;
                        if (!enemyPawn)
                        {
                            whiteRookFileBonus += TaperGroup2Value(Option::RookOpenFileMiddleGame, Option::RookOpenFileEndGame, phase);
                        }
                        else
                        {
                            whiteRookFileBonus += TaperGroup2Value(Option::RookSemiOpenFileMiddleGame, Option::RookSemiOpenFileEndGame, phase);
                            unsigned long long pawnsOnFile = thisBoard.blackPawns & fileMask;
                            if (pawnsOnFile) {
                                int targetSq = __builtin_ctzll(pawnsOnFile);
                                int targetRank = targetSq / 8;
                                if ((thisBoard.blackPawns & AdjacentFilesMask[file] & RankGeMask[targetRank]) == 0) {
                                    whiteRookFileBonus += 25;
                                }
                            }
                        }
                    }
                    moveCount = 0;
                    movement += taperedGroup2Table(Option::RookInValueWhite, piecePoisiion);
                    centerValue += Option::RookInCenterValueWhite[piecePoisiion];
                    const uint64_t attacks = ctx.attacks[piecePoisiion];
                    moveCount = __builtin_popcountll(attacks & ~wholeBoard);
                    centerValue += centerSum(attacks, false, 4, Option::RookMoveCenterValueWhite);
                    uint64_t captures = attacks & blackPieces;
                    while (captures)
                    {
                        const int endPos = __builtin_ctzll(captures);
                        captures &= captures - 1;
                        const int endPiece = mainBoard[endPos];
                        whiteAttackValue += taperedGroup2Table(Option::RookAttackValue, endPiece);
                    }
                    movement += taperedGroup2Table(Option::RookMoveCountValue, moveCount);
                }
                break;
            case 5:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement += taperedGroup2Table(Option::QueenInValueWhite, piecePoisiion);
                    centerValue += Option::QueenInCenterValueWhite[piecePoisiion];
                    int qFile = piecePoisiion % 8;
                    unsigned long long qFileMask = 0x0101010101010101ULL << qFile;
                    if ((thisBoard.whitePawns & qFileMask) == 0 && (thisBoard.blackPawns & qFileMask) != 0)
                    {
                        unsigned long long pawnsOnFile = thisBoard.blackPawns & qFileMask;
                        int targetSq = __builtin_ctzll(pawnsOnFile);
                        int targetRank = targetSq / 8;
                        if ((thisBoard.blackPawns & AdjacentFilesMask[qFile] & RankGeMask[targetRank]) == 0) {
                            movement += 20;
                            for (int rsq : thisBoard.pieces[4]) {
                                if (rsq % 8 == qFile) { movement += 15; break; }
                            }
                        }
                    }
                    const uint64_t attacks = ctx.attacks[piecePoisiion];
                    moveCount = __builtin_popcountll(attacks & ~wholeBoard);
                    centerValue += centerSum(attacks, false, 5, Option::QueenMoveCenterValueWhite);
                    uint64_t captures = attacks & blackPieces;
                    while (captures)
                    {
                        const int endPos = __builtin_ctzll(captures);
                        captures &= captures - 1;
                        const int endPiece = mainBoard[endPos];
                        whiteAttackValue += taperedGroup1Table(Option::QueenAttackValue, endPiece);
                    }
                    movement += taperedGroup1Table(Option::QueenMoveCountValue, moveCount);
                }
                break;
            case 6:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement += taperedTable(Option::KingInValueWhite, piecePoisiion);
                    centerValue += Option::KingInCenterValueWhite[piecePoisiion];
                    const uint64_t attacks = ctx.attacks[piecePoisiion];
                    moveCount = __builtin_popcountll(attacks & ~wholeBoard);
                    centerValue += centerSum(attacks, false, 6, Option::KingMoveCenterValueWhite);
                    uint64_t captures = attacks & blackPieces;
                    while (captures)
                    {
                        const int endPos = __builtin_ctzll(captures);
                        captures &= captures - 1;
                        const int endPiece = mainBoard[endPos];
                        whiteAttackValue += taperedTable(Option::KingAttackValue, endPiece);
                    }
                    movement += taperedTable(Option::KingMoveCountValue, moveCount);
                }
                break;
            }
        }
    }
    {
        for (int piece = 9; piece < 15; piece++)
        {
            switch (piece)
            {
            case 9:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement -= taperedTable(Option::PawnInValueBlack, piecePoisiion);
                    centerValue -= Option::PawnInCenterValueBlack[piecePoisiion];

                    if (PieceMoves::BlackPawnMoves[piecePoisiion][0] != nullptr)
                    {
                        if ((PieceMoves::pawnTwoMove[piecePoisiion] & wholeBoard) == 0)
                        {
                            // pieceMovePosition[1][1][piecePoisiion - 20][piece - 8]++;
                            moveCount++;
                            // movement -= taperedTable(Option::PawnMoveValueBlack, piecePoisiion - 16);
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][1] != nullptr && (Option::PowerTwo[piecePoisiion - 8] & wholeBoard) == 0)
                    {
                        // pieceMovePosition[1][1][piecePoisiion - 10][piece - 8]++;
                        moveCount++;
                        // movement -= taperedTable(Option::PawnMoveValueBlack, piecePoisiion - 8);
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][6] != nullptr && piecePoisiion - 7 == thisBoard.unpassentPlace)
                    {
                        // pieceMovePosition[1][2][piecePoisiion - 7][piece - 8]++;
                        blackAttackValue += taperedGroup1Table(Option::PawnAttackValue, 1);
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][7] != nullptr && piecePoisiion - 9 == thisBoard.unpassentPlace)
                    {
                        // pieceMovePosition[1][2][piecePoisiion - 9][piece - 8]++;
                        blackAttackValue += taperedGroup1Table(Option::PawnAttackValue, 1);
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][8] != nullptr && (Option::PowerTwo[piecePoisiion - 7] & whitePieces) != 0)
                    {
                        // pieceMovePosition[1][2][piecePoisiion - 7][piece - 8]++;
                        blackAttackValue += taperedGroup1Table(Option::PawnAttackValue, mainBoard[piecePoisiion - 7]);
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][13] != nullptr && (Option::PowerTwo[piecePoisiion - 9] & whitePieces) != 0)
                    {
                        // pieceMovePosition[1][2][piecePoisiion - 9][piece - 8]++;
                        blackAttackValue += taperedGroup1Table(Option::PawnAttackValue, mainBoard[piecePoisiion - 9]);
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][8] != nullptr)
                    {
                        centerValue -= Option::PawnMoveCenterValueBlack[piecePoisiion - 7];
                    }
                    if (PieceMoves::BlackPawnMoves[piecePoisiion][13] != nullptr)
                    {
                        centerValue -= Option::PawnMoveCenterValueBlack[piecePoisiion - 9];
                    }
                    movement -= taperedTable(Option::PawnMoveCountValue, moveCount);
                }
                break;
            case 10:
            {
                uint64_t blackOutpostHolesAwarded = 0;
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement -= taperedTable(Option::KnightInValueBlack, piecePoisiion);
                    if (phase >= 16 && (piecePoisiion % 8 == 0 || piecePoisiion % 8 == 7)) movement += 15;
                    movement -= KnightOutpostValue(thisBoard, piecePoisiion, false, phase);
                    centerValue -= Option::KnightInCenterValueBlack[piecePoisiion];
                    const uint64_t attacks = ctx.attacks[piecePoisiion];
                    centerValue -= centerSum(attacks, true, 2, Option::KnightMoveCenterValueBlack);
                    moveCount = __builtin_popcountll(attacks & ~wholeBoard & ~wpa);
                    uint64_t holes = attacks & ~wholeBoard & ~blackOutpostHolesAwarded;
                    while (holes)
                    {
                        const int endPlace = __builtin_ctzll(holes);
                        holes &= holes - 1;
                        if (KnightOutpostAdvanced[0][endPlace] &&
                            (KnightOutpostChallengeMask[0][endPlace] & thisBoard.whitePawns) == 0 &&
                            (KnightOutpostSupportMask[0][endPlace] & thisBoard.blackPawns) != 0)
                        {
                            blackOutpostHolesAwarded |= Option::PowerTwo[endPlace];
                            movement -= TaperGroup1Value(10, 2, phase) * KnightOutpostFileScale[endPlace] / 100;
                        }
                    }
                    uint64_t captures = attacks & whitePieces;
                    while (captures)
                    {
                        const int endPlace = __builtin_ctzll(captures);
                        captures &= captures - 1;
                        blackAttackValue += taperedGroup1Table(Option::KnightAttackValue, mainBoard[endPlace]);
                        if (mainBoard[endPlace] == 1 &&
                            (ctx.legacyAttacks[0] & Option::PowerTwo[endPlace]) == 0)
                        {
                            const int f = endPlace % 8, r = endPlace / 8;
                            blackAttackValue += (f >= 2 && f <= 5 && r >= 2 && r <= 5) ? 32 : 16;
                        }
                    }
                    movement -= taperedGroup1Table(Option::KnightMoveCountValue, moveCount);
                }
                break;
            }
            case 11:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement -= taperedTable(Option::BishopInValueBlack, piecePoisiion);
                    centerValue -= Option::BishopInCenterValueBlack[piecePoisiion];

                    const uint64_t attacks = ctx.attacks[piecePoisiion];
                    moveCount = __builtin_popcountll(attacks & ~wholeBoard & ~wpa);
                    centerValue -= centerSum(attacks, true, 3, Option::BishopMoveCenterValueBlack);
                    uint64_t captures = attacks & whitePieces;
                    while (captures)
                    {
                        const int endPos = __builtin_ctzll(captures);
                        captures &= captures - 1;
                        const int endPiece = mainBoard[endPos];
                        blackAttackValue += taperedGroup1Table(Option::BishopAttackValue, endPiece);
                        if (endPiece == 1 && (ctx.legacyAttacks[0] & Option::PowerTwo[endPos]) == 0)
                        {
                            const int f = endPos % 8, r = endPos / 8;
                            blackAttackValue += (f >= 2 && f <= 5 && r >= 2 && r <= 5) ? 32 : 16;
                        }
                    }
                    movement -= taperedGroup1Table(Option::BishopMoveCountValue, moveCount);
                }
                break;
            case 12:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    int file = piecePoisiion % 8;
                    unsigned long long fileMask = 0x0101010101010101ULL << file;
                    bool friendlyPawn = (thisBoard.blackPawns & fileMask) != 0;
                    if (!friendlyPawn)
                    {
                        bool enemyPawn = (thisBoard.whitePawns & fileMask) != 0;
                        if (!enemyPawn)
                        {
                            blackRookFileBonus += TaperGroup2Value(Option::RookOpenFileMiddleGame, Option::RookOpenFileEndGame, phase);
                        }
                        else
                        {
                            blackRookFileBonus += TaperGroup2Value(Option::RookSemiOpenFileMiddleGame, Option::RookSemiOpenFileEndGame, phase);
                            unsigned long long pawnsOnFile = thisBoard.whitePawns & fileMask;
                            if (pawnsOnFile) {
                                int targetSq = 63 - __builtin_clzll(pawnsOnFile);
                                int targetRank = targetSq / 8;
                                if ((thisBoard.whitePawns & AdjacentFilesMask[file] & RankLeMask[targetRank]) == 0) {
                                    blackRookFileBonus += 25;
                                }
                            }
                        }
                    }
                    moveCount = 0;
                    movement -= taperedGroup2Table(Option::RookInValueBlack, piecePoisiion);
                    centerValue -= Option::RookInCenterValueBlack[piecePoisiion];

                    const uint64_t attacks = ctx.attacks[piecePoisiion];
                    moveCount = __builtin_popcountll(attacks & ~wholeBoard);
                    centerValue -= centerSum(attacks, true, 4, Option::RookMoveCenterValueBlack);
                    uint64_t captures = attacks & whitePieces;
                    while (captures)
                    {
                        const int endPos = __builtin_ctzll(captures);
                        captures &= captures - 1;
                        const int endPiece = mainBoard[endPos];
                        blackAttackValue += taperedGroup2Table(Option::RookAttackValue, endPiece);
                    }
                    movement -= taperedGroup2Table(Option::RookMoveCountValue, moveCount);
                }
                break;
            case 13:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement -= taperedGroup2Table(Option::QueenInValueBlack, piecePoisiion);
                    centerValue -= Option::QueenInCenterValueBlack[piecePoisiion];
                    int qFile = piecePoisiion % 8;
                    unsigned long long qFileMask = 0x0101010101010101ULL << qFile;
                    if ((thisBoard.blackPawns & qFileMask) == 0 && (thisBoard.whitePawns & qFileMask) != 0)
                    {
                        unsigned long long pawnsOnFile = thisBoard.whitePawns & qFileMask;
                        int targetSq = 63 - __builtin_clzll(pawnsOnFile);
                        int targetRank = targetSq / 8;
                        if ((thisBoard.whitePawns & AdjacentFilesMask[qFile] & RankLeMask[targetRank]) == 0) {
                            movement -= 20;
                            for (int rsq : thisBoard.pieces[12]) {
                                if (rsq % 8 == qFile) { movement -= 15; break; }
                            }
                        }
                    }

                    const uint64_t attacks = ctx.attacks[piecePoisiion];
                    moveCount = __builtin_popcountll(attacks & ~wholeBoard);
                    centerValue -= centerSum(attacks, true, 5, Option::QueenMoveCenterValueBlack);
                    uint64_t captures = attacks & whitePieces;
                    while (captures)
                    {
                        const int endPos = __builtin_ctzll(captures);
                        captures &= captures - 1;
                        const int endPiece = mainBoard[endPos];
                        blackAttackValue += taperedGroup1Table(Option::QueenAttackValue, endPiece);
                    }
                    movement -= taperedGroup1Table(Option::QueenMoveCountValue, moveCount);
                }
                break;
            case 14:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement -= taperedTable(Option::KingInValueBlack, piecePoisiion);
                    centerValue -= Option::KingInCenterValueBlack[piecePoisiion];
                    const uint64_t attacks = ctx.attacks[piecePoisiion];
                    moveCount = __builtin_popcountll(attacks & ~wholeBoard);
                    centerValue -= centerSum(attacks, true, 6, Option::KingMoveCenterValueBlack);
                    uint64_t captures = attacks & whitePieces;
                    while (captures)
                    {
                        const int endPos = __builtin_ctzll(captures);
                        captures &= captures - 1;
                        const int endPiece = mainBoard[endPos];
                        blackAttackValue += taperedTable(Option::KingAttackValue, endPiece);
                    }
                    movement -= taperedTable(Option::KingMoveCountValue, moveCount);
                }
                break;
            }
        }
    }
    int scaledAttackNet = ((whiteAttackValue - blackAttackValue) * 135) / 100;
    int rookFileNet = whiteRookFileBonus - blackRookFileBonus;
    movement += scaledAttackNet;
    movement += rookFileNet;

    
    bool whiteCastled = (!thisBoard.whiteSmallCastle && !thisBoard.whiteBigCastle && (thisBoard.pieces[6].front() == 6 || thisBoard.pieces[6].front() == 2));
    bool blackCastled = (!thisBoard.blackSmallCastle && !thisBoard.blackBigCastle && (thisBoard.pieces[14].front() == 62 || thisBoard.pieces[14].front() == 58));

    // Premature Queen activity with undeveloped sleeping minor pieces
    if (phase >= 16) {
        int sleepWhite = 0;
        for (int sq : thisBoard.pieces[2]) if (sq == 1 || sq == 6) sleepWhite++;
        for (int sq : thisBoard.pieces[3]) if (sq == 2 || sq == 5) sleepWhite++;
        if (sleepWhite > 0) {
            for (int sq : thisBoard.pieces[5]) {
                int rank = sq / 8;
                if (rank >= 2) {
                    int advance = rank - 1;
                    int pen = 14 * advance * sleepWhite;
                    if (advance >= 2 && sleepWhite >= 2) pen += 15;
                    if (whiteCastled) pen /= 2;
                    movement -= pen;
                }
            }
        }

        int sleepBlack = 0;
        for (int sq : thisBoard.pieces[10]) if (sq == 57 || sq == 62) sleepBlack++;
        for (int sq : thisBoard.pieces[11]) if (sq == 58 || sq == 61) sleepBlack++;
        if (sleepBlack > 0) {
            for (int sq : thisBoard.pieces[13]) {
                int rank = sq / 8;
                if (rank <= 5) {
                    int advance = 6 - rank;
                    int pen = 14 * advance * sleepBlack;
                    if (advance >= 2 && sleepBlack >= 2) pen += 15;
                    if (blackCastled) pen /= 2;
                    movement += pen;
                }
            }
        }
    }

    // Pinned vulnerable pawn (chess-rational vulnerability model)
    const int whiteKingSq = thisBoard.pieces[6].front();
    const int whiteKingR = whiteKingSq / 8, whiteKingC = whiteKingSq % 8;
    const bool whiteCentralKing = (whiteKingR <= 1 && whiteKingC >= 2 && whiteKingC <= 5);
    for (int psq : thisBoard.pieces[1]) {
        const auto& pin = PinData.data[psq][whiteKingSq];
        if (!pin.isAligned) continue;
        if ((AttackPlaces::BetweenMask[psq][whiteKingSq] & wholeBoard) != 0) continue;

        int r = psq / 8, c = psq % 8;
        for (int cr = r - pin.stepR, cc = c - pin.stepC; cr >= 0 && cr < 8 && cc >= 0 && cc < 8; cr -= pin.stepR, cc -= pin.stepC) {
            int p = mainBoard[cr * 8 + cc];
            if (p != 0) {
                bool isSlider = pin.isOrthogonal ? (p == 12 || p == 13) : (p == 11 || p == 13);
                if (isSlider) {
                    int sliderSq = cr * 8 + cc;
                    bool pawnDefended = (AttackPlaces::BlackPawnAttackPlaces[psq] & thisBoard.whitePawns) != 0;
                    bool kingDefended = (AttackPlaces::KingAttackPlaces[whiteKingSq] & (1ULL << psq)) != 0;
                    bool pieceDefended = false;
                    for (int rsq : thisBoard.pieces[4]) {
                        if (AttackPlaces::RookAttack[rsq][psq] && (AttackPlaces::BetweenMask[rsq][psq] & wholeBoard) == 0) {
                            pieceDefended = true; break;
                        }
                    }
                    if (!pieceDefended) {
                        for (int nsq : thisBoard.pieces[2]) {
                            if (AttackPlaces::KnightAttackPlaces[nsq] & (1ULL << psq)) { pieceDefended = true; break; }
                        }
                    }
                    if (!pieceDefended) {
                        for (int bsq : thisBoard.pieces[3]) {
                            if (AttackPlaces::BishopAttack[bsq][psq] && (AttackPlaces::BetweenMask[bsq][psq] & wholeBoard) == 0) {
                                pieceDefended = true; break;
                            }
                        }
                    }
                    if (!pieceDefended) {
                        for (int qsq : thisBoard.pieces[5]) {
                            if (AttackPlaces::QueenAttack[qsq][psq] && (AttackPlaces::BetweenMask[qsq][psq] & wholeBoard) == 0) {
                                pieceDefended = true; break;
                            }
                        }
                    }

                    int defenders = (pawnDefended ? 1 : 0) + (kingDefended ? 1 : 0) + (pieceDefended ? 1 : 0);
                    int attackers = 1; // pinning slider
                    attackers += __builtin_popcountll(AttackPlaces::WhitePawnAttackPlaces[psq] & thisBoard.blackPawns);
                    for (int nsq : thisBoard.pieces[10]) {
                        if (AttackPlaces::KnightAttackPlaces[nsq] & (1ULL << psq)) attackers++;
                    }
                    for (int bsq : thisBoard.pieces[11]) {
                        if (bsq != sliderSq && AttackPlaces::BishopAttack[bsq][psq] && (AttackPlaces::BetweenMask[bsq][psq] & wholeBoard) == 0) attackers++;
                    }
                    for (int rsq : thisBoard.pieces[12]) {
                        if (rsq != sliderSq && AttackPlaces::RookAttack[rsq][psq] && (AttackPlaces::BetweenMask[rsq][psq] & wholeBoard) == 0) attackers++;
                    }

                    if (defenders == 0) {
                        movement -= 40;
                    } else if (attackers > defenders) {
                        movement -= 24;
                    } else if (whiteCentralKing && (c == 3 || c == 4) && !pawnDefended) {
                        movement -= 12;
                    }
                }
                break;
            }
        }
    }
    const int blackKingSq = thisBoard.pieces[14].front();
    const int blackKingR = blackKingSq / 8, blackKingC = blackKingSq % 8;
    const bool blackCentralKing = (blackKingR >= 6 && blackKingC >= 2 && blackKingC <= 5);
    for (int psq : thisBoard.pieces[9]) {
        const auto& pin = PinData.data[psq][blackKingSq];
        if (!pin.isAligned) continue;
        if ((AttackPlaces::BetweenMask[psq][blackKingSq] & wholeBoard) != 0) continue;

        int r = psq / 8, c = psq % 8;
        for (int cr = r - pin.stepR, cc = c - pin.stepC; cr >= 0 && cr < 8 && cc >= 0 && cc < 8; cr -= pin.stepR, cc -= pin.stepC) {
            int p = mainBoard[cr * 8 + cc];
            if (p != 0) {
                bool isSlider = pin.isOrthogonal ? (p == 4 || p == 5) : (p == 3 || p == 5);
                if (isSlider) {
                    int sliderSq = cr * 8 + cc;
                    bool pawnDefended = (AttackPlaces::WhitePawnAttackPlaces[psq] & thisBoard.blackPawns) != 0;
                    bool kingDefended = (AttackPlaces::KingAttackPlaces[blackKingSq] & (1ULL << psq)) != 0;
                    bool pieceDefended = false;
                    for (int rsq : thisBoard.pieces[12]) {
                        if (AttackPlaces::RookAttack[rsq][psq] && (AttackPlaces::BetweenMask[rsq][psq] & wholeBoard) == 0) {
                            pieceDefended = true; break;
                        }
                    }
                    if (!pieceDefended) {
                        for (int nsq : thisBoard.pieces[10]) {
                            if (AttackPlaces::KnightAttackPlaces[nsq] & (1ULL << psq)) { pieceDefended = true; break; }
                        }
                    }
                    if (!pieceDefended) {
                        for (int bsq : thisBoard.pieces[11]) {
                            if (AttackPlaces::BishopAttack[bsq][psq] && (AttackPlaces::BetweenMask[bsq][psq] & wholeBoard) == 0) {
                                pieceDefended = true; break;
                            }
                        }
                    }
                    if (!pieceDefended) {
                        for (int qsq : thisBoard.pieces[13]) {
                            if (AttackPlaces::QueenAttack[qsq][psq] && (AttackPlaces::BetweenMask[qsq][psq] & wholeBoard) == 0) {
                                pieceDefended = true; break;
                            }
                        }
                    }

                    int defenders = (pawnDefended ? 1 : 0) + (kingDefended ? 1 : 0) + (pieceDefended ? 1 : 0);
                    int attackers = 1; // pinning slider
                    attackers += __builtin_popcountll(AttackPlaces::BlackPawnAttackPlaces[psq] & thisBoard.whitePawns);
                    for (int nsq : thisBoard.pieces[2]) {
                        if (AttackPlaces::KnightAttackPlaces[nsq] & (1ULL << psq)) attackers++;
                    }
                    for (int bsq : thisBoard.pieces[3]) {
                        if (bsq != sliderSq && AttackPlaces::BishopAttack[bsq][psq] && (AttackPlaces::BetweenMask[bsq][psq] & wholeBoard) == 0) attackers++;
                    }
                    for (int rsq : thisBoard.pieces[4]) {
                        if (rsq != sliderSq && AttackPlaces::RookAttack[rsq][psq] && (AttackPlaces::BetweenMask[rsq][psq] & wholeBoard) == 0) attackers++;
                    }

                    if (defenders == 0) {
                        movement += 40;
                    } else if (attackers > defenders) {
                        movement += 24;
                    } else if (blackCentralKing && (c == 3 || c == 4) && !pawnDefended) {
                        movement += 12;
                    }
                }
                break;
            }
        }
    }

    // Rook obstruction: rook on 3rd rank directly blocking unmoved 2nd rank pawn
    for (int sq : thisBoard.pieces[4]) {
        if (sq / 8 == 2 && thisBoard.mainBoard[sq - 8] == 1) movement -= 25;
    }
    for (int sq : thisBoard.pieces[12]) {
        if (sq / 8 == 5 && thisBoard.mainBoard[sq + 8] == 9) movement += 25;
    }

    if (phase >= 12) {
        const int wKingSq = thisBoard.pieces[6].front();
        const int bKingSq = thisBoard.pieces[14].front();
        const int wKr = wKingSq / 8, wKf = wKingSq % 8;
        const int bKr = bKingSq / 8, bKf = bKingSq % 8;
        for (int qSq : thisBoard.pieces[13]) {
            int qr = qSq / 8;
            if (qr <= 1 && wKr <= 1 && wKf >= 2 && wKf <= 5) {
                movement -= 85;
            }
        }
        for (int qSq : thisBoard.pieces[5]) {
            int qr = qSq / 8;
            if (qr >= 6 && bKr >= 6 && bKf >= 2 && bKf <= 5) {
                movement += 85;
            }
        }
    }

    // Hanging central pawn attacked by Queen
    static const int whiteCentralPawns[4] = {18, 27, 28, 21}; // c3, d4, e4, f3
    for (int sq : whiteCentralPawns) {
        if (mainBoard[sq] == 1) {
            bool attackedByQ = false;
            for (int qSq : thisBoard.pieces[13]) {
                if (AttackPlaces::QueenAttack[qSq][sq] && (AttackPlaces::BetweenMask[qSq][sq] & wholeBoard) == 0) {
                    attackedByQ = true; break;
                }
            }
            if (attackedByQ) {
                bool defended = ((ctx.legacyAttacks[0] & Option::PowerTwo[sq]) != 0);
                if (!defended) {
                    movement -= 40;
                } else {
                    for (int qSq : thisBoard.pieces[5]) {
                        if (AttackPlaces::QueenAttack[qSq][sq] && (AttackPlaces::BetweenMask[qSq][sq] & wholeBoard) == 0) {
                            movement += 15; break;
                        }
                    }
                }
            }
        }
    }
    static const int blackCentralPawns[4] = {42, 35, 36, 45}; // c6, d5, e5, f6
    for (int sq : blackCentralPawns) {
        if (mainBoard[sq] == 9) {
            bool attackedByQ = false;
            for (int qSq : thisBoard.pieces[5]) {
                if (AttackPlaces::QueenAttack[qSq][sq] && (AttackPlaces::BetweenMask[qSq][sq] & wholeBoard) == 0) {
                    attackedByQ = true; break;
                }
            }
            if (attackedByQ) {
                bool defended = ((ctx.legacyAttacks[1] & Option::PowerTwo[sq]) != 0);
                if (!defended) {
                    movement += 40;
                } else {
                    for (int qSq : thisBoard.pieces[13]) {
                        if (AttackPlaces::QueenAttack[qSq][sq] && (AttackPlaces::BetweenMask[qSq][sq] & wholeBoard) == 0) {
                            movement -= 15; break;
                        }
                    }
                }
            }
        }
    }

    // Uncontested central knight outpost on d5 / d4
    for (int nSq : thisBoard.pieces[2]) {
        if (nSq == 35) { // d5
            bool canChallenge = false;
            for (int pSq : thisBoard.pieces[9]) {
                int pf = pSq % 8, pr = pSq / 8;
                if (pf == 2 && pr > 4) canChallenge = true;
                if (pf == 4 && pr > 4) {
                    if (mainBoard[36] != 1) canChallenge = true; // e5 pawn blocks e-file pawns
                }
            }
            if (!canChallenge) movement += 16;
        }
    }
    for (int nSq : thisBoard.pieces[10]) {
        if (nSq == 27) { // d4
            bool canChallenge = false;
            for (int pSq : thisBoard.pieces[1]) {
                int pf = pSq % 8, pr = pSq / 8;
                if (pf == 2 && pr < 3) canChallenge = true;
                if (pf == 4 && pr < 3) {
                    if (mainBoard[28] != 9) canChallenge = true; // e4 pawn blocks e-file pawns
                }
            }
            if (!canChallenge) movement -= 16;
        }
    }

    


    MovementResult result;
    result.movement = movement;
    result.attackNet = scaledAttackNet;
    result.center = centerValue;
    result.rookFileNet = rookFileNet;
    result.whiteRookFile = whiteRookFileBonus;
    result.blackRookFile = blackRookFileBonus;
    return result;
}

int *EvaluationLogic::PieceMoveCount(Board &thisBoard, int phase)
{
    MovementResult res = PieceMoveCountFast(thisBoard, phase);
    int *movementAndKingSafetyAndCenter = new int[6];
    movementAndKingSafetyAndCenter[0] = res.movement;
    movementAndKingSafetyAndCenter[1] = res.attackNet;
    movementAndKingSafetyAndCenter[2] = res.center;
    movementAndKingSafetyAndCenter[3] = res.rookFileNet;
    movementAndKingSafetyAndCenter[4] = res.whiteRookFile;
    movementAndKingSafetyAndCenter[5] = res.blackRookFile;
    return movementAndKingSafetyAndCenter;
}
