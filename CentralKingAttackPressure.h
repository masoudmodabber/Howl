#ifndef CENTRALKINGATTACKPRESSURE_H
#define CENTRALKINGATTACKPRESSURE_H

#include "AttackPlaces.h"
#include "Board.h"
#include "BoardLogic.h"
#include "Option.h"

#include <algorithm>
#include <cstdlib>

namespace CentralKingAttackPressure
{
struct Weights
{
    int innerMinorPressure;
    int outerMinorPressure;
    int readinessLagWeight;
    int pressureScale;
};

inline Weights ProductionWeights()
{
    return {Option::CentralKingInnerMinorPressure,
            Option::CentralKingOuterMinorPressure,
            Option::CentralKingReadinessLagWeight,
            Option::CentralKingPressureScale};
}

struct Result
{
    int contribution = 0;
    int generalCentreOpenness = 0;
    int effectiveOpenness = 0;
    int dFileExposure = 0;
    int eFileExposure = 0;
    int heavyLinePressure = 0;
    int bishopDiagonalPressure = 0;
    int directHeavyLines = 0;
    int oneBlockerHeavyLines = 0;
    int multiBlockerHeavyLines = 0;
    int directBishopLines = 0;
    int oneBlockerBishopLines = 0;
    int multiBlockerBishopLines = 0;
    int innerAttackers = 0;
    int outerAttackers = 0;
    int innerAttackContribution = 0;
    int outerAttackContribution = 0;
    int nonlinearEscalation = 0;
    int readinessPressure = 0;
    int castlingMitigation = 0;
    bool centralKingActive = false;
    bool centreLocked = false;
    bool centreOpen = false;
    bool immediateCastling = false;
};

inline int MissingPawnPresences(uint8_t whitePawnFiles, uint8_t blackPawnFiles, int firstFile, int lastFile)
{
    int missing = 0;
    for (int file = firstFile; file <= lastFile; ++file)
    {
        missing += (whitePawnFiles & (1 << file)) ? 0 : 1;
        missing += (blackPawnFiles & (1 << file)) ? 0 : 1;
    }
    return missing;
}

inline int MissingPawnPresences(Board& board, int firstFile, int lastFile)
{
    uint8_t whitePawnFiles = 0;
    uint8_t blackPawnFiles = 0;
    for (int square : board.pieces[1]) whitePawnFiles |= static_cast<uint8_t>(1 << (square % 8));
    for (int square : board.pieces[9]) blackPawnFiles |= static_cast<uint8_t>(1 << (square % 8));
    return MissingPawnPresences(whitePawnFiles, blackPawnFiles, firstFile, lastFile);
}

inline bool IsImmediatelyLegalCastle(Board& board, bool whiteKing, bool kingSide,
                                     const uint64_t* legacyAttacks = nullptr)
{
    const int home = whiteKing ? 0 : 56;
    const int kingSquare = home + 4;
    if (board.pieces[whiteKing ? 6 : 14].front() != kingSquare)
        return false;

    const bool rights = whiteKing
        ? (kingSide ? board.whiteSmallCastle : board.whiteBigCastle)
        : (kingSide ? board.blackSmallCastle : board.blackBigCastle);
    if (!rights)
        return false;

    const int path[3] = {
        home + (kingSide ? 5 : 3),
        home + (kingSide ? 6 : 2),
        home + (kingSide ? 6 : 1)
    };
    const int pathCount = kingSide ? 2 : 3;
    for (int i = 0; i < pathCount; ++i)
        if (board.mainBoard[path[i]] != 0)
            return false;

    const int kingPath[3] = {kingSquare, path[0], path[1]};
    for (int square : kingPath)
        if (legacyAttacks ? (legacyAttacks[whiteKing ? 1 : 0] & Option::PowerTwo[square]) != 0
                          : BoardLogic::UnderAttack(board, square, whiteKing))
            return false;
    return true;
}

inline bool HasCentralPawnBreak(const Board& board, bool white)
{
    if (white)
    {
        // White d-break: d3-d4 or d2-d4
        if (board.mainBoard[19] == 1 && board.mainBoard[27] == 0)
        {
            if (board.mainBoard[34] == 9 || board.mainBoard[36] == 9) return true;
        }
        if (board.mainBoard[11] == 1 && board.mainBoard[19] == 0 && board.mainBoard[27] == 0)
        {
            if (board.mainBoard[34] == 9 || board.mainBoard[36] == 9) return true;
        }
        // White e-break: e3-e4 or e2-e4
        if (board.mainBoard[20] == 1 && board.mainBoard[28] == 0)
        {
            if (board.mainBoard[35] == 9 || board.mainBoard[37] == 9) return true;
        }
        if (board.mainBoard[12] == 1 && board.mainBoard[20] == 0 && board.mainBoard[28] == 0)
        {
            if (board.mainBoard[35] == 9 || board.mainBoard[37] == 9) return true;
        }
    }
    else
    {
        // Black d-break: d6-d5 or d7-d5
        if (board.mainBoard[43] == 9 && board.mainBoard[35] == 0)
        {
            if (board.mainBoard[26] == 1 || board.mainBoard[28] == 1) return true;
        }
        if (board.mainBoard[51] == 9 && board.mainBoard[43] == 0 && board.mainBoard[35] == 0)
        {
            if (board.mainBoard[26] == 1 || board.mainBoard[28] == 1) return true;
        }
        // Black e-break: e6-e5 or e7-e5
        if (board.mainBoard[44] == 9 && board.mainBoard[36] == 0)
        {
            if (board.mainBoard[27] == 1 || board.mainBoard[29] == 1) return true;
        }
        if (board.mainBoard[52] == 9 && board.mainBoard[44] == 0 && board.mainBoard[36] == 0)
        {
            if (board.mainBoard[27] == 1 || board.mainBoard[29] == 1) return true;
        }
    }
    return false;
}

inline bool IsLockedCore(Board& board)
{
    const bool dBlocked = (board.mainBoard[27] == 1 && board.mainBoard[35] == 9);
    const bool eBlocked = (board.mainBoard[28] == 1 && board.mainBoard[36] == 9);
    if (dBlocked && eBlocked)
        return true;
    if (dBlocked)
        return !HasCentralPawnBreak(board, true) && !HasCentralPawnBreak(board, false);
    if (eBlocked)
        return !HasCentralPawnBreak(board, true) && !HasCentralPawnBreak(board, false);
    return false;
}

inline int BlockersBetween(long long ray, long long occupancy, int target)
{
    return __builtin_popcountll(static_cast<unsigned long long>(
        (ray ^ Option::PowerTwo[target]) & occupancy));
}

inline bool MinorAttacksSquare(long long occupancy, int pieceType,
                               int from, int target)
{
    if (pieceType == 2)
        return (AttackPlaces::KnightAttackPlaces[from] & Option::PowerTwo[target]) != 0;
    const long long ray = AttackPlaces::BishopAttack[from][target];
    return ray != 0 && ((ray ^ Option::PowerTwo[target]) & occupancy) == 0;
}

inline bool IsInKingLayer(int square, int kingSquare, int innerRadius,
                          int outerRadius)
{
    const int rankDistance = std::abs(square / 8 - kingSquare / 8);
    const int fileDistance = std::abs(square % 8 - kingSquare % 8);
    const int distance = std::max(rankDistance, fileDistance);
    return distance > innerRadius && distance <= outerRadius;
}

inline bool MinorAttacksLayer(long long occupancy, int pieceType, int from,
                              int kingSquare, int innerRadius, int outerRadius)
{
    for (int target = 0; target < 64; ++target)
        if (IsInKingLayer(target, kingSquare, innerRadius, outerRadius) &&
            MinorAttacksSquare(occupancy, pieceType, from, target))
            return true;
    return false;
}

struct CentralKingLayers {
    long long inner[64]{};
    long long outer[64]{};
    constexpr CentralKingLayers() {
        for (int ksq = 0; ksq < 64; ++ksq) {
            int kr = ksq / 8, kf = ksq % 8;
            for (int sq = 0; sq < 64; ++sq) {
                int r = sq / 8, f = sq % 8;
                int dr = r - kr; if (dr < 0) dr = -dr;
                int df = f - kf; if (df < 0) df = -df;
                int dist = dr > df ? dr : df;
                if (dist <= 1) inner[ksq] |= (1ULL << sq);
                if (dist == 2) outer[ksq] |= (1ULL << sq);
            }
        }
    }
};
static constexpr CentralKingLayers KingLayers{};

inline bool MinorAttacksLayerFast(long long occupancy, int pieceType, int from, long long layerMask)
{
    if (pieceType == 2)
    {
        return (AttackPlaces::KnightAttackPlaces[from] & layerMask) != 0;
    }
    long long targets = AttackPlaces::BishopPseudoAttacks[from] & layerMask;
    while (targets)
    {
        int target = __builtin_ctzll(targets);
        targets &= targets - 1;
        if ((AttackPlaces::BetweenMask[from][target] & occupancy) == 0)
            return true;
    }
    return false;
}

inline Result Evaluate(Board& board, bool whiteKing, uint8_t whitePawnFiles, uint8_t blackPawnFiles,
                       const uint64_t* pieceAttacks = nullptr, const uint64_t* legacyAttacks = nullptr,
                       const Weights* suppliedWeights = nullptr)
{
    const Weights productionWeights = ProductionWeights();
    const Weights& weights = suppliedWeights ? *suppliedWeights : productionWeights;
    Result result;
    result.generalCentreOpenness = MissingPawnPresences(whitePawnFiles, blackPawnFiles, 2, 5);
    result.dFileExposure = MissingPawnPresences(whitePawnFiles, blackPawnFiles, 3, 3);
    result.eFileExposure = MissingPawnPresences(whitePawnFiles, blackPawnFiles, 4, 4);
    result.centreLocked = IsLockedCore(board);
    result.centreOpen = !result.centreLocked &&
        (result.generalCentreOpenness >= 4 ||
         result.dFileExposure + result.eFileExposure >= 2);
    const int kingSquare = board.pieces[whiteKing ? 6 : 14].front();
    const int rank = kingSquare / 8;
    const int file = kingSquare % 8;
    const bool nearHomeRank = whiteKing ? rank <= 1 : rank >= 6;
    result.centralKingActive = nearHomeRank && file >= 3 && file <= 5;
    if (!result.centralKingActive)
        return result;

    const bool attackerHasBreak = HasCentralPawnBreak(board, !whiteKing);
    const int breakPotential = attackerHasBreak ? 3 : 0;
    result.effectiveOpenness = std::min(
        16,
        result.generalCentreOpenness +
        2 * (result.dFileExposure + result.eFileExposure) +
        breakPotential);
    if (result.centreLocked)
        result.effectiveOpenness /= 2;

    result.immediateCastling =
        IsImmediatelyLegalCastle(board, whiteKing, true, legacyAttacks) ||
        IsImmediatelyLegalCastle(board, whiteKing, false, legacyAttacks);

    const bool enemyWhite = !whiteKing;
    const long long occupancy = board.whitePieces | board.blackPieces;
    const bool kingOnCentralFile = (file == 3 || file == 4);
    for (int pieceType : {4, 5})
    {
        const int pieceIndex = pieceType + (enemyWhite ? 0 : 8);
        for (int square : board.pieces[pieceIndex])
        {
            const long long ray = AttackPlaces::RookAttack[square][kingSquare];
            if (ray == 0)
                continue;
            const int blockers = __builtin_popcountll(static_cast<unsigned long long>(
                AttackPlaces::BetweenMask[square][kingSquare] & occupancy));
            const bool centralFile = kingOnCentralFile && (square % 8 == file);
            if (blockers == 0)
            {
                result.directHeavyLines++;
                result.heavyLinePressure += centralFile ? 40 : 30;
            }
            else if (blockers == 1)
            {
                result.oneBlockerHeavyLines++;
                result.heavyLinePressure += centralFile ? 20 : 14;
            }
            else
            {
                result.multiBlockerHeavyLines++;
                result.heavyLinePressure += centralFile ? 6 : 4;
            }
        }
    }

    const int enemyBishopIndex = enemyWhite ? 3 : 11;
    for (int square : board.pieces[enemyBishopIndex])
    {
        const long long ray = AttackPlaces::BishopAttack[square][kingSquare];
        if (ray == 0)
            continue;
        const int blockers = __builtin_popcountll(static_cast<unsigned long long>(
            AttackPlaces::BetweenMask[square][kingSquare] & occupancy));
        if (blockers == 0)
        {
            result.directBishopLines++;
            result.bishopDiagonalPressure += 14;
        }
        else if (blockers == 1)
        {
            result.oneBlockerBishopLines++;
            result.bishopDiagonalPressure += 6;
        }
        else
        {
            result.multiBlockerBishopLines++;
            result.bishopDiagonalPressure += 2;
        }
    }

    const long long innerLayer = KingLayers.inner[kingSquare];
    const long long outerLayer = KingLayers.outer[kingSquare];
    for (int pieceType : {2, 3})
    {
        const int pieceIndex = pieceType + (enemyWhite ? 0 : 8);
        for (int square : board.pieces[pieceIndex])
        {
            if (pieceAttacks ? (pieceAttacks[square] & innerLayer) != 0
                             : MinorAttacksLayerFast(occupancy, pieceType, square, innerLayer))
            {
                const bool contested = legacyAttacks
                    ? (legacyAttacks[whiteKing ? 1 : 0] & Option::PowerTwo[square]) != 0
                    : BoardLogic::UnderAttack(board, square, whiteKing);
                if (!contested)
                    result.innerAttackers++;
                else
                    result.outerAttackers++;
            }
            else if ((outerLayer & Option::PowerTwo[square]) != 0 ||
                     (pieceAttacks ? (pieceAttacks[square] & outerLayer) != 0
                                   : MinorAttacksLayerFast(occupancy, pieceType, square, outerLayer)))
            {
                result.outerAttackers++;
            }
        }
    }
    result.innerAttackContribution = result.innerAttackers * weights.innerMinorPressure;
    result.outerAttackContribution = result.outerAttackers * weights.outerMinorPressure;

    const int attackerCount = result.innerAttackers + result.outerAttackers;
    if ((result.heavyLinePressure > 0 || result.bishopDiagonalPressure > 0) &&
        result.dFileExposure + result.eFileExposure >= 2 && attackerCount >= 2)
        result.nonlinearEscalation = std::min(12, 3 * (attackerCount - 1));

    if (attackerHasBreak && !result.centreLocked && !result.immediateCastling)
    {
        const bool canCastleK = whiteKing ? board.whiteSmallCastle : board.blackSmallCastle;
        const bool canCastleQ = whiteKing ? board.whiteBigCastle : board.blackBigCastle;
        int kObstructions = 0;
        int qObstructions = 0;
        if (canCastleK)
        {
            if (whiteKing)
                kObstructions = (board.mainBoard[5] != 0 ? 1 : 0) + (board.mainBoard[6] != 0 ? 1 : 0);
            else
                kObstructions = (board.mainBoard[61] != 0 ? 1 : 0) + (board.mainBoard[62] != 0 ? 1 : 0);
        }
        if (canCastleQ)
        {
            if (whiteKing)
                qObstructions = (board.mainBoard[1] != 0 ? 1 : 0) + (board.mainBoard[2] != 0 ? 1 : 0) + (board.mainBoard[3] != 0 ? 1 : 0);
            else
                qObstructions = (board.mainBoard[57] != 0 ? 1 : 0) + (board.mainBoard[58] != 0 ? 1 : 0) + (board.mainBoard[59] != 0 ? 1 : 0);
        }
        int evacuationObstruction = 3;
        if (canCastleK && canCastleQ)
            evacuationObstruction = std::min(kObstructions, qObstructions);
        else if (canCastleK)
            evacuationObstruction = kObstructions;
        else if (canCastleQ)
            evacuationObstruction = qObstructions;

        const int defHomeMinors = whiteKing
            ? ((board.mainBoard[1] == 2) + (board.mainBoard[2] == 3) + (board.mainBoard[5] == 3) + (board.mainBoard[6] == 2))
            : ((board.mainBoard[57] == 10) + (board.mainBoard[58] == 11) + (board.mainBoard[61] == 11) + (board.mainBoard[62] == 10));
        const int attHomeMinors = whiteKing
            ? ((board.mainBoard[57] == 10) + (board.mainBoard[58] == 11) + (board.mainBoard[61] == 11) + (board.mainBoard[62] == 10))
            : ((board.mainBoard[1] == 2) + (board.mainBoard[2] == 3) + (board.mainBoard[5] == 3) + (board.mainBoard[6] == 2));
        const int minorLag = std::max(0, defHomeMinors - attHomeMinors);
        const bool queenInCorridor = (board.mainBoard[whiteKing ? 3 : 59] == (whiteKing ? 5 : 13));

        const int readinessLag = evacuationObstruction + minorLag + (queenInCorridor && evacuationObstruction > 0 ? 1 : 0);
        result.readinessPressure = readinessLag * weights.readinessLagWeight;
    }

    const int rawPressure = result.heavyLinePressure +
                            result.bishopDiagonalPressure +
                            result.innerAttackContribution +
                            result.outerAttackContribution +
                            result.nonlinearEscalation +
                            result.readinessPressure;
    int pressure = (rawPressure * result.effectiveOpenness + 8) / 16;
    if (result.immediateCastling)
    {
        const int mitigated = (pressure * 3 + 2) / 4;
        result.castlingMitigation = pressure - mitigated;
        pressure = mitigated;
    }
    result.contribution = -pressure * weights.pressureScale;
    return result;
}

inline Result Evaluate(Board& board, bool whiteKing)
{
    uint8_t whitePawnFiles = 0;
    uint8_t blackPawnFiles = 0;
    for (int square : board.pieces[1]) whitePawnFiles |= static_cast<uint8_t>(1 << (square % 8));
    for (int square : board.pieces[9]) blackPawnFiles |= static_cast<uint8_t>(1 << (square % 8));
    return Evaluate(board, whiteKing, whitePawnFiles, blackPawnFiles);
}

inline Result Evaluate(Board& board, bool whiteKing, const Weights& weights)
{
    uint8_t whitePawnFiles = 0;
    uint8_t blackPawnFiles = 0;
    for (int square : board.pieces[1]) whitePawnFiles |= static_cast<uint8_t>(1 << (square % 8));
    for (int square : board.pieces[9]) blackPawnFiles |= static_cast<uint8_t>(1 << (square % 8));
    return Evaluate(board, whiteKing, whitePawnFiles, blackPawnFiles, nullptr, nullptr, &weights);
}
}

#endif
