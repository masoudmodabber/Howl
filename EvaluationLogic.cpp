#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "EvaluationLogic.h"
#include "Option.h"
#include "KingSetup.h"
#include "AttackPlaces.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include <algorithm>
#include <iostream>

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

inline int ChebyshevDistance(int sq1, int sq2)
{
    return std::max(std::abs((sq1 % 8) - (sq2 % 8)), std::abs((sq1 / 8) - (sq2 / 8)));
}

int EvaluatePassedPawnKingRace(Board &board, int whiteKingSq, int blackKingSq)
{
    static const int rankWeight[8] = {0, 0, 0, 1, 5, 8, 14, 22};
    int whiteRaceTempo = (!board.sideToMove) ? 1 : -1;
    int blackRaceTempo = (board.sideToMove) ? 1 : -1;

    int whiteAdjustment = 0;
    for (int pawnPlace : board.pieces[1])
    {
        if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & board.blackPawns) == 0)
        {
            int blockSq = pawnPlace + 8;
            int promoSq = 56 + (pawnPlace % 8);

            int blockRace = ChebyshevDistance(blackKingSq, blockSq) - ChebyshevDistance(whiteKingSq, blockSq);
            int promotionRace = ChebyshevDistance(blackKingSq, promoSq) - ChebyshevDistance(whiteKingSq, promoSq);

            int mult = rankWeight[(pawnPlace / 8) + 1];
            whiteAdjustment += (blockRace + promotionRace + whiteRaceTempo) * mult;
        }
    }

    int blackAdjustment = 0;
    for (int pawnPlace : board.pieces[9])
    {
        if ((PassedPawnSetup::BlackPassedMask[pawnPlace] & board.whitePawns) == 0)
        {
            int blockSq = pawnPlace - 8;
            int promoSq = pawnPlace % 8;

            int blockRace = ChebyshevDistance(whiteKingSq, blockSq) - ChebyshevDistance(blackKingSq, blockSq);
            int promotionRace = ChebyshevDistance(whiteKingSq, promoSq) - ChebyshevDistance(blackKingSq, promoSq);

            int mult = rankWeight[8 - (pawnPlace / 8)];
            blackAdjustment += (blockRace + promotionRace + blackRaceTempo) * mult;
        }
    }

    return whiteAdjustment - blackAdjustment;
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

