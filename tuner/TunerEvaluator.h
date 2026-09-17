#ifndef HOWL_TUNER_EVALUATOR_H
#define HOWL_TUNER_EVALUATOR_H

#include "Board.h"
#include "BoardLogic.h"
#include "EvaluationLogic.h"
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

inline int UndefendedKingZoneDanger(Board& board, bool whiteKing, int kingSquare,
                                    const TunerEvaluationState& state)
{
    const int UndefendedSquareDanger = state.KingUndefendedZoneDanger;
    const int AdditionalAttackerDanger = state.KingAdditionalZoneAttackerDanger;
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

inline int ShelterDanger(Board& board, bool whiteKing, int kingSquare,
                         const TunerEvaluationState& state)
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
        danger += secondRankPawn ? state.KingShelterSecondRankDanger
                                 : (fartherPawn ? state.KingShelterAdvancedPawnDanger
                                               : state.KingShelterMissingPawnDanger);

        bool fileHasEnemyPawn = false;
        for (int pawn : enemyPawns)
        {
            fileHasEnemyPawn |= pawn % 8 == file;
        }
        if (!secondRankPawn && !fartherPawn && !fileHasEnemyPawn)
        {
            danger += state.KingShelterOpenFileDanger;
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

inline KingDangerResult EvaluateKingDanger(Board& board, bool whiteKing,
                                           const TunerEvaluationState& state)
{
    EvaluationContext ctx(board, CalculatePhase(board));
    ctx.InitializeAttacks();
    const int attackerWeight[7] = {0, state.KingAttackerPawnWeight,
        state.KingAttackerMinorWeight, state.KingAttackerMinorWeight,
        state.KingAttackerRookWeight, state.KingAttackerQueenWeight, 0};
    const int defenderWeight[7] = {0, state.KingDefenderPawnWeight,
        state.KingDefenderMinorWeight, state.KingDefenderMinorWeight,
        state.KingDefenderRookWeight, state.KingDefenderQueenWeight, 0};
    const int kingSquare = board.pieces[whiteKing ? 6 : 14].front();
    const bool attackingWhite = !whiteKing;
    const PrecomputedKingZone& zone = KingZonesData.zones[kingSquare];
    const int kingFile = kingSquare % 8;
    const int minFile = std::max(0, kingFile - 1);
    const int maxFile = std::min(7, kingFile + 1);
    const uint8_t friendlyPawnFiles = whiteKing ? ctx.whitePawnFiles : ctx.blackPawnFiles;
    const uint8_t enemyPawnFiles = whiteKing ? ctx.blackPawnFiles : ctx.whitePawnFiles;
    int fileOpenness[8] = {};
    for (int file = minFile; file <= maxFile; ++file)
        fileOpenness[file] = (friendlyPawnFiles & (1 << file)) ? 0 :
                            ((enemyPawnFiles & (1 << file)) ? 1 : 2);

    int attackerParticipation = 0, defenderParticipation = 0;
    int attackerCount = 0, defenderCount = 0, loneAttackerSq = -1;
    int nonPawnRingAttackers = 0;
    int filePressure = 0, diagonalPressure = 0;
    const int enemySide = attackingWhite ? 0 : 1;
    const int ownSide = whiteKing ? 0 : 1;
    const uint64_t neighbours = zone.mask & ~Option::PowerTwo[kingSquare];
    const uint64_t enemyControl = ctx.sideAttacks[enemySide];
    const uint64_t undefended = neighbours & ~ctx.nonKingAttacks[ownSide];
    const int undefendedSquareCount = __builtin_popcountll(undefended & enemyControl);
    int additionalUndefendedAttackers = -undefendedSquareCount;
    const uint64_t attackingPawnAttacks = ctx.pawnAttacks[enemySide];
    uint64_t restrictedBetweenSquares = 0;
    uint64_t queenHits = 0, rookHits = 0;

    for (int type = 1; type <= 5; ++type)
    {
        for (int square : board.pieces[enemySide * 8 + type])
        {
            const uint64_t hits = ctx.attacks[square] & zone.mask;
            if (!hits) continue;
            attackerParticipation += attackerWeight[type];
            ++attackerCount;
            if (type >= 2) ++nonPawnRingAttackers;
            loneAttackerSq = square;
            additionalUndefendedAttackers += __builtin_popcountll(hits & undefended);
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
                    diagonalPressure += state.KingDiagonalLineDanger;
            }
            if (type >= 4)
            {
                for (int file = minFile; file <= maxFile; ++file)
                    if (fileOpenness[file] && (hits & (0x0101010101010101ULL << file)))
                        filePressure += fileOpenness[file] == 2
                            ? state.KingOpenLineDanger : state.KingSemiOpenLineDanger;
                if (type == 4) rookHits |= hits;
                else queenHits |= hits;
            }
        }
        for (int square : board.pieces[ownSide * 8 + type])
            if (ctx.attacks[square] & zone.mask)
            {
                defenderParticipation += defenderWeight[type];
                ++defenderCount;
            }
    }
    const int enemyKingSquare = attackingWhite ? ctx.whiteKingSq : ctx.blackKingSq;
    additionalUndefendedAttackers += __builtin_popcountll(ctx.attacks[enemyKingSquare] & undefended);
    const int undefendedKingZoneDanger =
        undefendedSquareCount * state.KingUndefendedZoneDanger +
        std::max(0, additionalUndefendedAttackers) * state.KingAdditionalZoneAttackerDanger;
    const uint64_t ownOccupancy = whiteKing ? board.whitePieces : board.blackPieces;
    const int occupiedEscapes = __builtin_popcountll(neighbours & ownOccupancy);
    const int controlledEscapes = __builtin_popcountll(neighbours & ~ownOccupancy & enemyControl);
    const int safeEscapes = zone.count - 1 - occupiedEscapes - controlledEscapes;
    const int edgeDirections = 9 - zone.count;
    const int phaseVal = ctx.phase;
    const bool hasHeavyMatingBattery = phaseVal >= 12 && (queenHits & rookHits & undefended) != 0;

    const int escapeDanger = controlledEscapes * state.KingControlledEscapeDanger
                           + (occupiedEscapes + edgeDirections) * state.KingBlockedEscapeDanger
                           + std::max(0, 3 - safeEscapes) * state.KingTrappedEscapeDanger;
    const int balanceDanger = std::max(0, attackerParticipation - defenderParticipation) +
                              std::max(0, attackerCount - defenderCount) * 4;
    int shelterDanger = ShelterDanger(board, whiteKing, kingSquare, state);
    if (whiteKing && board.whiteSmallCastle) shelterDanger = std::min(shelterDanger, ShelterDanger(board, true, 6, state));
    if (whiteKing && board.whiteBigCastle) shelterDanger = std::min(shelterDanger, ShelterDanger(board, true, 2, state));
    if (!whiteKing && board.blackSmallCastle) shelterDanger = std::min(shelterDanger, ShelterDanger(board, false, 62, state));
    if (!whiteKing && board.blackBigCastle) shelterDanger = std::min(shelterDanger, ShelterDanger(board, false, 58, state));
    int pawnStorm = 0;
    for (int pawn : board.pieces[enemySide * 8 + 1])
    {
        if (std::abs(pawn % 8 - kingFile) > 1) continue;
        const int distance = attackingWhite ? kingSquare / 8 - pawn / 8 : pawn / 8 - kingSquare / 8;
        if (distance < 1 || distance > 3) continue;
        int storm = (4 - distance) * state.KingShelterAdvancedPawnDanger;
        const int forward = pawn + (attackingWhite ? 8 : -8);
        if (forward >= 0 && forward < 64 && board.mainBoard[forward] == (whiteKing ? 1 : 9)) storm /= 2;
        pawnStorm += storm;
    }
    int safeCheckWeight = 0, safeCheckCount = 0;
    const uint64_t safeSquares = ~(attackingWhite ? board.whitePieces : board.blackPieces) &
        ~ctx.pawnAttacks[ownSide] & ~(ctx.doubleAttacks[ownSide] & ~ctx.doubleAttacks[enemySide]);
    for (int type = 2; type <= 5; ++type)
    {
        const uint64_t checkingMask = type == 2 ? AttackPlaces::KnightAttackPlaces[kingSquare]
            : (type == 3 ? AttackPlaces::BishopPseudoAttacks[kingSquare]
            : (type == 4 ? static_cast<uint64_t>(AttackPlaces::RookPseudoAttacks[kingSquare])
                         : static_cast<uint64_t>(AttackPlaces::QueenPseudoAttacks[kingSquare])));
        for (int square : board.pieces[enemySide * 8 + type])
            if (ctx.attacks[square] & checkingMask & safeSquares)
            {
                safeCheckWeight += attackerWeight[type];
                ++safeCheckCount;
            }
    }
    const int lineDanger = filePressure + diagonalPressure;
    const int defensiveRestriction = __builtin_popcountll(restrictedBetweenSquares) * 6;
    const uint64_t pinnedShelter = ctx.absolutelyPinnedPieces[ownSide] &
        (whiteKing ? board.whitePawns : board.blackPawns);
    int infiltratedQueenDanger = 0;
    for (int queen : board.pieces[enemySide * 8 + 5])
        if (whiteKing ? queen / 8 <= 1 : queen / 8 >= 6)
            infiltratedQueenDanger += state.KingInfiltratedQueenWeight;
    int rawDanger = attackerParticipation * 2 + safeCheckWeight + escapeDanger +
                    lineDanger + shelterDanger + pawnStorm + balanceDanger + undefendedKingZoneDanger +
                    defensiveRestriction + (hasHeavyMatingBattery ? state.KingHeavyBatteryDanger : 0);
    rawDanger += __builtin_popcountll(pinnedShelter) * state.KingPinnedShelterPawnWeight +
                 infiltratedQueenDanger;

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
    const bool credibleAttack = nonPawnRingAttackers >= 2 || safeCheckCount > 0 ||
                                filePressure > 0 || diagonalPressure > 0;
    int escalatedDanger = 0;
    if (!credibleAttack) rawDanger = 0;
    if (credibleAttack && attackerCount >= 2)
    {
        const int divisor = 180 + defenderParticipation * 4;
        escalatedDanger = rawDanger + (rawDanger * rawDanger) / divisor;
    }
    else if (credibleAttack && attackerCount == 1)
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

inline int PieceMoveCount(Board& board, int phase, const TunerEvaluationState& state)
{
    EvaluationContext ctx(board, phase);
    ctx.InitializeAttacks();
    const auto taper = [phase](const auto& values, int index)
    { return TaperEvaluationValue(values[0][index], values[2][index], phase); };
    const auto taper1 = [phase](const auto& values, int index)
    { return TaperGroup1Value(values[0][index], values[2][index], phase); };
    const auto taper2 = [phase](const auto& values, int index)
    { return TaperGroup2Value(values[0][index], values[2][index], phase); };

    int score = 0;
    for (int side = 0; side < 2; ++side)
    {
        const int sign = side == 0 ? 1 : -1;
        for (int type = 1; type <= 6; ++type)
            for (int sq : board.pieces[side * 8 + type])
            {
                int placement = type == 1 ? (side == 0 ? taper(state.PawnInValueWhite, sq) : taper(state.PawnInValueBlack, sq))
                    : type == 2 ? (side == 0 ? taper(state.KnightInValueWhite, sq) : taper(state.KnightInValueBlack, sq))
                    : type == 3 ? (side == 0 ? taper(state.BishopInValueWhite, sq) : taper(state.BishopInValueBlack, sq))
                    : type == 4 ? (side == 0 ? taper2(state.RookInValueWhite, sq) : taper2(state.RookInValueBlack, sq))
                    : type == 5 ? (side == 0 ? taper2(state.QueenInValueWhite, sq) : taper2(state.QueenInValueBlack, sq))
                                : (side == 0 ? taper(state.KingInValueWhite, sq) : taper(state.KingInValueBlack, sq));
                score += sign * placement;
                if (type == 2)
                    score += sign * KnightOutpostValue(board, sq, side == 0, phase, state);
                if (type >= 2 && type <= 5)
                {
                    const int count = __builtin_popcountll(ctx.attacks[sq] & ctx.mobilityArea[side]);
                    const int mobility = type == 2 ? taper1(state.KnightMoveCountValue, count)
                        : type == 3 ? taper1(state.BishopMoveCountValue, count)
                        : type == 4 ? taper2(state.RookMoveCountValue, count)
                                    : taper1(state.QueenMoveCountValue, count);
                    score += sign * mobility;
                }
                if (type == 4)
                {
                    const uint64_t fileMask = 0x0101010101010101ULL << (sq % 8);
                    const uint64_t friendlyPawns = side == 0 ? board.whitePawns : board.blackPawns;
                    const uint64_t enemyPawns = side == 0 ? board.blackPawns : board.whitePawns;
                    if (!(friendlyPawns & fileMask))
                        score += sign * (!(enemyPawns & fileMask)
                            ? TaperGroup2Value(state.RookOpenFileMiddleGame, state.RookOpenFileEndGame, phase)
                            : TaperGroup2Value(state.RookSemiOpenFileMiddleGame, state.RookSemiOpenFileEndGame, phase));
                }
            }
    }

    const auto threatScore = [&](int attackingSide)
    {
        const int victimSide = 1 - attackingSide;
        uint64_t victims = ctx.weakPieces[victimSide];
        int total = 0;
        while (victims)
        {
            const int victim = __builtin_ctzll(victims);
            victims &= victims - 1;
            const int boardVictim = board.mainBoard[victim];
            const int victimType = victimSide == 0 ? boardVictim : boardVictim - 8;
            int best = 0;
            for (int type = 1; type <= 6; ++type)
                for (int attacker : board.pieces[attackingSide * 8 + type])
                {
                    if (!(ctx.attacks[attacker] & (1ULL << victim))) continue;
                    if (type == 1 && ((ctx.pawnAttacks[victimSide] & (1ULL << attacker)) ||
                        ((ctx.doubleAttacks[victimSide] & (1ULL << attacker)) &&
                         !(ctx.doubleAttacks[attackingSide] & (1ULL << attacker))))) continue;
                    const int value = type == 1 ? taper1(state.PawnAttackValue, victimType)
                        : type == 2 ? taper1(state.KnightAttackValue, victimType)
                        : type == 3 ? taper1(state.BishopAttackValue, victimType)
                        : type == 4 ? taper2(state.RookAttackValue, victimType)
                        : type == 5 ? taper1(state.QueenAttackValue, victimType)
                                    : taper(state.KingAttackValue, victimType);
                    best = std::max(best, std::max(0, value));
                }
            total += best + ((ctx.hangingPieces[victimSide] & (1ULL << victim)) ? best / 2 : 0);
        }
        return total;
    };
    score += (threatScore(0) - threatScore(1)) * state.PieceAttackScalePercent / 100;

    const auto space = [&](int side)
    {
        const bool white = side == 0;
        const uint64_t friendlyPawns = white ? board.whitePawns : board.blackPawns;
        int units = 0;
        for (int rr = 2; rr <= 4; ++rr)
            for (int file = 2; file <= 5; ++file)
            {
                const int sq = (white ? rr - 1 : 8 - rr) * 8 + file;
                const uint64_t bit = 1ULL << sq;
                if ((friendlyPawns & bit) || (ctx.pawnAttacks[1 - side] & bit) ||
                    ((ctx.doubleAttacks[1 - side] & bit) && !(ctx.sideAttacks[side] & bit))) continue;
                ++units;
                for (int behind = 1; behind <= 2; ++behind)
                {
                    const int pawnSq = sq + (white ? 8 * behind : -8 * behind);
                    if (pawnSq >= 0 && pawnSq < 64 && (friendlyPawns & (1ULL << pawnSq)))
                    { ++units; break; }
                }
            }
        const int offset = side * 8;
        const int pieces = board.pieces[offset + 2].size() + board.pieces[offset + 3].size() +
                           board.pieces[offset + 4].size() + board.pieces[offset + 5].size();
        return units * pieces / 4;
    };
    score += (space(0) - space(1)) * phase / 24;
    return score;
}

inline int GetPawnStructureValue(Board& board, int phase, const TunerEvaluationState& state)
{
    EvaluationContext ctx(board, phase);
    const uint64_t pawns[2] = {static_cast<uint64_t>(board.whitePawns),
                               static_cast<uint64_t>(board.blackPawns)};
    uint64_t scoredConnected[2] = {0, 0};
    const auto connectedScore = [&](int side)
    {
        const bool white = side == 0;
        uint64_t remaining = pawns[side];
        int total = 0;
        while (remaining)
        {
            uint64_t component = remaining & -remaining;
            remaining &= ~component;
            for (bool changed = true; changed; )
            {
                changed = false;
                uint64_t scan = component;
                while (scan)
                {
                    const int sq = __builtin_ctzll(scan);
                    scan &= scan - 1;
                    const int file = sq % 8;
                    uint64_t neighbours = white ? AttackPlaces::BlackPawnAttackPlaces[sq]
                                                : AttackPlaces::WhitePawnAttackPlaces[sq];
                    neighbours |= white ? AttackPlaces::WhitePawnAttackPlaces[sq]
                                        : AttackPlaces::BlackPawnAttackPlaces[sq];
                    if (file > 0) neighbours |= 1ULL << (sq - 1);
                    if (file < 7) neighbours |= 1ULL << (sq + 1);
                    const uint64_t add = neighbours & remaining;
                    if (add) { component |= add; remaining &= ~add; changed = true; }
                }
            }
            if (__builtin_popcountll(component) < 2) continue;
            int frontRank = 0;
            uint64_t scan = component;
            while (scan)
            {
                const int sq = __builtin_ctzll(scan);
                scan &= scan - 1;
                frontRank = std::max(frontRank, white ? sq / 8 + 1 : 8 - sq / 8);
            }
            if (frontRank <= 3) continue;
            scoredConnected[side] |= component;
            int mg = 6, eg = 6;
            if (frontRank >= 5) { mg += 6; eg += 6; }
            if (frontRank >= 6) { mg += 6; eg += 6; }
            if ((component & ctx.supportedPawns[side]) && (component & ctx.phalanxPawns[side]))
            { mg += 6; eg += 6; }
            uint64_t front = 0;
            scan = component;
            while (scan)
            {
                const int sq = __builtin_ctzll(scan);
                scan &= scan - 1;
                if ((white ? sq / 8 + 1 : 8 - sq / 8) == frontRank) front |= 1ULL << sq;
            }
            if ((front & ctx.blockedPawns[side]) == front) { mg /= 2; eg /= 2; }
            if ((front & ctx.opposedPawns[side]) == front) { mg /= 2; eg /= 2; }
            if (frontRank == 5 || frontRank == 6)
            {
                const int enemyKing = white ? ctx.blackKingSq : ctx.whiteKingSq;
                uint64_t eligible = front & ~ctx.blockedPawns[side];
                bool nearKing = false;
                while (eligible)
                {
                    const int sq = __builtin_ctzll(eligible);
                    eligible &= eligible - 1;
                    nearKing |= ChebyshevDistance(sq, enemyKing) <= 3;
                }
                if (nearKing) { mg += 14; eg += 2; }
            }
            total += TaperGroup1Value(std::min(32, mg), std::min(20, eg), phase);
        }
        return total;
    };
    const int connected[2] = {connectedScore(0), connectedScore(1)};

    int totals[2] = {connected[0], connected[1]};
    for (int side = 0; side < 2; ++side)
    {
        const bool white = side == 0;
        for (int file = 0; file < 8; ++file)
        {
            uint64_t onFile = pawns[side] & (0x0101010101010101ULL << file);
            if (__builtin_popcountll(onFile) > 1)
            {
                const int foremost = white ? 63 - __builtin_clzll(onFile) : __builtin_ctzll(onFile);
                onFile &= ~(1ULL << foremost);
                while (onFile)
                {
                    const int sq = __builtin_ctzll(onFile);
                    onFile &= onFile - 1;
                    int penalty = state.DoubledPawnValue;
                    if ((ctx.supportedPawns[side] | ctx.phalanxPawns[side]) & (1ULL << sq))
                        penalty /= 2;
                    totals[side] += penalty;
                }
            }
        }
        for (int sq : board.pieces[side * 8 + 1])
        {
            if (IsIsolatedPawn(pawns[side], sq))
                totals[side] += TaperEvaluationValue(
                    state.IsolatedPawnMiddleGame, state.IsolatedPawnEndGame, phase);
            const uint64_t bit = 1ULL << sq;
            if (!(ctx.strictPassedPawns[side] & bit) && !(scoredConnected[side] & bit))
            {
                const int advancement = white ? sq / 8 : 7 - sq / 8;
                totals[side] += TaperGroup3Value(
                    0, advancement * state.EndgamePawnAdvancementRankMultiplier, phase);
            }
            if (ctx.strictPassedPawns[side] & bit)
                totals[side] += TaperGroup3Value(
                    white ? state.WhitePassedPawnValueMiddleGam[sq] : state.BlackPassedPawnValueMiddleGam[sq],
                    white ? state.WhitePassedPawnValueEndGame[sq] : state.BlackPassedPawnValueEndGam[sq],
                    phase);
        }
    }
    return totals[0] - totals[1];
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

        int pieceEvaluation = whitePieceEvaluation - blackPieceEvaluation;

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
        Detail::KingDangerResult whiteKingDanger = Detail::EvaluateKingDanger(thisBoard, true, state);
        Detail::KingDangerResult blackKingDanger = Detail::EvaluateKingDanger(thisBoard, false, state);
        int kingDangerNet = blackKingDanger.danger - whiteKingDanger.danger;

        int whiteKingSq = pieces[6].front();
        int blackKingSq = pieces[14].front();
        int kingSafety = kingDangerNet;

        // Pawn Structure
        int pawnStructure = Detail::GetPawnStructureValue(thisBoard, phase, state);
        int whitePassedBase = 0, blackPassedBase = 0;
        for (int sq : pieces[1])
            if ((PassedPawnSetup::WhitePassedMask[sq] & thisBoard.blackPawns) == 0)
                whitePassedBase += Detail::TaperGroup3Value(
                    state.WhitePassedPawnValueMiddleGam[sq], state.WhitePassedPawnValueEndGame[sq], phase);
        for (int sq : pieces[9])
            if ((PassedPawnSetup::BlackPassedMask[sq] & thisBoard.whitePawns) == 0)
                blackPassedBase += Detail::TaperGroup3Value(
                    state.BlackPassedPawnValueMiddleGam[sq], state.BlackPassedPawnValueEndGam[sq], phase);
        const int rawPasserContext = Detail::EvaluatePassedPawnKingRace(thisBoard, whiteKingSq, blackKingSq) +
            Detail::EvaluatePassedPawnMinorAccessibility(thisBoard) +
            Detail::RookBehindPassedPawnValue(thisBoard, phase, state) +
            Detail::EvaluatePassedPawnCorridorSafety(thisBoard);
        pawnStructure += rawPasserContext >= 0 ? std::min(rawPasserContext, whitePassedBase)
                                              : -std::min(-rawPasserContext, blackPassedBase);

        int rookValue = 0;

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

        const int loneKingMateGuidance = EvaluationLogic::LoneKingMateGuidance(
            thisBoard, state.LoneKingBase, state.LoneKingEdgeWeight,
            state.LoneKingCornerWeight, state.LoneKingConfinementWeight,
            state.LoneKingRestrictedNeighbourWeight);
        int unscaled = pieceEvaluation + bishopPairValue + movement + pawnStructure
                     + kingSafety + rookValue + loneKingMateGuidance + temp;

        double endgameScaleFactor = oppositeColorBishop;
        endgameScaleFactor = std::clamp(endgameScaleFactor, 0.0, 1.0);

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
