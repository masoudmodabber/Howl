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
constexpr int FinalMultiplier = 2;

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
    int castlingMitigation = 0;
    bool centralKingActive = false;
    bool centreLocked = false;
    bool centreOpen = false;
    bool immediateCastling = false;
};

inline int MissingPawnPresences(Board& board, int firstFile, int lastFile)
{
    int missing = 0;
    for (int file = firstFile; file <= lastFile; ++file)
    {
        bool whitePawn = false;
        bool blackPawn = false;
        for (int square : board.pieces[1]) whitePawn |= square % 8 == file;
        for (int square : board.pieces[9]) blackPawn |= square % 8 == file;
        missing += whitePawn ? 0 : 1;
        missing += blackPawn ? 0 : 1;
    }
    return missing;
}

inline bool IsImmediatelyLegalCastle(Board& board, bool whiteKing, bool kingSide)
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
        if (BoardLogic::UnderAttack(board, square, whiteKing))
            return false;
    return true;
}

inline bool IsLockedCore(Board& board)
{
    return (board.mainBoard[27] == 1 && board.mainBoard[35] == 9) ||
           (board.mainBoard[28] == 1 && board.mainBoard[36] == 9);
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

inline Result Evaluate(Board& board, bool whiteKing)
{
    Result result;
    result.generalCentreOpenness = MissingPawnPresences(board, 2, 5);
    result.dFileExposure = MissingPawnPresences(board, 3, 3);
    result.eFileExposure = MissingPawnPresences(board, 4, 4);
    result.centreLocked = IsLockedCore(board);
    result.centreOpen = !result.centreLocked &&
        (result.generalCentreOpenness >= 4 ||
         result.dFileExposure + result.eFileExposure >= 2);
    result.effectiveOpenness = std::min(
        16,
        result.generalCentreOpenness +
        2 * (result.dFileExposure + result.eFileExposure));
    if (result.centreLocked)
        result.effectiveOpenness /= 2;

    const int kingSquare = board.pieces[whiteKing ? 6 : 14].front();
    const int rank = kingSquare / 8;
    const int file = kingSquare % 8;
    const bool nearHomeRank = whiteKing ? rank <= 1 : rank >= 6;
    result.centralKingActive = nearHomeRank && file >= 3 && file <= 5;
    if (!result.centralKingActive)
        return result;

    result.immediateCastling =
        IsImmediatelyLegalCastle(board, whiteKing, true) ||
        IsImmediatelyLegalCastle(board, whiteKing, false);

    const bool enemyWhite = !whiteKing;
    const long long occupancy = board.whitePieces | board.blackPieces;
    for (int pieceType : {4, 5})
    {
        const int pieceIndex = pieceType + (enemyWhite ? 0 : 8);
        for (int square : board.pieces[pieceIndex])
        {
            const long long ray = AttackPlaces::RookAttack[square][kingSquare];
            if (ray == 0)
                continue;
            const int blockers = BlockersBetween(ray, occupancy, kingSquare);
            const bool centralFile = square % 8 == kingSquare % 8 &&
                                     (kingSquare % 8 == 3 || kingSquare % 8 == 4);
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
        const int blockers = BlockersBetween(ray, occupancy, kingSquare);
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

    for (int pieceType : {2, 3})
    {
        const int pieceIndex = pieceType + (enemyWhite ? 0 : 8);
        for (int square : board.pieces[pieceIndex])
        {
            if (MinorAttacksLayer(occupancy, pieceType, square,
                                  kingSquare, -1, 1))
                result.innerAttackers++;
            else if (IsInKingLayer(square, kingSquare, 1, 2) ||
                     MinorAttacksLayer(occupancy, pieceType, square,
                                       kingSquare, 1, 2))
                result.outerAttackers++;
        }
    }
    result.innerAttackContribution = result.innerAttackers * 12;
    result.outerAttackContribution = result.outerAttackers * 6;

    const int attackerCount = result.innerAttackers + result.outerAttackers;
    if ((result.heavyLinePressure > 0 || result.bishopDiagonalPressure > 0) &&
        result.dFileExposure + result.eFileExposure >= 2 && attackerCount >= 2)
        result.nonlinearEscalation = std::min(12, 3 * (attackerCount - 1));

    const int rawPressure = result.heavyLinePressure +
                            result.bishopDiagonalPressure +
                            result.innerAttackContribution +
                            result.outerAttackContribution +
                            result.nonlinearEscalation;
    int pressure = (rawPressure * result.effectiveOpenness + 8) / 16;
    if (result.immediateCastling)
    {
        const int mitigated = (pressure * 3 + 2) / 4;
        result.castlingMitigation = pressure - mitigated;
        pressure = mitigated;
    }
    result.contribution = -pressure * FinalMultiplier;
    return result;
}
}

#endif