int EvaluatePassedPawnMinorAccessibility(Board &board)
{
    InitializeKnightDistance();

    static const int rankScalePercent[8] = {0, 0, 0, 25, 50, 75, 100, 100};

    auto knightCorridorValue = [&](int knightSq, int pawnPlace, bool isWhite) -> int
    {
        int minDist = 99;
        if (isWhite)
        {
            for (int sq = pawnPlace + 8; sq < 64; sq += 8)
            {
                int d = KnightDistance[knightSq][sq];
                if (d < minDist) minDist = d;
            }
        }
        else
        {
            for (int sq = pawnPlace - 8; sq >= 0; sq -= 8)
            {
                int d = KnightDistance[knightSq][sq];
                if (d < minDist) minDist = d;
            }
        }
        if (minDist == 0) return 20;
        if (minDist == 1) return 0;
        if (minDist == 2) return 10;
        if (minDist == 3) return 5;
        return 0;
    };

    int whiteNet = 0;
    for (int pawnPlace : board.pieces[1])
    {
        if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & board.blackPawns) == 0)
        {
            int accessibility = 0;
            for (int kSq : board.pieces[2])
            {
                accessibility += knightCorridorValue(kSq, pawnPlace, true);
            }
            for (int kSq : board.pieces[10])
            {
                accessibility -= knightCorridorValue(kSq, pawnPlace, true);
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
                accessibility += knightCorridorValue(kSq, pawnPlace, false);
            }
            for (int kSq : board.pieces[2])
            {
                accessibility -= knightCorridorValue(kSq, pawnPlace, false);
            }

            int relRank = 8 - (pawnPlace / 8);
            blackNet += (accessibility * rankScalePercent[relRank]) / 100;
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

int EvaluatePassedPawnCorridorSafety(Board &board)
{
    static const int rankScalePercent[8] = {0, 0, 0, 25, 50, 75, 100, 100};

    int whiteTotal = 0;
    for (int pawnPlace : board.pieces[1])
    {
        if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & board.blackPawns) == 0)
        {
            int corridorControl = 0;
            for (int sq = pawnPlace + 8; sq < 64; sq += 8)
            {
                bool friendly = IsSquareAttackedBySide(board, sq, true);
                bool enemy = IsSquareAttackedBySide(board, sq, false);

                if (enemy && !friendly)
                {
                    corridorControl -= 8;
                }
                else if (friendly && !enemy)
                {
                    corridorControl += 4;
                }
                else if (friendly && enemy)
                {
                    corridorControl -= 2;
                }
            }

            int relRank = (pawnPlace / 8) + 1;
            int scaled = (corridorControl * rankScalePercent[relRank]) / 100;
            if (scaled < -32) scaled = -32;
            if (scaled > 24) scaled = 24;
            whiteTotal += scaled;
        }
    }

    int blackTotal = 0;
    for (int pawnPlace : board.pieces[9])
    {
        if ((PassedPawnSetup::BlackPassedMask[pawnPlace] & board.whitePawns) == 0)
        {
            int corridorControl = 0;
            for (int sq = pawnPlace - 8; sq >= 0; sq -= 8)
            {
                bool friendly = IsSquareAttackedBySide(board, sq, false);
                bool enemy = IsSquareAttackedBySide(board, sq, true);

                if (enemy && !friendly)
                {
                    corridorControl -= 8;
                }
                else if (friendly && !enemy)
                {
                    corridorControl += 4;
                }
                else if (friendly && enemy)
                {
                    corridorControl -= 2;
                }
            }

            int relRank = 8 - (pawnPlace / 8);
            int scaled = (corridorControl * rankScalePercent[relRank]) / 100;
            if (scaled < -32) scaled = -32;
            if (scaled > 24) scaled = 24;
            blackTotal += scaled;
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
    return ((ray ^ Option::PowerTwo[target]) & occupiedSquares) == 0;
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

int ShelterDanger(Board& board, bool whiteKing, int kingSquare)
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

KingDangerResult EvaluateKingDanger(Board& board, bool whiteKing)
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
    int defenderCount = 0;

    for (int boardPiece = attackerFirst; boardPiece < attackerFirst + 5; boardPiece++)
    {
        const int pieceType = attackingWhite ? boardPiece : boardPiece - 8;
        for (int square : board.pieces[boardPiece])
        {
            if (PieceParticipatesInZoneFast(occupiedSquares, boardPiece, square, zone))
            {
                attackerParticipation += attackerWeight[pieceType];
                attackerCount++;
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
    const int escapeDanger = controlledEscapes * 6 + occupiedEscapes * 2 +
                             edgeDirections * 2 + std::max(0, 3 - safeEscapes) * 8;
    const int balanceDanger = std::max(0, attackerParticipation - defenderParticipation) * 2 +
                              std::max(0, attackerCount - defenderCount) * 4;
    const int shelterDanger = ShelterDanger(board, whiteKing, kingSquare);
    const int lineDanger = filePressure + diagonalPressure;
    int rawDanger = attackerParticipation * 2 + escapeDanger +
                    lineDanger + shelterDanger + balanceDanger;

    const int queenCount = board.pieces[attackingWhite ? 5 : 13].size();
    const int rookCount = board.pieces[attackingWhite ? 4 : 12].size();
    const int minorCount = board.pieces[attackingWhite ? 2 : 10].size() +
                           board.pieces[attackingWhite ? 3 : 11].size();
    const int attackingMaterialScale = std::min(100, 20 + queenCount * 45 +
                                                     rookCount * 12 + minorCount * 5);
    rawDanger = rawDanger * attackingMaterialScale / 100;
    const int escalatedDanger = rawDanger + rawDanger * rawDanger / 180;
    KingDangerResult result{std::min(escalatedDanger, 450), attackerParticipation,
            defenderParticipation, escapeDanger, filePressure,
            diagonalPressure, shelterDanger, attackingMaterialScale};
    return result;
}
}

EvaluationChessCache EvaluationLogic::EvalCache;
PawnCache EvaluationLogic::PawnEvalCache;

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
    if (pieces[3].size() == 2 && ((pieces[3][0] / 8 + pieces[3][0] % 8) % 2) != ((pieces[3][1] / 8 + pieces[3][1] % 8) % 2))    
    {
        whiteBishopPair = 50;
    }
    if (pieces[11].size() == 2 && ((pieces[11][0] / 8 + pieces[11][0] % 8) % 2) != ((pieces[11][1] / 8 + pieces[11][1] % 8) % 2))
    {
        blackBishopPair = 50;
    }
    int bishopPairVaue = whiteBishopPair - blackBishopPair;

    int phase = EvaluationLogic::CalculatePhase(thisBoard);

    // Movement
    int *movementAndKingSafety = EvaluationLogic::PieceMoveCount(thisBoard, phase);
    int movement = movementAndKingSafety[0];
    int attackNet = movementAndKingSafety[1];
    int center = movementAndKingSafety[2];
    int rookFileNet = movementAndKingSafety[3];
    int whiteRookFile = movementAndKingSafety[4];
    int blackRookFile = movementAndKingSafety[5];
    delete[] movementAndKingSafety;

    KingDangerResult whiteKingDanger = EvaluateKingDanger(thisBoard, true);
    KingDangerResult blackKingDanger = EvaluateKingDanger(thisBoard, false);
    int kingDangerNet = blackKingDanger.danger - whiteKingDanger.danger;

    int whiteKingSq = pieces[6].front();
    int blackKingSq = pieces[14].front();
    int whiteKingPlacement = (Option::WhiteKingPlaceSafetyMiddleGame[whiteKingSq] * phase
                              + Option::KingInValueWhiteEndGame[whiteKingSq] * (24 - phase)) / 24;
    int blackKingPlacement = (Option::BlackKingPlaceSafetyMiddleGame[blackKingSq] * phase
                              + Option::KingInValueBlackEndGame[blackKingSq] * (24 - phase)) / 24;
    int kingPlacementNet = whiteKingPlacement - blackKingPlacement;
    int kingSafety = kingDangerNet + kingPlacementNet;

    // Pawn Structure
    int pawnStructure = EvaluationLogic::GetPawnStructureValue(thisBoard, phase);
    int passedPawnKingRace = EvaluatePassedPawnKingRace(thisBoard, whiteKingSq, blackKingSq);
    pawnStructure += passedPawnKingRace;
    int passedPawnMinorAccessibility = EvaluatePassedPawnMinorAccessibility(thisBoard);
    pawnStructure += passedPawnMinorAccessibility;
    int passedPawnCorridorSafety = EvaluatePassedPawnCorridorSafety(thisBoard);
    pawnStructure += passedPawnCorridorSafety;

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

    int unscaled = pieceEvaluation + bishopPairVaue + movement + pawnStructure + kingSafety + rookValue + center + temp;

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
        breakdown->whitePawnShield = 0;
        breakdown->blackPawnShield = 0;
        breakdown->pawnShieldNet = 0;
        breakdown->whiteCentralKingExposure = 0;
        breakdown->blackCentralKingExposure = 0;
        breakdown->centralKingExposureNet = 0;
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

int EvaluationLogic::GetPawnStructureValue(Board &thisBoard, int phase)
{
    long long whitePawns = thisBoard.whitePawns;
    long long blackPawns = thisBoard.blackPawns;
    std::vector<std::vector<int>> whitePawnPerColumn(8);
    std::vector<std::vector<int>> blackPawnPerColumn(8);

    for (int counter = 0; counter < 8; counter++)
    {
        whitePawnPerColumn[counter] = std::vector<int>();
        blackPawnPerColumn[counter] = std::vector<int>();
    }

    int whitePawnSum;
    std::optional<int> calculatedPawnEval = PawnEvalCache.GetFromCache(whitePawns, 0, phase);
    if (calculatedPawnEval.has_value())
    {
        whitePawnSum = calculatedPawnEval.value();
    }
    else
    {
        for (int item : thisBoard.pieces[1])
        {
            whitePawnPerColumn[item % 8].push_back(item);
        }
        int doubledPawnValueWhite = 0;
        for (int counter = 0; counter < 8; counter++)
        {
            if (whitePawnPerColumn[counter].size() > 1)
            {
                doubledPawnValueWhite += Option::DoubledPawnValue * (whitePawnPerColumn[counter].size() - 1);
            }
        }
        int singlePastWhite = 0;
        for (int pawnPlace : thisBoard.pieces[1])
        {
            if ((PassedPawnSetup::WhitePassedMask[pawnPlace] & blackPawns) == 0)
            {
                int mgVal = Option::WhitePassedPawnValueMiddleGam[pawnPlace];
                int egVal = Option::WhitePassedPawnValueEndGame[pawnPlace];
                singlePastWhite += TaperGroup3Value(mgVal, egVal, phase);
            }
        }
        int goForwardPawnWhite = 0;
        for (int pawnPlace : thisBoard.pieces[1])
        {
            const int endGameValue = pawnPlace / 8 * 2;
            goForwardPawnWhite += TaperGroup3Value(0, endGameValue, phase);
        }
        whitePawnSum = doubledPawnValueWhite + singlePastWhite + goForwardPawnWhite;
        PawnEvalCache.Add(whitePawns, whitePawnSum, 0, phase);
    }
    int blackPawnSum;
    calculatedPawnEval = PawnEvalCache.GetFromCache(blackPawns, 1, phase);
    if (calculatedPawnEval.has_value())
    {
        blackPawnSum = calculatedPawnEval.value();
    }
    else
    {
        for (int item : thisBoard.pieces[9])
        {
            blackPawnPerColumn[item % 8].push_back(item);
        }
        int doubledPawnValueBlack = 0;
        for (int counter = 0; counter < 8; counter++)
        {
            if (blackPawnPerColumn[counter].size() > 1)
            {
                doubledPawnValueBlack += Option::DoubledPawnValue * (blackPawnPerColumn[counter].size() - 1);
            }
        }
        int singlePastBlack = 0;

        for (int pawnPlace : thisBoard.pieces[9])
        {
            if ((PassedPawnSetup::BlackPassedMask[pawnPlace] & whitePawns) == 0)
            {
                int mgVal = Option::BlackPassedPawnValueMiddleGam[pawnPlace];
                int egVal = Option::BlackPassedPawnValueEndGam[pawnPlace];
                singlePastBlack += TaperGroup3Value(mgVal, egVal, phase);
            }
        }
        int goForwardPawnBlack = 0;
        for (int pawnPlace : thisBoard.pieces[9])
        {
            const int endGameValue = (7 - (pawnPlace / 8)) * 2;
            goForwardPawnBlack += TaperGroup3Value(0, endGameValue, phase);
        }
        blackPawnSum = doubledPawnValueBlack + singlePastBlack + goForwardPawnBlack;
        PawnEvalCache.Add(blackPawns, blackPawnSum, 1, phase);
    }
    whitePawnSum -= blackPawnSum;
    return whitePawnSum;
}

int *EvaluationLogic::PieceMoveCount(Board &thisBoard, int phase)
{
    // int pieceMovePosition[2][3][64][7];
    int whitePieceAttack[64] = {0};
    int blackPieceAttack[64] = {0};
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
                static const int knightOffsets[8] = {17, 10, 15, 6, -10, -17, -15, -6};
                static const int knightDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement += taperedTable(Option::KnightInValueWhite, piecePoisiion);
                    centerValue += Option::KnightInCenterValueWhite[piecePoisiion];
                    for (int i = 0; i < 8; ++i)
                    {
                        int endPlace = piecePoisiion + knightOffsets[i];
                        int dir = knightDirs[i];
                        if (PieceMoves::KnightMoves[piecePoisiion][dir] != nullptr)
                        {
                            centerValue += Option::KnightMoveCenterValueWhite[endPlace];
                            if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                            {
                                whiteAttackValue += taperedGroup1Table(Option::KnightAttackValue, mainBoard[endPlace]);
                            }
                        }
                    }
                    movement += taperedGroup1Table(Option::KnightMoveCountValue, moveCount);
                }
                break;
            case 3:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement += taperedTable(Option::BishopInValueWhite, piecePoisiion);
                    centerValue += Option::BishopInCenterValueWhite[piecePoisiion];
                    for (int direction = 0; direction <= 6; direction += 2)
                    {
                        for (int counter = 0; counter < PieceMoves::BishopMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::BishopMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue += Option::BishopMoveCenterValueWhite[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPos] & blackPieces) != 0)
                            {
                                whiteAttackValue += taperedGroup1Table(Option::BishopAttackValue, endPiece);
                                break;
                            }
                            else
                            {
                                break;
                            }
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
                        }
                    }
                    moveCount = 0;
                    movement += taperedGroup2Table(Option::RookInValueWhite, piecePoisiion);
                    centerValue += Option::RookInCenterValueWhite[piecePoisiion];
                    for (int direction = 0; direction <= 6; direction += 2)
                    {
                        for (int counter = 0; counter < PieceMoves::RookMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::RookMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue += Option::RookMoveCenterValueWhite[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPos] & blackPieces) != 0)
                            {
                                whiteAttackValue += taperedGroup2Table(Option::RookAttackValue, endPiece);
                                break;
                            }
                            else
                            {
                                break;
                            }
                        }
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
                    for (int direction = 0; direction <= 14; direction += 2)
                    {
                        for (int counter = 0; counter < PieceMoves::QueenMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::QueenMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue += Option::QueenMoveCenterValueWhite[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPos] & blackPieces) != 0)
                            {
                                whiteAttackValue += taperedGroup1Table(Option::QueenAttackValue, endPiece);
                                break;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                    movement += taperedGroup1Table(Option::QueenMoveCountValue, moveCount);
                }
                break;
            case 6:
                static const int kingOffsets[8] = {7, 8, 9, 1, -7, -8, -9, -1};
                static const int kingDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement += taperedTable(Option::KingInValueWhite, piecePoisiion);
                    centerValue += Option::KingInCenterValueWhite[piecePoisiion];
                    for (int i = 0; i < 8; ++i)
                    {
                        int endPlace = piecePoisiion + kingOffsets[i];
                        int dir = kingDirs[i];
                        if (PieceMoves::WhiteKingMoves[piecePoisiion][dir] != nullptr)
                        {
                            centerValue += Option::KingMoveCenterValueWhite[endPlace];
                            if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                            {
                                whiteAttackValue += taperedTable(Option::KingAttackValue, mainBoard[endPlace]);
                            }
                        }
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
                static const int knightOffsets[8] = {17, 10, 15, 6, -10, -17, -15, -6};
                static const int knightDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement -= taperedTable(Option::KnightInValueBlack, piecePoisiion);
                    centerValue -= Option::KnightInCenterValueBlack[piecePoisiion];
                    for (int i = 0; i < 8; ++i)
                    {
                        int endPlace = piecePoisiion + knightOffsets[i];
                        int dir = knightDirs[i];
                        if (PieceMoves::KnightMoves[piecePoisiion][dir] != nullptr)
                        {
                            centerValue -= Option::KnightMoveCenterValueBlack[endPlace];
                            if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                            {
                                blackAttackValue += taperedGroup1Table(Option::KnightAttackValue, mainBoard[endPlace]);
                            }
                        }
                    }
                    movement -= taperedGroup1Table(Option::KnightMoveCountValue, moveCount);
                }
                break;
            case 11:
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement -= taperedTable(Option::BishopInValueBlack, piecePoisiion);
                    centerValue -= Option::BishopInCenterValueBlack[piecePoisiion];

                    for (int direction = 0; direction <= 6; direction += 2)
                    {
                        for (int counter = 0; counter < PieceMoves::BishopMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::BishopMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue -= Option::BishopMoveCenterValueBlack[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                // pieceMovePosition[1][1][endPos][piece - 8]++;
                                moveCount++;
                                // movement -= taperedTable(Option::BishopMoveValueBlack, endPos);
                            }
                            else if ((Option::PowerTwo[endPos] & whitePieces) != 0)
                            {
                                // pieceMovePosition[1][2][endPos][piece - 8]++;
                                blackAttackValue += taperedGroup1Table(Option::BishopAttackValue, endPiece);
                                break;
                            }
                            else
                            {
                                break;
                            }
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
                        }
                    }
                    moveCount = 0;
                    movement -= taperedGroup2Table(Option::RookInValueBlack, piecePoisiion);
                    centerValue -= Option::RookInCenterValueBlack[piecePoisiion];

                    for (int direction = 0; direction <= 6; direction += 2)
                    {
                        for (int counter = 0; counter < PieceMoves::RookMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::RookMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue -= Option::RookMoveCenterValueBlack[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                // pieceMovePosition[1][1][endPos][piece - 8]++;
                                moveCount++;
                                // movement -= taperedTable(Option::RookMoveValueBlack, endPos);
                            }
                            else if ((Option::PowerTwo[endPos] & whitePieces) != 0)
                            {
                                // pieceMovePosition[1][2][endPos][piece - 8]++;
                                blackAttackValue += taperedGroup2Table(Option::RookAttackValue, endPiece);
                                break;
                            }
                            else
                            {
                                break;
                            }
                        }
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

                    int directions[] = {0, 2, 4, 6, 8, 10, 12, 14};

                    for (int dir = 0; dir < 8; dir++)
                    {
                        int direction = directions[dir];
                        for (int counter = 0; counter < PieceMoves::QueenMoves[piecePoisiion][direction].size(); counter++)
                        {
                            int endPos = PieceMoves::QueenMoves[piecePoisiion][direction][counter]->endPlace;
                            int endPiece = mainBoard[endPos];
                            centerValue -= Option::QueenMoveCenterValueBlack[endPos];
                            if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPos] & whitePieces) != 0)
                            {
                                blackAttackValue += taperedGroup1Table(Option::QueenAttackValue, endPiece);
                                break;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                    movement -= taperedGroup1Table(Option::QueenMoveCountValue, moveCount);
                }
                break;
            case 14:
                static const int kingOffsets[8] = {7, 8, 9, 1, -7, -8, -9, -1};
                static const int kingDirs[8] = {0, 2, 4, 6, 8, 10, 12, 14};
                for (int piecePoisiion : thisBoard.pieces[piece])
                {
                    moveCount = 0;
                    movement -= taperedTable(Option::KingInValueBlack, piecePoisiion);
                    centerValue -= Option::KingInCenterValueBlack[piecePoisiion];
                    for (int i = 0; i < 8; ++i)
                    {
                        int endPlace = piecePoisiion + kingOffsets[i];
                        int dir = kingDirs[i];
                        if (PieceMoves::BlackKingMoves[piecePoisiion][dir] != nullptr)
                        {
                            centerValue -= Option::KingMoveCenterValueBlack[endPlace];
                            if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                            {
                                moveCount++;
                            }
                            else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                            {
                                blackAttackValue += taperedTable(Option::KingAttackValue, mainBoard[endPlace]);
                            }
                        }
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

    int *movementAndKingSafetyAndCenter = new int[6];
    movementAndKingSafetyAndCenter[0] = movement;
    movementAndKingSafetyAndCenter[1] = scaledAttackNet;
    movementAndKingSafetyAndCenter[2] = centerValue;
    movementAndKingSafetyAndCenter[3] = rookFileNet;
    movementAndKingSafetyAndCenter[4] = whiteRookFileBonus;
    movementAndKingSafetyAndCenter[5] = blackRookFileBonus;
    return movementAndKingSafetyAndCenter;
}
