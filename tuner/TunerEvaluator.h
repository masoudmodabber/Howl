#ifndef HOWL_TUNER_EVALUATOR_H
#define HOWL_TUNER_EVALUATOR_H

#include "Board.h"
#include "EvaluationLogic.h"
#include "CentralKingAttackPressure.h"
#include "Option.h"
#include "KingSetup.h"
#include "AttackPlaces.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "tuner/TunerEvaluationState.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>
#include <utility>

namespace Tuner
{

namespace Detail
{

inline int TaperEvaluationValue(int middleGameValue, int endGameValue, int phase)
{
    return (middleGameValue * phase + endGameValue * (24 - phase)) / 24;
}

inline bool IsIsolatedPawn(long long friendlyPawns, int square)
{
    const int file = square % 8;
    long long adjacentFiles = 0;
    if (file > 0)
        adjacentFiles |= static_cast<long long>(0x0101010101010101ULL << (file - 1));
    if (file < 7)
        adjacentFiles |= static_cast<long long>(0x0101010101010101ULL << (file + 1));
    return (friendlyPawns & adjacentFiles) == 0;
}

inline int KnightOutpostValue(const Board& board, int square, bool white, int phase,
                              const TunerEvaluationState& state)
{
    const int rank = square / 8;
    const bool advanced = white ? (rank >= 3 && rank <= 5) : (rank >= 2 && rank <= 4);
    if (!advanced)
        return 0;

    const int file = square % 8;
    const long long ownFile = static_cast<long long>(0x0101010101010101ULL << file);
    const long long challengeMask =
        (white ? PassedPawnSetup::WhitePassedMask[square]
               : PassedPawnSetup::BlackPassedMask[square]) & ~ownFile;
    if ((challengeMask & (white ? board.blackPawns : board.whitePawns)) != 0)
        return 0;

    int value = TaperEvaluationValue(
        state.KnightOutpostMiddleGame, state.KnightOutpostEndGame, phase);
    const long long supportMask = white
        ? AttackPlaces::BlackPawnAttackPlaces[square]
        : AttackPlaces::WhitePawnAttackPlaces[square];
    if ((supportMask & (white ? board.whitePawns : board.blackPawns)) != 0)
    {
        value += TaperEvaluationValue(
            state.KnightSupportedOutpostMiddleGame,
            state.KnightSupportedOutpostEndGame,
            phase);
    }
    static const int fileScale[8] = {25, 60, 90, 100, 100, 90, 60, 25};
    return value * fileScale[file] / 100;
}

inline bool IsKnightOutpostHole(int square, bool white, long long friendlyPawns, long long enemyPawns)
{
    const int rank = square / 8;
    const bool advanced = white ? (rank >= 3 && rank <= 5) : (rank >= 2 && rank <= 4);
    if (!advanced)
        return false;

    const int file = square % 8;
    const long long ownFile = static_cast<long long>(0x0101010101010101ULL << file);
    const long long challengeMask =
        (white ? PassedPawnSetup::WhitePassedMask[square]
               : PassedPawnSetup::BlackPassedMask[square]) & ~ownFile;
    if ((challengeMask & enemyPawns) != 0)
        return false;

    const long long supportMask = white
        ? AttackPlaces::BlackPawnAttackPlaces[square]
        : AttackPlaces::WhitePawnAttackPlaces[square];
    return (supportMask & friendlyPawns) != 0;
}

static const int KnightOutpostHoleFileScale[8] = {25, 60, 90, 100, 100, 90, 60, 25};

inline int TaperGroup1Value(int middleGameValue, int endGameValue, int phase)
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

inline int TaperGroup2Value(int middleGameValue, int endGameValue, int phase)
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

inline int TaperGroup3Value(int middleGameValue, int endGameValue, int phase)
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

inline bool RooksAreConnected(int firstRook, int secondRook, long long occupiedSquares)
{
    return (AttackPlaces::RookAttack[firstRook][secondRook] & occupiedSquares) ==
        Option::PowerTwo[secondRook];
}

inline bool HasConnectedRooks(const MyList& rooks, long long occupiedSquares)
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

inline int RookConnectionValue(MyList (&pieces)[15], long long occupiedSquares)
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

inline int RookBehindPassedPawnValue(Board& board, int phase, const TunerEvaluationState& state)
{
    const long long occupiedSquares = board.whitePieces | board.blackPieces;
    const int bonus = TaperEvaluationValue(
        state.RookBehindPassedPawnMiddleGame,
        state.RookBehindPassedPawnEndGame,
        phase);
    int value = 0;
    for (int pawn : board.pieces[1])
    {
        if ((PassedPawnSetup::WhitePassedMask[pawn] & board.blackPawns) != 0)
            continue;
        for (int rook : board.pieces[4])
            if (rook % 8 == pawn % 8 && rook < pawn && RooksAreConnected(rook, pawn, occupiedSquares)) value += bonus;
        for (int rook : board.pieces[12])
            if (rook % 8 == pawn % 8 && rook < pawn && RooksAreConnected(rook, pawn, occupiedSquares)) value -= bonus;
    }
    for (int pawn : board.pieces[9])
    {
        if ((PassedPawnSetup::BlackPassedMask[pawn] & board.whitePawns) != 0)
            continue;
        for (int rook : board.pieces[12])
            if (rook % 8 == pawn % 8 && rook > pawn && RooksAreConnected(rook, pawn, occupiedSquares)) value -= bonus;
        for (int rook : board.pieces[4])
            if (rook % 8 == pawn % 8 && rook > pawn && RooksAreConnected(rook, pawn, occupiedSquares)) value += bonus;
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

inline bool IsInsideBoard(int rank, int file)
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
    return ((ray ^ Option::PowerTwo[target]) & occupiedSquares) == 0;
}

inline bool PieceAttacksSquare(Board& board, int pieceType, bool white,
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

inline int CountSideAttacks(Board& board, bool white, int target)
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

inline int CountNonKingSideAttacks(Board& board, bool white, int target)
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

inline int UndefendedKingZoneDanger(Board& board, bool whiteKing, int kingSquare)
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
            danger += UndefendedSquareDanger + (enemyAttacks - 1) * AdditionalAttackerDanger;
    }
    return danger;
}

inline std::vector<int> KingZone(int kingSquare)
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

inline bool PieceParticipatesInZone(Board& board, int boardPiece, int square,
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

inline int ShelterDanger(Board& board, bool whiteKing, int kingSquare)
{
    const int direction = whiteKing ? 1 : -1;
    const int kingRank = kingSquare / 8;
    const int kingFile = kingSquare % 8;
    MyList& friendlyPawns = board.pieces[whiteKing ? 1 : 9];
    MyList& enemyPawns = board.pieces[whiteKing ? 9 : 1];
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
        for (int pawn : enemyPawns)
        {
            fileHasEnemyPawn |= pawn % 8 == file;
        }
        if (!secondRankPawn && !fartherPawn && !fileHasEnemyPawn)
        {
            danger += 7;
        }
    }
    return danger;
}

inline int CalculatePhase(const Board& thisBoard)
{
    const auto& pieces = thisBoard.pieces;
    int phase = pieces[2].size() * 1 + pieces[3].size() * 1 + pieces[4].size() * 2 + pieces[5].size() * 4
              + pieces[10].size() * 1 + pieces[11].size() * 1 + pieces[12].size() * 2 + pieces[13].size() * 4;
    return std::clamp(phase, 0, 24);
}

inline KingDangerResult EvaluateKingDanger(Board& board, bool whiteKing)
{
    static constexpr int attackerWeight[7] = {0, 2, 5, 5, 8, 12, 0};
    static constexpr int defenderWeight[7] = {0, 2, 4, 4, 5, 7, 0};
    const int kingSquare = board.pieces[whiteKing ? 6 : 14].front();
    const bool attackingWhite = !whiteKing;
    const PrecomputedKingZone& zone = KingZonesData.zones[kingSquare];
    const long long occupiedSquares = board.whitePieces | board.blackPieces;
    const int attackerFirst = attackingWhite ? 1 : 9;
    const int defenderFirst = whiteKing ? 1 : 9;
    int attackerParticipation = 0;
    int defenderParticipation = 0;
    int attackerCount = 0;
    int loneAttackerSq = -1;
    int defenderCount = 0;

    const long long attackingPawnAttacks = attackingWhite
        ? (((board.whitePawns & ~0x8080808080808080ULL) << 9) | ((board.whitePawns & ~0x0101010101010101ULL) << 7))
        : (((board.blackPawns & ~0x0101010101010101ULL) >> 9) | ((board.blackPawns & ~0x8080808080808080ULL) >> 7));
    long long restrictedBetweenSquares = 0;

    for (int boardPiece = attackerFirst; boardPiece < attackerFirst + 5; boardPiece++)
    {
        const int pieceType = attackingWhite ? boardPiece : boardPiece - 8;
        for (int square : board.pieces[boardPiece])
        {
            if (PieceParticipatesInZoneFast(occupiedSquares, boardPiece, square, zone))
            {
                attackerParticipation += attackerWeight[pieceType];
                attackerCount++;
                loneAttackerSq = square;
                if (pieceType >= 3 && pieceType <= 5)
                {
                    for (int i = 1; i < zone.count; i++)
                    {
                        const int target = zone.squares[i];
                        if (PieceAttacksSquareFast(occupiedSquares, pieceType, attackingWhite, square, target))
                        {
                            restrictedBetweenSquares |= (AttackPlaces::BetweenMask[square][target] & attackingPawnAttacks);
                        }
                    }
                }
            }
        }
    }
    for (int boardPiece = defenderFirst; boardPiece < defenderFirst + 5; boardPiece++)
    {
        const int pieceType = whiteKing ? boardPiece : boardPiece - 8;
        for (int square : board.pieces[boardPiece])
        {
            if (PieceParticipatesInZoneFast(occupiedSquares, boardPiece, square, zone))
            {
                defenderParticipation += defenderWeight[pieceType];
                defenderCount++;
            }
        }
    }
    const int kingFile = kingSquare % 8;
    int safeEscapes = 0;
    int controlledEscapes = 0;
    int occupiedEscapes = 0;
    const int edgeDirections = 9 - zone.count;
    for (int i = 1; i < zone.count; i++)
    {
        const int target = zone.squares[i];
        const int occupant = board.mainBoard[target];
        const bool occupiedByDefender = occupant != 0 && (occupant < 8) == whiteKing;
        if (occupiedByDefender)
        {
            occupiedEscapes++;
        }
        else if (HasSideAttack(board, attackingWhite, target, occupiedSquares))
        {
            controlledEscapes++;
        }
        else
        {
            safeEscapes++;
        }
    }
    int filePressure = 0;
    for (int file = std::max(0, kingFile - 1); file <= std::min(7, kingFile + 1); file++)
    {
        bool friendlyPawn = false;
        bool enemyPawn = false;
        for (int pawn : board.pieces[whiteKing ? 1 : 9]) friendlyPawn |= pawn % 8 == file;
        for (int pawn : board.pieces[whiteKing ? 9 : 1]) enemyPawn |= pawn % 8 == file;
        const int openness = !friendlyPawn ? (!enemyPawn ? 2 : 1) : 0;
        if (openness == 0)
        {
            continue;
        }
        for (int pieceType : {4, 5})
        {
            const int boardPiece = attackingWhite ? pieceType : pieceType + 8;
            for (int square : board.pieces[boardPiece])
            {
                for (int i = 0; i < zone.count; i++)
                {
                    const int target = zone.squares[i];
                    if (target % 8 == file && PieceAttacksSquareFast(occupiedSquares, pieceType, attackingWhite, square, target))
                    {
                        filePressure += openness == 2 ? 10 : 6;
                        break;
                    }
                }
            }
        }
    }
    int diagonalPressure = 0;
    for (int pieceType : {3, 5})
    {
        const int boardPiece = attackingWhite ? pieceType : pieceType + 8;
        for (int square : board.pieces[boardPiece])
        {
            if (PieceAttacksSquareFast(occupiedSquares, pieceType, attackingWhite, square, kingSquare))
            {
                diagonalPressure += 9;
            }
        }
    }
    bool hasHeavyMatingBattery = false;
    const int phaseVal = Detail::CalculatePhase(board);
    if (phaseVal >= 12 && attackerCount >= 2)
    {
        for (int i = 1; i < zone.count; i++)
        {
            const int target = zone.squares[i];
            bool defended = false;
            for (int dPiece = defenderFirst; dPiece < defenderFirst + 5; dPiece++)
            {
                const int pType = whiteKing ? dPiece : dPiece - 8;
                for (int dSq : board.pieces[dPiece])
                {
                    if (PieceAttacksSquareFast(occupiedSquares, pType, whiteKing, dSq, target))
                    {
                        defended = true; break;
                    }
                }
                if (defended) break;
            }
            if (!defended)
            {
                bool hasQ = false, hasR = false;
                for (int qSq : board.pieces[attackingWhite ? 5 : 13])
                {
                    if (PieceAttacksSquareFast(occupiedSquares, 5, attackingWhite, qSq, target)) { hasQ = true; break; }
                }
                for (int rSq : board.pieces[attackingWhite ? 4 : 12])
                {
                    if (PieceAttacksSquareFast(occupiedSquares, 4, attackingWhite, rSq, target)) { hasR = true; break; }
                }
                if (hasQ && hasR)
                {
                    hasHeavyMatingBattery = true;
                    break;
                }
            }
        }
    }

    const int escapeDanger = controlledEscapes * 6 + occupiedEscapes * 2 +
                             edgeDirections * 2 + std::max(0, 3 - safeEscapes) * 8;
    const int balanceDanger = std::max(0, attackerParticipation - defenderParticipation) +
                              std::max(0, attackerCount - defenderCount) * 4;
    const int shelterDanger = ShelterDanger(board, whiteKing, kingSquare);
    const int lineDanger = filePressure + diagonalPressure;
    const int undefendedKingZoneDanger = UndefendedKingZoneDanger(board, whiteKing, kingSquare);
    const int defensiveRestriction = __builtin_popcountll(restrictedBetweenSquares) * 6;
    int rawDanger = attackerParticipation * 2 + escapeDanger +
                    lineDanger + shelterDanger + balanceDanger + undefendedKingZoneDanger +
                    defensiveRestriction + (hasHeavyMatingBattery ? 140 : 0);

    const int queenCount = board.pieces[attackingWhite ? 5 : 13].size();
    const int rookCount = board.pieces[attackingWhite ? 4 : 12].size();
    const int minorCount = board.pieces[attackingWhite ? 2 : 10].size() +
                           board.pieces[attackingWhite ? 3 : 11].size();
    const int phase = Detail::CalculatePhase(board);
    const int base = (queenCount > 0) ? 20 : (20 * phase / 24);
    int attackingMaterialScale = std::min(100, base + queenCount * 45 +
                                                     rookCount * 12 + minorCount * 5);
    if (queenCount == 0)
    {
        attackingMaterialScale = attackingMaterialScale * phase / 24;
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
            const bool attackerContested = (loneAttackerSq >= 0 && BoardLogic::UnderAttack(board, loneAttackerSq, !whiteKing));
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


inline int ChebyshevDistance(int sq1, int sq2)
{
    return std::max(std::abs((sq1 % 8) - (sq2 % 8)), std::abs((sq1 / 8) - (sq2 / 8)));
}

inline int EvaluatePassedPawnKingRace(Board &board, int whiteKingSq, int blackKingSq)
{
    // If either side has queens on the board, direct king race is unrealistic.
    if (board.pieces[5].size() > 0 || board.pieces[13].size() > 0)
    {
        return 0;
    }

    int phase = Detail::CalculatePhase(board);
    // King distance influence decreases when remaining material is high.
    if (phase >= 12)
    {
        return 0;
    }

    static const int rankWeight[8] = {0, 0, 0, 1, 5, 8, 14, 22};
    int whiteRaceTempo = (!board.sideToMove) ? 1 : -1;
    int blackRaceTempo = (board.sideToMove) ? 1 : -1;

    int whiteAdjustment = 0;
    std::vector<int> whitePassers;
    for (int pawnPlace : board.pieces[1])
    {
        if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & board.blackPawns) == 0)
        {
            whitePassers.push_back(pawnPlace);
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
    }

    int blackAdjustment = 0;
    std::vector<int> blackPassers;
    for (int pawnPlace : board.pieces[9])
    {
        if ((PassedPawnSetup::BlackPassedMask[pawnPlace] & board.whitePawns) == 0)
        {
            blackPassers.push_back(pawnPlace);
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
    }

    int netRace = whiteAdjustment - blackAdjustment;
    int raceValue = (netRace * (12 - phase)) / 12;

    // King blockade of enemy passed pawns
    int whiteBlockade = 0;
    for (int pawnPlace : blackPassers)
    {
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
    for (int pawnPlace : whitePassers)
    {
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
    for (int sq : whitePassers) {
        int d = 7 - (sq / 8);
        if (d <= 2) whiteAdv++;
    }
    for (int sq : blackPassers) {
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
        for (int sq : whitePassers) {
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
        for (int sq : blackPassers) {
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
        for (int sq : whitePassers) {
            int d = 7 - (sq / 8);
            int f = sq % 8;
            if (d == 3 && (f <= 1 || f >= 5)) whiteD3Outside++;
        }
        for (int sq : blackPassers) {
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
    for (int sq : blackPassers) {
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
    for (int sq : whitePassers) {
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

static int KnightDistance[64][64];
static bool knightDistanceInitialized = false;

inline void InitializeKnightDistance()
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

inline int EvaluatePassedPawnMinorAccessibility(Board &board)
{
    InitializeKnightDistance();

    static const int rankScalePercent[8] = {0, 0, 0, 25, 50, 75, 100, 100};

    auto controlledByPasserKingOrBishop = [&](int square, bool passerIsWhite) -> bool
    {
        const int sideOffset = passerIsWhite ? 0 : 8;
        const int kingSquare = board.pieces[sideOffset + 6].front();
        if ((AttackPlaces::KingAttackPlaces[kingSquare] & Option::PowerTwo[square]) != 0)
            return true;

        const long long occupiedWithTarget =
            (board.whitePieces | board.blackPieces) | Option::PowerTwo[square];
        for (int bishopSquare : board.pieces[sideOffset + 3])
        {
            if ((AttackPlaces::BishopAttack[bishopSquare][square] & occupiedWithTarget) ==
                Option::PowerTwo[square])
                return true;
        }
        return false;
    };

    auto knightCorridorValue = [&](int knightSq, bool knightIsWhite,
                                   int pawnPlace, bool passerIsWhite) -> int
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
                if (!controlledByPasserKingOrBishop(square, passerIsWhite))
                    hasMaintainableDestination = true;

                int memo[64];
                std::fill(std::begin(memo), std::end(memo), -1);
                std::function<int(int)> minimumControlledLandings = [&](int from) -> int
                {
                    if (from == square)
                        return 0;
                    int &cached = memo[from];
                    if (cached >= 0)
                        return cached;
                    cached = 64;
                    const int remainingDistance = KnightDistance[from][square];
                    const long long hops = AttackPlaces::KnightAttackPlaces[from];
                    for (int hop = 0; hop < 64; hop++)
                    {
                        if ((hops & Option::PowerTwo[hop]) == 0 ||
                            KnightDistance[hop][square] != remainingDistance - 1)
                            continue;
                        const int controlledLanding =
                            controlledByPasserKingOrBishop(hop, passerIsWhite) ? 1 : 0;
                        cached = std::min(cached,
                            controlledLanding + minimumControlledLandings(hop));
                    }
                    return cached;
                };
                const int controlledLandings = minimumControlledLandings(knightSq);
                value = value * std::max(1, 4 - controlledLandings) / 4;
            }
            bestValue = std::max(bestValue, value);
        }
        if (hasTimelyUsefulDestination && !hasMaintainableDestination)
            return 0;
        return bestValue;
    };

    int whiteNet = 0;
    for (int pawnPlace : board.pieces[1])
    {
        if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & board.blackPawns) == 0)
        {
            int accessibility = 0;
            for (int kSq : board.pieces[2])
            {
                accessibility += knightCorridorValue(kSq, true, pawnPlace, true);
            }
            for (int kSq : board.pieces[10])
            {
                accessibility -= knightCorridorValue(kSq, false, pawnPlace, true);
            }

            int relRank = (pawnPlace / 8) + 1;
            whiteNet += (accessibility * rankScalePercent[relRank]) / 100;
        }
    }

    int blackNet = 0;
    for (int pawnPlace : board.pieces[9])
    {
        if ((PassedPawnSetup::BlackPassedMask[pawnPlace] & board.whitePawns) == 0)
        {
            int accessibility = 0;
            for (int kSq : board.pieces[10])
            {
                accessibility += knightCorridorValue(kSq, false, pawnPlace, false);
            }
            for (int kSq : board.pieces[2])
            {
                accessibility -= knightCorridorValue(kSq, true, pawnPlace, false);
            }

            int relRank = 8 - (pawnPlace / 8);
            blackNet += (accessibility * rankScalePercent[relRank]) / 100;
        }
    }

    return whiteNet - blackNet;
}

inline bool IsSquareAttackedBySide(Board &board, int sq, bool byWhite)
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

inline int CountSquareAttacksBySide(Board &board, int sq, bool byWhite, bool friendlySliderBehindFile, int pawnFile)
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

inline int EvaluatePassedPawnCorridorSafety(Board &board)
{
    static const int rankScalePercent[8] = {0, 0, 0, 25, 50, 75, 100, 100};

    int whiteTotal = 0;
    for (int pawnPlace : board.pieces[1])
    {
        if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & board.blackPawns) == 0)
        {
            int pFile = pawnPlace % 8;
            int pRank = pawnPlace / 8;

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
            whiteTotal += scaled;
        }
    }

    int blackTotal = 0;
    for (int pawnPlace : board.pieces[9])
    {
        if ((PassedPawnSetup::BlackPassedMask[pawnPlace] & board.whitePawns) == 0)
        {
            int pFile = pawnPlace % 8;
            int pRank = pawnPlace / 8;

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
            blackTotal += scaled;
        }
    }

    return whiteTotal - blackTotal;
}

inline int PieceMoveCount(Board& thisBoard, int phase, const TunerEvaluationState& state)
{
    long long whitePieces = thisBoard.whitePieces;
    long long blackPieces = thisBoard.blackPieces;
    int* mainBoard = thisBoard.mainBoard;
    long long wholeBoard = whitePieces | blackPieces;
    int movement = 0;
    int whiteAttackValue = 0;
    int blackAttackValue = 0;
    int whiteRookFileBonus = 0;
    int blackRookFileBonus = 0;
    int moveCount;
    const long long bpa = ((thisBoard.blackPawns & ~0x0101010101010101ULL) >> 9) |
                          ((thisBoard.blackPawns & ~0x8080808080808080ULL) >> 7);
    const long long wpa = ((thisBoard.whitePawns & ~0x8080808080808080ULL) << 9) |
                          ((thisBoard.whitePawns & ~0x0101010101010101ULL) << 7);

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

    // White pieces (1..6)
    for (int piece = 1; piece < 7; piece++)
    {
        switch (piece)
        {
        case 1:
            for (int piecePoisiion : thisBoard.pieces[piece])
            {
                moveCount = 0;
                movement += taperedTable(state.PawnInValueWhite, piecePoisiion);
                if (PieceMoves::WhitePawnMoves[piecePoisiion][0] != nullptr)
                {
                    if ((PieceMoves::pawnTwoMove[piecePoisiion] & wholeBoard) == 0)
                    {
                        moveCount++;
                    }
                }
                if (PieceMoves::WhitePawnMoves[piecePoisiion][1] != nullptr && (Option::PowerTwo[piecePoisiion + 8] & wholeBoard) == 0)
                {
                    moveCount++;
                }
                if (PieceMoves::WhitePawnMoves[piecePoisiion][6] != nullptr && piecePoisiion + 7 == thisBoard.unpassentPlace)
                {
                    whiteAttackValue += taperedGroup1Table(state.PawnAttackValue, 9);
                }
                if (PieceMoves::WhitePawnMoves[piecePoisiion][7] != nullptr && piecePoisiion + 9 == thisBoard.unpassentPlace)
                {
                    whiteAttackValue += taperedGroup1Table(state.PawnAttackValue, 9);
                }
                if (PieceMoves::WhitePawnMoves[piecePoisiion][8] != nullptr && (Option::PowerTwo[piecePoisiion + 7] & blackPieces) != 0)
                {
                    whiteAttackValue += taperedGroup1Table(state.PawnAttackValue, mainBoard[piecePoisiion + 7]);
                }
                if (PieceMoves::WhitePawnMoves[piecePoisiion][13] != nullptr && (Option::PowerTwo[piecePoisiion + 9] & blackPieces) != 0)
                {
                    whiteAttackValue += taperedGroup1Table(state.PawnAttackValue, mainBoard[piecePoisiion + 9]);
                }
                movement += taperedTable(state.PawnMoveCountValue, moveCount);
            }
            break;
        case 2:
        {
            static const int knightOffsets[8] = {17, 10, 15, 6, -10, -17, -15, -6};
            static const int knightDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
            for (int piecePoisiion : thisBoard.pieces[piece])
            {
                moveCount = 0;
                movement += taperedTable(state.KnightInValueWhite, piecePoisiion);
                if (phase >= 16 && (piecePoisiion % 8 == 0 || piecePoisiion % 8 == 7)) movement -= 15;
                movement += KnightOutpostValue(thisBoard, piecePoisiion, true, phase, state);
                for (int i = 0; i < 8; ++i)
                {
                    int endPlace = piecePoisiion + knightOffsets[i];
                    int dir = knightDirs[i];
                    if (PieceMoves::KnightMoves[piecePoisiion][dir] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if ((Option::PowerTwo[endPlace] & bpa) == 0)
                                moveCount++;
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            whiteAttackValue += taperedGroup1Table(state.KnightAttackValue, mainBoard[endPlace]);
                            if (mainBoard[endPlace] == 9 && !BoardLogic::UnderAttack(thisBoard, endPlace, true))
                            {
                                int f = endPlace % 8, r = endPlace / 8;
                                bool isCentral = (f >= 2 && f <= 5 && r >= 2 && r <= 5);
                                whiteAttackValue += isCentral ? 32 : 16;
                            }
                        }
                    }
                }
                movement += taperedGroup1Table(state.KnightMoveCountValue, moveCount);
            }
            break;
        }
        case 3:
            for (int piecePoisiion : thisBoard.pieces[piece])
            {
                moveCount = 0;
                movement += taperedTable(state.BishopInValueWhite, piecePoisiion);
                for (int direction = 0; direction <= 6; direction += 2)
                {
                    for (size_t counter = 0; counter < PieceMoves::BishopMoves[piecePoisiion][direction].size(); counter++)
                    {
                        int endPos = PieceMoves::BishopMoves[piecePoisiion][direction][counter]->endPlace;
                        int endPiece = mainBoard[endPos];
                        if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                        {
                            if ((Option::PowerTwo[endPos] & bpa) == 0)
                                moveCount++;
                        }
                        else if ((Option::PowerTwo[endPos] & blackPieces) != 0)
                        {
                            whiteAttackValue += taperedGroup1Table(state.BishopAttackValue, endPiece);
                            if (endPiece == 9 && !BoardLogic::UnderAttack(thisBoard, endPos, true))
                            {
                                int f = endPos % 8, r = endPos / 8;
                                bool isCentral = (f >= 2 && f <= 5 && r >= 2 && r <= 5);
                                whiteAttackValue += isCentral ? 32 : 16;
                            }
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                movement += taperedGroup1Table(state.BishopMoveCountValue, moveCount);
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
                        whiteRookFileBonus += TaperGroup2Value(state.RookOpenFileMiddleGame, state.RookOpenFileEndGame, phase);
                    }
                    else
                    {
                        whiteRookFileBonus += TaperGroup2Value(state.RookSemiOpenFileMiddleGame, state.RookSemiOpenFileEndGame, phase);
                        int targetSq = -1;
                        for (int r = 1; r < 7; ++r) {
                            if (mainBoard[r * 8 + file] == 9) { targetSq = r * 8 + file; break; }
                        }
                        if (targetSq != -1) {
                            bool hasNeighborBehind = false;
                            int targetRank = targetSq / 8;
                            for (int adjF : {file - 1, file + 1}) {
                                if (adjF >= 0 && adjF < 8) {
                                    for (int r = targetRank; r < 7; ++r) {
                                        if (mainBoard[r * 8 + adjF] == 9) { hasNeighborBehind = true; break; }
                                    }
                                }
                            }
                            if (!hasNeighborBehind) {
                                whiteRookFileBonus += 25;
                            }
                        }
                    }
                }

                moveCount = 0;
                movement += taperedGroup2Table(state.RookInValueWhite, piecePoisiion);
                for (int direction = 0; direction <= 6; direction += 2)
                {
                    for (size_t counter = 0; counter < PieceMoves::RookMoves[piecePoisiion][direction].size(); counter++)
                    {
                        int endPos = PieceMoves::RookMoves[piecePoisiion][direction][counter]->endPlace;
                        int endPiece = mainBoard[endPos];
                        if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                        {
                            moveCount++;
                        }
                        else if ((Option::PowerTwo[endPos] & blackPieces) != 0)
                        {
                            whiteAttackValue += taperedGroup2Table(state.RookAttackValue, endPiece);
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                movement += taperedGroup2Table(state.RookMoveCountValue, moveCount);
            }
            break;
        case 5:
            for (int piecePoisiion : thisBoard.pieces[piece])
            {
                moveCount = 0;
                movement += taperedGroup2Table(state.QueenInValueWhite, piecePoisiion);
                int qFile = piecePoisiion % 8;
                unsigned long long qFileMask = 0x0101010101010101ULL << qFile;
                if ((thisBoard.whitePawns & qFileMask) == 0 && (thisBoard.blackPawns & qFileMask) != 0)
                {
                    int targetSq = -1;
                    for (int r = 1; r < 7; ++r) {
                        if (mainBoard[r * 8 + qFile] == 9) { targetSq = r * 8 + qFile; break; }
                    }
                    if (targetSq != -1) {
                        bool hasNeighborBehind = false;
                        int targetRank = targetSq / 8;
                        for (int adjF : {qFile - 1, qFile + 1}) {
                            if (adjF >= 0 && adjF < 8) {
                                for (int r = targetRank; r < 7; ++r) {
                                    if (mainBoard[r * 8 + adjF] == 9) { hasNeighborBehind = true; break; }
                                }
                            }
                        }
                        if (!hasNeighborBehind) {
                            movement += 20;
                            for (int rsq : thisBoard.pieces[4]) {
                                if (rsq % 8 == qFile) { movement += 15; break; }
                            }
                        }
                    }
                }
                for (int direction = 0; direction <= 14; direction += 2)
                {
                    for (size_t counter = 0; counter < PieceMoves::QueenMoves[piecePoisiion][direction].size(); counter++)
                    {
                        int endPos = PieceMoves::QueenMoves[piecePoisiion][direction][counter]->endPlace;
                        int endPiece = mainBoard[endPos];
                        if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                        {
                            moveCount++;
                        }
                        else if ((Option::PowerTwo[endPos] & blackPieces) != 0)
                        {
                            whiteAttackValue += taperedGroup1Table(state.QueenAttackValue, endPiece);
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                movement += taperedGroup1Table(state.QueenMoveCountValue, moveCount);
            }
            break;
        case 6:
            static const int kingOffsets[8] = {7, 8, 9, 1, -7, -8, -9, -1};
            static const int kingDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
            for (int piecePoisiion : thisBoard.pieces[piece])
            {
                moveCount = 0;
                movement += taperedTable(state.KingInValueWhite, piecePoisiion);
                for (int i = 0; i < 8; ++i)
                {
                    int endPlace = piecePoisiion + kingOffsets[i];
                    int dir = kingDirs[i];
                    if (PieceMoves::WhiteKingMoves[piecePoisiion][dir] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            moveCount++;
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            whiteAttackValue += taperedTable(state.KingAttackValue, mainBoard[endPlace]);
                        }
                    }
                }
                movement += taperedTable(state.KingMoveCountValue, moveCount);
            }
            break;
        }
    }

    // Black pieces (9..14)
    for (int piece = 9; piece < 15; piece++)
    {
        switch (piece)
        {
        case 9:
            for (int piecePoisiion : thisBoard.pieces[piece])
            {
                moveCount = 0;
                movement -= taperedTable(state.PawnInValueBlack, piecePoisiion);

                if (PieceMoves::BlackPawnMoves[piecePoisiion][0] != nullptr)
                {
                    if ((PieceMoves::pawnTwoMove[piecePoisiion] & wholeBoard) == 0)
                    {
                        moveCount++;
                    }
                }
                if (PieceMoves::BlackPawnMoves[piecePoisiion][1] != nullptr && (Option::PowerTwo[piecePoisiion - 8] & wholeBoard) == 0)
                {
                    moveCount++;
                }
                if (PieceMoves::BlackPawnMoves[piecePoisiion][6] != nullptr && piecePoisiion - 7 == thisBoard.unpassentPlace)
                {
                    blackAttackValue += taperedGroup1Table(state.PawnAttackValue, 1);
                }
                if (PieceMoves::BlackPawnMoves[piecePoisiion][7] != nullptr && piecePoisiion - 9 == thisBoard.unpassentPlace)
                {
                    blackAttackValue += taperedGroup1Table(state.PawnAttackValue, 1);
                }
                if (PieceMoves::BlackPawnMoves[piecePoisiion][8] != nullptr && (Option::PowerTwo[piecePoisiion - 7] & whitePieces) != 0)
                {
                    blackAttackValue += taperedGroup1Table(state.PawnAttackValue, mainBoard[piecePoisiion - 7]);
                }
                if (PieceMoves::BlackPawnMoves[piecePoisiion][13] != nullptr && (Option::PowerTwo[piecePoisiion - 9] & whitePieces) != 0)
                {
                    blackAttackValue += taperedGroup1Table(state.PawnAttackValue, mainBoard[piecePoisiion - 9]);
                }
                movement -= taperedTable(state.PawnMoveCountValue, moveCount);
            }
            break;
        case 10:
        {
            static const int knightOffsets[8] = {17, 10, 15, 6, -10, -17, -15, -6};
            static const int knightDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
            for (int piecePoisiion : thisBoard.pieces[piece])
            {
                moveCount = 0;
                movement -= taperedTable(state.KnightInValueBlack, piecePoisiion);
                if (phase >= 16 && (piecePoisiion % 8 == 0 || piecePoisiion % 8 == 7)) movement += 15;
                movement -= KnightOutpostValue(thisBoard, piecePoisiion, false, phase, state);
                for (int i = 0; i < 8; ++i)
                {
                    int endPlace = piecePoisiion + knightOffsets[i];
                    int dir = knightDirs[i];
                    if (PieceMoves::KnightMoves[piecePoisiion][dir] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if ((Option::PowerTwo[endPlace] & wpa) == 0)
                                moveCount++;
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            blackAttackValue += taperedGroup1Table(state.KnightAttackValue, mainBoard[endPlace]);
                            if (mainBoard[endPlace] == 1 && !BoardLogic::UnderAttack(thisBoard, endPlace, false))
                            {
                                int f = endPlace % 8, r = endPlace / 8;
                                bool isCentral = (f >= 2 && f <= 5 && r >= 2 && r <= 5);
                                blackAttackValue += isCentral ? 32 : 16;
                            }
                        }
                    }
                }
                movement -= taperedGroup1Table(state.KnightMoveCountValue, moveCount);
            }
            break;
        }
        case 11:
            for (int piecePoisiion : thisBoard.pieces[piece])
            {
                moveCount = 0;
                movement -= taperedTable(state.BishopInValueBlack, piecePoisiion);

                for (int direction = 0; direction <= 6; direction += 2)
                {
                    for (size_t counter = 0; counter < PieceMoves::BishopMoves[piecePoisiion][direction].size(); counter++)
                    {
                        int endPos = PieceMoves::BishopMoves[piecePoisiion][direction][counter]->endPlace;
                        int endPiece = mainBoard[endPos];
                        if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                        {
                            if ((Option::PowerTwo[endPos] & wpa) == 0)
                                moveCount++;
                        }
                        else if ((Option::PowerTwo[endPos] & whitePieces) != 0)
                        {
                            blackAttackValue += taperedGroup1Table(state.BishopAttackValue, endPiece);
                            if (endPiece == 1 && !BoardLogic::UnderAttack(thisBoard, endPos, false))
                            {
                                int f = endPos % 8, r = endPos / 8;
                                bool isCentral = (f >= 2 && f <= 5 && r >= 2 && r <= 5);
                                blackAttackValue += isCentral ? 32 : 16;
                            }
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                movement -= taperedGroup1Table(state.BishopMoveCountValue, moveCount);
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
                        blackRookFileBonus += TaperGroup2Value(state.RookOpenFileMiddleGame, state.RookOpenFileEndGame, phase);
                    }
                    else
                    {
                        blackRookFileBonus += TaperGroup2Value(state.RookSemiOpenFileMiddleGame, state.RookSemiOpenFileEndGame, phase);
                        int targetSq = -1;
                        for (int r = 6; r >= 1; --r) {
                            if (mainBoard[r * 8 + file] == 1) { targetSq = r * 8 + file; break; }
                        }
                        if (targetSq != -1) {
                            bool hasNeighborBehind = false;
                            int targetRank = targetSq / 8;
                            for (int adjF : {file - 1, file + 1}) {
                                if (adjF >= 0 && adjF < 8) {
                                    for (int r = targetRank; r >= 1; --r) {
                                        if (mainBoard[r * 8 + adjF] == 1) { hasNeighborBehind = true; break; }
                                    }
                                }
                            }
                            if (!hasNeighborBehind) {
                                blackRookFileBonus += 25;
                            }
                        }
                    }
                }

                moveCount = 0;
                movement -= taperedGroup2Table(state.RookInValueBlack, piecePoisiion);

                for (int direction = 0; direction <= 6; direction += 2)
                {
                    for (size_t counter = 0; counter < PieceMoves::RookMoves[piecePoisiion][direction].size(); counter++)
                    {
                        int endPos = PieceMoves::RookMoves[piecePoisiion][direction][counter]->endPlace;
                        int endPiece = mainBoard[endPos];
                        if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                        {
                            moveCount++;
                        }
                        else if ((Option::PowerTwo[endPos] & whitePieces) != 0)
                        {
                            blackAttackValue += taperedGroup2Table(state.RookAttackValue, endPiece);
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                movement -= taperedGroup2Table(state.RookMoveCountValue, moveCount);
            }
            break;
        case 13:
            for (int piecePoisiion : thisBoard.pieces[piece])
            {
                moveCount = 0;
                movement -= taperedGroup2Table(state.QueenInValueBlack, piecePoisiion);
                int qFile = piecePoisiion % 8;
                unsigned long long qFileMask = 0x0101010101010101ULL << qFile;
                if ((thisBoard.blackPawns & qFileMask) == 0 && (thisBoard.whitePawns & qFileMask) != 0)
                {
                    int targetSq = -1;
                    for (int r = 6; r >= 1; --r) {
                        if (mainBoard[r * 8 + qFile] == 1) { targetSq = r * 8 + qFile; break; }
                    }
                    if (targetSq != -1) {
                        bool hasNeighborBehind = false;
                        int targetRank = targetSq / 8;
                        for (int adjF : {qFile - 1, qFile + 1}) {
                            if (adjF >= 0 && adjF < 8) {
                                for (int r = targetRank; r >= 1; --r) {
                                    if (mainBoard[r * 8 + adjF] == 1) { hasNeighborBehind = true; break; }
                                }
                            }
                        }
                        if (!hasNeighborBehind) {
                            movement -= 20;
                            for (int rsq : thisBoard.pieces[12]) {
                                if (rsq % 8 == qFile) { movement -= 15; break; }
                            }
                        }
                    }
                }

                for (int direction = 0; direction <= 14; direction += 2)
                {
                    for (size_t counter = 0; counter < PieceMoves::QueenMoves[piecePoisiion][direction].size(); counter++)
                    {
                        int endPos = PieceMoves::QueenMoves[piecePoisiion][direction][counter]->endPlace;
                        int endPiece = mainBoard[endPos];
                        if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                        {
                            moveCount++;
                        }
                        else if ((Option::PowerTwo[endPos] & whitePieces) != 0)
                        {
                            blackAttackValue += taperedGroup1Table(state.QueenAttackValue, endPiece);
                            break;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                movement -= taperedGroup1Table(state.QueenMoveCountValue, moveCount);
            }
            break;
        case 14:
            static const int kingOffsets[8] = {7, 8, 9, 1, -7, -8, -9, -1};
            static const int kingDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
            for (int piecePoisiion : thisBoard.pieces[piece])
            {
                moveCount = 0;
                movement -= taperedTable(state.KingInValueBlack, piecePoisiion);
                for (int i = 0; i < 8; ++i)
                {
                    int endPlace = piecePoisiion + kingOffsets[i];
                    int dir = kingDirs[i];
                    if (PieceMoves::BlackKingMoves[piecePoisiion][dir] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            moveCount++;
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            blackAttackValue += taperedTable(state.KingAttackValue, mainBoard[endPlace]);
                        }
                    }
                }
                movement -= taperedTable(state.KingMoveCountValue, moveCount);
            }
            break;
        }
    }

    int scaledAttackNet = ((whiteAttackValue - blackAttackValue) * state.PieceAttackScalePercent) / 100;
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
        int r = psq / 8, c = psq % 8;
        int kr = whiteKingR, kc = whiteKingC;
        int dr = kr - r, dc = kc - c;
        if (dr == 0 || dc == 0 || std::abs(dr) == std::abs(dc)) {
            int stepR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
            int stepC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
            bool clearToKing = true;
            for (int cr = r + stepR, cc = c + stepC; cr != kr || cc != kc; cr += stepR, cc += stepC) {
                if (mainBoard[cr * 8 + cc] != 0) { clearToKing = false; break; }
            }
            if (clearToKing) {
                for (int cr = r - stepR, cc = c - stepC; cr >= 0 && cr < 8 && cc >= 0 && cc < 8; cr -= stepR, cc -= stepC) {
                    int p = mainBoard[cr * 8 + cc];
                    if (p != 0) {
                        bool isSlider = (stepR == 0 || stepC == 0) ? (p == 12 || p == 13) : (p == 11 || p == 13);
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
        }
    }
    const int blackKingSq = thisBoard.pieces[14].front();
    const int blackKingR = blackKingSq / 8, blackKingC = blackKingSq % 8;
    const bool blackCentralKing = (blackKingR >= 6 && blackKingC >= 2 && blackKingC <= 5);
    for (int psq : thisBoard.pieces[9]) {
        int r = psq / 8, c = psq % 8;
        int kr = blackKingR, kc = blackKingC;
        int dr = kr - r, dc = kc - c;
        if (dr == 0 || dc == 0 || std::abs(dr) == std::abs(dc)) {
            int stepR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
            int stepC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
            bool clearToKing = true;
            for (int cr = r + stepR, cc = c + stepC; cr != kr || cc != kc; cr += stepR, cc += stepC) {
                if (mainBoard[cr * 8 + cc] != 0) { clearToKing = false; break; }
            }
            if (clearToKing) {
                for (int cr = r - stepR, cc = c - stepC; cr >= 0 && cr < 8 && cc >= 0 && cc < 8; cr -= stepR, cc -= stepC) {
                    int p = mainBoard[cr * 8 + cc];
                    if (p != 0) {
                        bool isSlider = (stepR == 0 || stepC == 0) ? (p == 4 || p == 5) : (p == 3 || p == 5);
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
        if (thisBoard.mainBoard[sq] == 1) {
            bool attackedByQ = false;
            for (int qSq : thisBoard.pieces[13]) {
                if (AttackPlaces::QueenAttack[qSq][sq] && (AttackPlaces::BetweenMask[qSq][sq] & wholeBoard) == 0) {
                    attackedByQ = true; break;
                }
            }
            if (attackedByQ) {
                bool defended = BoardLogic::UnderAttack(thisBoard, sq, false);
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
        if (thisBoard.mainBoard[sq] == 9) {
            bool attackedByQ = false;
            for (int qSq : thisBoard.pieces[5]) {
                if (AttackPlaces::QueenAttack[qSq][sq] && (AttackPlaces::BetweenMask[qSq][sq] & wholeBoard) == 0) {
                    attackedByQ = true; break;
                }
            }
            if (attackedByQ) {
                bool defended = BoardLogic::UnderAttack(thisBoard, sq, true);
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
                    if (thisBoard.mainBoard[36] != 1) canChallenge = true; // e5 pawn blocks e-file pawns
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
                    if (thisBoard.mainBoard[28] != 9) canChallenge = true; // e4 pawn blocks e-file pawns
                }
            }
            if (!canChallenge) movement -= 16;
        }
    }

    return movement;
}

inline int GetPawnStructureValue(Board& thisBoard, int phase, const TunerEvaluationState& state)
{
    long long whitePawns = thisBoard.whitePawns;
    long long blackPawns = thisBoard.blackPawns;
    std::vector<std::vector<int>> whitePawnPerColumn(8);
    std::vector<std::vector<int>> blackPawnPerColumn(8);

    for (int item : thisBoard.pieces[1])
    {
        whitePawnPerColumn[item % 8].push_back(item);
    }
    int doubledPawnValueWhite = 0;
    for (int counter = 0; counter < 8; counter++)
    {
        if (whitePawnPerColumn[counter].size() > 1)
        {
            int penalty = state.DoubledPawnValue * (whitePawnPerColumn[counter].size() - 1);
            for (int psq : whitePawnPerColumn[counter])
            {
                if ((AttackPlaces::WhitePawnAttackPlaces[psq] & whitePawns) != 0 ||
                    (AttackPlaces::BlackPawnAttackPlaces[psq] & whitePawns) != 0)
                {
                    penalty = penalty / 2;
                    break;
                }
            }
            doubledPawnValueWhite += penalty;
        }
    }
    int singlePastWhite = 0;
    for (int pawnPlace : thisBoard.pieces[1])
    {
        if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & blackPawns) == 0)
        {
            int mgVal = state.WhitePassedPawnValueMiddleGam[pawnPlace];
            int egVal = state.WhitePassedPawnValueEndGame[pawnPlace];
            singlePastWhite += TaperGroup3Value(mgVal, egVal, phase);
        }
    }
    int isolatedPawnValueWhite = 0;
    const int isolatedPenalty = TaperEvaluationValue(
        state.IsolatedPawnMiddleGame, state.IsolatedPawnEndGame, phase);
    for (int pawnPlace : thisBoard.pieces[1])
    {
        if (IsIsolatedPawn(whitePawns, pawnPlace))
            isolatedPawnValueWhite += isolatedPenalty;
    }
    int goForwardPawnWhite = 0;
    for (int pawnPlace : thisBoard.pieces[1])
    {
        const int endGameValue = (pawnPlace / 8) * state.EndgamePawnAdvancementRankMultiplier;
        goForwardPawnWhite += TaperGroup3Value(0, endGameValue, phase);
    }
    int pawnChainWhite = 0;
    const int whiteKingSq = thisBoard.pieces[6].front();
    const int blackKingSq = thisBoard.pieces[14].front();
    for (int pawnPlace : thisBoard.pieces[1])
    {
        if ((AttackPlaces::BlackPawnAttackPlaces[pawnPlace] & whitePawns) != 0)
        {
            pawnChainWhite += 6;
            int r = pawnPlace / 8, f = pawnPlace % 8;
            if (f >= 2 && f <= 5)
            {
                if (r >= 3) pawnChainWhite += 6;
                if (r >= 4) pawnChainWhite += 6;
                if ((r == 4 || r == 5) && Detail::ChebyshevDistance(pawnPlace, blackKingSq) <= 3)
                {
                    pawnChainWhite += TaperGroup1Value(14, 2, phase);
                }
            }
        }
    }
    int whitePawnSum = doubledPawnValueWhite + singlePastWhite + isolatedPawnValueWhite + goForwardPawnWhite + pawnChainWhite;

    for (int item : thisBoard.pieces[9])
    {
        blackPawnPerColumn[item % 8].push_back(item);
    }
    int doubledPawnValueBlack = 0;
    for (int counter = 0; counter < 8; counter++)
    {
        if (blackPawnPerColumn[counter].size() > 1)
        {
            int penalty = state.DoubledPawnValue * (blackPawnPerColumn[counter].size() - 1);
            for (int psq : blackPawnPerColumn[counter])
            {
                if ((AttackPlaces::BlackPawnAttackPlaces[psq] & blackPawns) != 0 ||
                    (AttackPlaces::WhitePawnAttackPlaces[psq] & blackPawns) != 0)
                {
                    penalty = penalty / 2;
                    break;
                }
            }
            doubledPawnValueBlack += penalty;
        }
    }
    int singlePastBlack = 0;
    for (int pawnPlace : thisBoard.pieces[9])
    {
        if ((PassedPawnSetup::BlackPassedMask[pawnPlace] & whitePawns) == 0)
        {
            int mgVal = state.BlackPassedPawnValueMiddleGam[pawnPlace];
            int egVal = state.BlackPassedPawnValueEndGam[pawnPlace];
            singlePastBlack += TaperGroup3Value(mgVal, egVal, phase);
        }
    }
    int isolatedPawnValueBlack = 0;
    for (int pawnPlace : thisBoard.pieces[9])
    {
        if (IsIsolatedPawn(blackPawns, pawnPlace))
            isolatedPawnValueBlack += isolatedPenalty;
    }
    int goForwardPawnBlack = 0;
    for (int pawnPlace : thisBoard.pieces[9])
    {
        const int endGameValue = (7 - (pawnPlace / 8)) * state.EndgamePawnAdvancementRankMultiplier;
        goForwardPawnBlack += TaperGroup3Value(0, endGameValue, phase);
    }
    int pawnChainBlack = 0;
    for (int pawnPlace : thisBoard.pieces[9])
    {
        if ((AttackPlaces::WhitePawnAttackPlaces[pawnPlace] & blackPawns) != 0)
        {
            pawnChainBlack += 6;
            int r = pawnPlace / 8, f = pawnPlace % 8;
            if (f >= 2 && f <= 5)
            {
                if (r <= 4) pawnChainBlack += 6;
                if (r <= 3) pawnChainBlack += 6;
                if ((r == 3 || r == 2) && Detail::ChebyshevDistance(pawnPlace, whiteKingSq) <= 3)
                {
                    pawnChainBlack += TaperGroup1Value(14, 2, phase);
                }
            }
        }
    }
    int blackPawnSum = doubledPawnValueBlack + singlePastBlack + isolatedPawnValueBlack + goForwardPawnBlack + pawnChainBlack;

    return whitePawnSum - blackPawnSum;
}

} // namespace Detail

class TunerEvaluator
{
public:
    static int Evaluate(Board& thisBoard, const TunerEvaluationState& state)
    {
        long long piecesBinary = thisBoard.whitePieces | thisBoard.blackPieces;
        MyList (&pieces)[15] = thisBoard.pieces;

        int whitePieceEvaluation = pieces[1].size() * state.PawnValue
            + pieces[2].size() * state.KnightValue
            + pieces[3].size() * (state.BishopValue + (8 - (pieces[1].size() + pieces[9].size())) * state.BishopOpenFilePawnScale)
            + pieces[4].size() * state.RookValue
            + pieces[5].size() * state.QueenValue;

        int blackPieceEvaluation = pieces[9].size() * state.PawnValue
            + pieces[10].size() * state.KnightValue
            + pieces[11].size() * (state.BishopValue + (8 - (pieces[1].size() + pieces[9].size())) * state.BishopOpenFilePawnScale)
            + pieces[12].size() * state.RookValue
            + pieces[13].size() * state.QueenValue;

        double pieceBalance = 1;
        if (whitePieceEvaluation > blackPieceEvaluation)
        {
            pieceBalance = static_cast<double>(whitePieceEvaluation + state.MaterialBalanceOffset) / (blackPieceEvaluation + state.MaterialBalanceOffset);
        }
        else if (whitePieceEvaluation < blackPieceEvaluation)
        {
            pieceBalance = static_cast<double>(blackPieceEvaluation + state.MaterialBalanceOffset) / (whitePieceEvaluation + state.MaterialBalanceOffset);
        }

        if (whitePieceEvaluation > blackPieceEvaluation)
        {
            if (pieces[1].size() == 0)
            {
                pieceBalance *= (state.PawnDeficitZeroPawnMultiplierPermille / 1000.0);
            }
            if (pieces[1].size() == 1)
            {
                pieceBalance *= (state.PawnDeficitOnePawnMultiplierPermille / 1000.0);
            }
        }
        else if (whitePieceEvaluation < blackPieceEvaluation)
        {
            if (pieces[9].size() == 0)
            {
                pieceBalance *= (state.PawnDeficitZeroPawnMultiplierPermille / 1000.0);
            }
            if (pieces[9].size() == 1)
            {
                pieceBalance *= (state.PawnDeficitOnePawnMultiplierPermille / 1000.0);
            }
        }

        int pieceEvaluation = (int)((whitePieceEvaluation - blackPieceEvaluation) * pieceBalance);

        // Bishop pair
        int whiteBishopPair = 0;
        int blackBishopPair = 0;
        const int totalPawns = pieces[1].size() + pieces[9].size();
        const int bpBonus = std::max(0, state.BishopPairValue - 2 - totalPawns * 3);
        if (pieces[3].size() == 2 && ((pieces[3][0] / 8 + pieces[3][0] % 8) % 2) != ((pieces[3][1] / 8 + pieces[3][1] % 8) % 2))
        {
            whiteBishopPair = bpBonus;
        }
        if (pieces[11].size() == 2 && ((pieces[11][0] / 8 + pieces[11][0] % 8) % 2) != ((pieces[11][1] / 8 + pieces[11][1] % 8) % 2))
        {
            blackBishopPair = bpBonus;
        }
        int bishopPairValue = whiteBishopPair - blackBishopPair;

        int phase = Detail::CalculatePhase(thisBoard);

        int movement = Detail::PieceMoveCount(thisBoard, phase, state);

        // King Safety
        Detail::KingDangerResult whiteKingDanger = Detail::EvaluateKingDanger(thisBoard, true);
        Detail::KingDangerResult blackKingDanger = Detail::EvaluateKingDanger(thisBoard, false);
        int kingDangerNet = blackKingDanger.danger - whiteKingDanger.danger;

        int whiteKingSq = pieces[6].front();
        int blackKingSq = pieces[14].front();
        const CentralKingAttackPressure::Result whiteCentralPressure =
            CentralKingAttackPressure::Evaluate(thisBoard, true);
        const CentralKingAttackPressure::Result blackCentralPressure =
            CentralKingAttackPressure::Evaluate(thisBoard, false);
        int kingSafety = kingDangerNet +
                         whiteCentralPressure.contribution - blackCentralPressure.contribution;
        kingSafety += EvaluationLogic::CentralKingReadinessPenalty(thisBoard, false, phase) -
                      EvaluationLogic::CentralKingReadinessPenalty(thisBoard, true, phase);

        // Pawn Structure
        int pawnStructure = Detail::GetPawnStructureValue(thisBoard, phase, state);
        int passedPawnKingRace = Detail::EvaluatePassedPawnKingRace(thisBoard, whiteKingSq, blackKingSq);
        pawnStructure += passedPawnKingRace;
        int passedPawnMinorAccessibility = Detail::EvaluatePassedPawnMinorAccessibility(thisBoard);
        pawnStructure += passedPawnMinorAccessibility;
        pawnStructure += Detail::RookBehindPassedPawnValue(thisBoard, phase, state);
        int passedPawnCorridorSafety = Detail::EvaluatePassedPawnCorridorSafety(thisBoard);
        pawnStructure += passedPawnCorridorSafety;

        // Rook Connection
        int rookValue = Detail::RookConnectionValue(pieces, piecesBinary);

        // Tempo
        const int taperedTempo = Detail::TaperGroup1Value(state.TempoMiddleGame, state.TempoEndGame, phase);
        int temp = (!thisBoard.sideToMove) ? taperedTempo : -taperedTempo;

        // Opposite Color Bishop
        double oppositeColorBishop = 1.0;
        if (pieces[3].size() == 1 && pieces[11].size() == 1 &&
            ((pieces[3].front() / 8 + pieces[3].front() % 8) % 2) != ((pieces[11].front() / 8 + pieces[11].front() % 8) % 2))
        {
            oppositeColorBishop = ((state.OppositeColorBishopMiddleGameScalePermille / 1000.0) * phase +
                                   (state.OppositeColorBishopEndGameScalePermille / 1000.0) * (24 - phase)) / 24;
        }

        int unscaled = pieceEvaluation + bishopPairValue + movement + pawnStructure + kingSafety + rookValue + temp;

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

        return (!thisBoard.sideToMove) ? evaluation : -evaluation;
    }
};

} // namespace Tuner

#endif // HOWL_TUNER_EVALUATOR_H
