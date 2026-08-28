#include "QSearcher.h"
#include "PVSSearch.h"
#include <chrono>
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "MoveLogic.h"
#include "Move.h"
#include "Board.h"
#include "Search.h"
#include "Option.h"
#include "PieceMoves.h"
#include "AttackPlaces.h"
#include <algorithm>
#include <cstdint>

namespace
{
constexpr std::uint32_t PackedAttackerUnits[7] = {
    0,
    std::uint32_t{1} << 0,
    std::uint32_t{1} << 2,
    std::uint32_t{1} << 6,
    std::uint32_t{1} << 9,
    std::uint32_t{1} << 12,
    std::uint32_t{1} << 16
};

constexpr std::uint32_t PackedAttackerMasks[7] = {
    0,
    std::uint32_t{0x3} << 0,
    std::uint32_t{0xf} << 2,
    std::uint32_t{0x7} << 6,
    std::uint32_t{0x7} << 9,
    std::uint32_t{0xf} << 12,
    std::uint32_t{0x1} << 16
};

void AddPackedAttacker(std::uint32_t& attackers, int pieceType)
{
    attackers += PackedAttackerUnits[pieceType];
}

int PopLeastValuableAttacker(std::uint32_t& attackers)
{
    for (int pieceType = 1; pieceType <= 6; ++pieceType)
    {
        if ((attackers & PackedAttackerMasks[pieceType]) != 0)
        {
            attackers -= PackedAttackerUnits[pieceType];
            return pieceType;
        }
    }
    return 0;
}

bool IsSoleAttacker(std::uint32_t attackers, int pieceType)
{
    return attackers == PackedAttackerUnits[pieceType];
}

int NormalizeExchangePiece(int piece)
{
    return piece > 8 ? piece - 8 : piece;
}

std::uint64_t MakeExchangeKey(std::uint32_t attacker, std::uint32_t defender,
                              int beginPiece, int endPiece, int promotionPiece)
{
    return static_cast<std::uint64_t>(attacker)
        | (static_cast<std::uint64_t>(defender) << 17)
        | (static_cast<std::uint64_t>(beginPiece) << 34)
        | (static_cast<std::uint64_t>(NormalizeExchangePiece(endPiece)) << 37)
        | (static_cast<std::uint64_t>(promotionPiece % 8) << 40);
}
}

double MoveLogic::pieceValue[15];
int MoveLogic::pieceMoveStack[15];
ExchangeChessCache MoveLogic::ExchangeCache;
ExchangeChessCache MoveLogic::ExchangeCacheWithoutBeginPiece;

bool MoveLogic::initialized = false;

void MoveLogic::Initialize()
{
    if (!initialized)
    {
        pieceMoveStack[1] = 2;
        pieceMoveStack[2] = 3;
        pieceMoveStack[3] = 4;
        pieceMoveStack[4] = 5;
        pieceMoveStack[5] = 1;
        pieceMoveStack[6] = 6;
        pieceMoveStack[9] = 10;
        pieceMoveStack[10] = 11;
        pieceMoveStack[11] = 12;
        pieceMoveStack[12] = 13;
        pieceMoveStack[13] = 9;
        pieceMoveStack[14] = 14;
        pieceValue[0] = 0.0;
        pieceValue[1] = 1.0;
        pieceValue[2] = 3.5;
        pieceValue[3] = 3.5;
        pieceValue[4] = 5.5;
        pieceValue[5] = 9.75;
        pieceValue[6] = 25;
        pieceValue[9] = 1.0;
        pieceValue[10] = 3.5;
        pieceValue[11] = 3.5;
        pieceValue[12] = 5.5;
        pieceValue[13] = 9.75;
        pieceValue[14] = 25;
        initialized = true;
    }
}

MoveList MoveLogic::MoveGenerator(Board &thisBoard, int depth, int depthGone, bool onlyCapturesAndChecks)
{
    MoveList moveList;
    Move* complicatedMoves[256];
    int complicatedCount = 0;
    if (Search::moveCount == 14962)
    {
        int x = 1;
    }
    AttackerState whiteAttacker = SetWhiteAttacker(thisBoard);
    AttackerState blackAttacker = SetBlackAttacker(thisBoard);
    long long whitePieces = thisBoard.whitePieces;
    long long blackPieces = thisBoard.blackPieces;
    int *mainBoard = thisBoard.mainBoard;
    long long wholeBoard = whitePieces | blackPieces;

    int enemyKingPos = -1;
    long long enemyKingBit = 0;
    long long friendlySliderRayMask = 0;
    if (onlyCapturesAndChecks)
    {
        int enemyKingIndex = (!thisBoard.sideToMove ? 14 : 6);
        if (thisBoard.pieces[enemyKingIndex].count > 0)
        {
            enemyKingPos = thisBoard.pieces[enemyKingIndex].front();
            enemyKingBit = Option::PowerTwo[enemyKingPos];
            int offset = (!thisBoard.sideToMove ? 0 : 8);
            for (int p = 3; p <= 5; ++p)
            {
                for (int pos : thisBoard.pieces[p + offset])
                {
                    friendlySliderRayMask |= AttackPlaces::LineMask[pos][enemyKingPos];
                }
            }
        }
    }

    if (!thisBoard.sideToMove)
    {
        for (int pieceCounter = 1; pieceCounter < 7; pieceCounter++)
        {
            int piece = pieceMoveStack[pieceCounter];
            switch (piece)
            {
            case 1:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    if (PieceMoves::WhitePawnMoves[piecePosition][0] != nullptr)
                    {
                        int endPlace = piecePosition + 16;
                        if (!onlyCapturesAndChecks || srcOnRay || ((AttackPlaces::WhitePawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (whiteAttacker.pieceCounts[endPlace] != 0 || endPlace >= 40)))
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][0]);
                            if ((PieceMoves::pawnTwoMove[piecePosition] & wholeBoard) == 0)
                            {
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                newMove->value = ExchangeWithoutBeginPiece(whiteAttacker.pieceCounts[newMove->endPlace], blackAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                                complicatedMoves[complicatedCount++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][1] != nullptr && (Option::PowerTwo[piecePosition + 8] & wholeBoard) == 0)
                    {
                        int endPlace = piecePosition + 8;
                        if (!onlyCapturesAndChecks || srcOnRay || ((AttackPlaces::WhitePawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (whiteAttacker.pieceCounts[endPlace] != 0 || endPlace >= 40)))
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(whiteAttacker.pieceCounts[newMove->endPlace], blackAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                            complicatedMoves[complicatedCount++] = newMove;
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][2] != nullptr && (Option::PowerTwo[piecePosition + 8] & wholeBoard) == 0)
                    {
                        for (int i = 2; i <= 5; i++)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(whiteAttacker.pieceCounts[newMove->endPlace], blackAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 5 - (i - 2));
                            complicatedMoves[complicatedCount++] = newMove;
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][6] != nullptr && piecePosition + 7 == thisBoard.unpassentPlace)
                    {
                        Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][6]);
                        newMove->endPiece = 9;
                        moveList.moves[moveList.count++] = newMove;
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][7] != nullptr && piecePosition + 9 == thisBoard.unpassentPlace)
                    {
                        Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][7]);
                        newMove->endPiece = 9;
                        moveList.moves[moveList.count++] = newMove;
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][8] != nullptr && (Option::PowerTwo[piecePosition + 7] & blackPieces) != 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][8]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                        moveList.moves[moveList.count++] = newMove;
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][9] != nullptr && (Option::PowerTwo[piecePosition + 7] & blackPieces) != 0)
                    {
                        for (int i = 9; i <= 12; i++)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][13] != nullptr && (Option::PowerTwo[piecePosition + 9] & blackPieces) != 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][13]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                        moveList.moves[moveList.count++] = newMove;
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][14] != nullptr && (Option::PowerTwo[piecePosition + 9] & blackPieces) != 0)
                    {
                        for (int i = 14; i <= 17; i++)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                }
                break;
            case 2:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace = piecePosition + 17;
                    if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][0]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 10;
                    if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][2]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 15;
                    if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][4]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 6;
                    if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][6]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 10;
                    if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][8]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 17;
                    if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][10]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 15;
                    if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][12]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 6;
                    if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][14]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                }
                break;
            case 3:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 4:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 5:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][8][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][9][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][10][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][11][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][12][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][13][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][14][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][15][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 6:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace;
                    endPlace = piecePosition + 7;
                    if (PieceMoves::WhiteKingMoves[piecePosition][0] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][0]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 8;
                    if (PieceMoves::WhiteKingMoves[piecePosition][2] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][2]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 9;
                    if (PieceMoves::WhiteKingMoves[piecePosition][4] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][4]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 1;
                    if (PieceMoves::WhiteKingMoves[piecePosition][6] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][6]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 7;
                    if (PieceMoves::WhiteKingMoves[piecePosition][8] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][8]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 8;
                    if (PieceMoves::WhiteKingMoves[piecePosition][10] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][10]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 9;
                    if (PieceMoves::WhiteKingMoves[piecePosition][12] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][12]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 1;
                    if (PieceMoves::WhiteKingMoves[piecePosition][14] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][14]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    if (thisBoard.whiteSmallCastle && (!onlyCapturesAndChecks || AttackPlaces::LineMask[5][enemyKingPos] != 0 || AttackPlaces::LineMask[6][enemyKingPos] != 0) && blackAttacker.pieceCounts[4] == 0 && blackAttacker.pieceCounts[5] == 0 && blackAttacker.pieceCounts[6] == 0 && mainBoard[5] == 0 && mainBoard[6] == 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][16]);
                        newMove->value = 50;
                        complicatedMoves[complicatedCount++] = newMove;
                    }
                    if (thisBoard.whiteBigCastle && (!onlyCapturesAndChecks || AttackPlaces::LineMask[3][enemyKingPos] != 0 || AttackPlaces::LineMask[2][enemyKingPos] != 0) && blackAttacker.pieceCounts[4] == 0 && blackAttacker.pieceCounts[3] == 0 && blackAttacker.pieceCounts[2] == 0 && mainBoard[3] == 0 && mainBoard[2] == 0 && mainBoard[1] == 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][17]);
                        newMove->value = 50;
                        complicatedMoves[complicatedCount++] = newMove;
                    }
                }
                break;
            }
        }
    }
    else
    {
        for (int pieceCounter = 9; pieceCounter < 15; pieceCounter++)
        {
            int piece = pieceMoveStack[pieceCounter];
            switch (piece)
            {
            case 9:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int exchangeInPlace = -ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[piecePosition], whiteAttacker.pieceCounts[piecePosition], piecePosition, piece - 8, 0, 0);
                    if (PieceMoves::BlackPawnMoves[piecePosition][0] != nullptr)
                    {
                        int endPlace = piecePosition - 16;
                        if (!onlyCapturesAndChecks || srcOnRay || ((AttackPlaces::BlackPawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (blackAttacker.pieceCounts[endPlace] != 0 || endPlace <= 23)))
                        {
                            if ((PieceMoves::pawnTwoMove[piecePosition] & wholeBoard) == 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][0]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                newMove->value = ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[newMove->endPlace], whiteAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                                complicatedMoves[complicatedCount++] = newMove;
                            }
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][1] != nullptr && (Option::PowerTwo[piecePosition - 8] & wholeBoard) == 0)
                    {
                        int endPlace = piecePosition - 8;
                        if (!onlyCapturesAndChecks || srcOnRay || ((AttackPlaces::BlackPawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (blackAttacker.pieceCounts[endPlace] != 0 || endPlace <= 23)))
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[newMove->endPlace], whiteAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                            complicatedMoves[complicatedCount++] = newMove;
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][2] != nullptr && (Option::PowerTwo[piecePosition - 8] & wholeBoard) == 0)
                    {
                        for (int i = 2; i <= 5; i++)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[newMove->endPlace], whiteAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 5 - (i - 2));
                            complicatedMoves[complicatedCount++] = newMove;
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][6] != nullptr && piecePosition - 7 == thisBoard.unpassentPlace)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][6]);
                        newMove->endPiece = 1;
                        moveList.moves[moveList.count++] = newMove;
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][7] != nullptr && piecePosition - 9 == thisBoard.unpassentPlace)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][7]);
                        newMove->endPiece = 1;
                        moveList.moves[moveList.count++] = newMove;
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][8] != nullptr && (Option::PowerTwo[piecePosition - 7] & whitePieces) != 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][8]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                        moveList.moves[moveList.count++] = newMove;
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][9] != nullptr && (Option::PowerTwo[piecePosition - 7] & whitePieces) != 0)
                    {
                        for (int i = 9; i <= 12; i++)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][13] != nullptr && (Option::PowerTwo[piecePosition - 9] & whitePieces) != 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][13]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                        moveList.moves[moveList.count++] = newMove;
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][14] != nullptr && (Option::PowerTwo[piecePosition - 9] & whitePieces) != 0)
                    {
                        for (int i = 14; i <= 17; i++)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                }
                break;
            case 10:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace = piecePosition + 17;
                    if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][0]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 10;
                    if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][2]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 15;
                    if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][4]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 6;
                    if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][6]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 10;
                    if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][8]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 17;
                    if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][10]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 15;
                    if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][12]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 6;
                    if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)
                            {
                                Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][14]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                }
                break;
            case 11:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 12:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 13:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][8][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][9][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][10][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][11][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][12][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][13][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][14][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))
                            {
                                moveList.moves[moveList.count++] = newMove;
                            }
                            else
                            {
                                delete newMove;
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            delete newMove;
                            newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][15][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                            break;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 14:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace;
                    endPlace = piecePosition + 7;
                    if (PieceMoves::BlackKingMoves[piecePosition][0] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][0]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 8;
                    if (PieceMoves::BlackKingMoves[piecePosition][2] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][2]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 9;
                    if (PieceMoves::BlackKingMoves[piecePosition][4] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][4]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 1;
                    if (PieceMoves::BlackKingMoves[piecePosition][6] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][6]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 7;
                    if (PieceMoves::BlackKingMoves[piecePosition][8] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][8]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 8;
                    if (PieceMoves::BlackKingMoves[piecePosition][10] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][10]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 9;
                    if (PieceMoves::BlackKingMoves[piecePosition][12] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][12]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 1;
                    if (PieceMoves::BlackKingMoves[piecePosition][14] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || srcOnRay)
                            {
                                Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][14]);
                                moveList.moves[moveList.count++] = newMove;
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    if (thisBoard.blackSmallCastle && (!onlyCapturesAndChecks || AttackPlaces::LineMask[61][enemyKingPos] != 0 || AttackPlaces::LineMask[62][enemyKingPos] != 0) && whiteAttacker.pieceCounts[60] == 0 && whiteAttacker.pieceCounts[61] == 0 && whiteAttacker.pieceCounts[62] == 0 && mainBoard[61] == 0 && mainBoard[62] == 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][16]);
                        newMove->value = 25;
                        complicatedMoves[complicatedCount++] = newMove;
                    }
                    if (thisBoard.blackBigCastle && (!onlyCapturesAndChecks || AttackPlaces::LineMask[59][enemyKingPos] != 0 || AttackPlaces::LineMask[58][enemyKingPos] != 0) && whiteAttacker.pieceCounts[60] == 0 && whiteAttacker.pieceCounts[59] == 0 && whiteAttacker.pieceCounts[58] == 0 && mainBoard[59] == 0 && mainBoard[58] == 0 && mainBoard[57] == 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][17]);
                        newMove->value = 25;
                        complicatedMoves[complicatedCount++] = newMove;
                    }
                }
                break;
            }
        }
    }
    for (int i = 0; i < complicatedCount; ++i) {
        moveList.moves[moveList.count++] = complicatedMoves[i];
    }
    // --- END REPLACEMENT OF VECTOR USAGE ---

    ScoreAndSortMoves(thisBoard, moveList, depth, depthGone, whiteAttacker, blackAttacker);
    return moveList;
}

void MoveLogic::ScoreAndSortMoves(Board& thisBoard, MoveList& moveList, int depth, int depthGone, const AttackerState& whiteAttacker, const AttackerState& blackAttacker)
{
    int state = 0;
    int* mainBoard = thisBoard.mainBoard;
    if (thisBoard.sideToMove)
    {
        for (int i = 0; i < moveList.count; ++i)
        {
            Move* move = moveList.moves[i];
            int beginPiece = mainBoard[move->beginPlace] % 8;
            move->value = MoveLogic::Exchange(blackAttacker.pieceCounts[move->endPlace], whiteAttacker.pieceCounts[move->endPlace], move->endPlace, beginPiece, move->endPiece, move->promotionPiece);
            move->value += whiteAttacker.orderingScores[move->beginPlace] + whiteAttacker.orderingScores[move->endPlace];
            move->value += Option::MoveOrderingValueBlack[state][beginPiece][move->endPlace];
        }
    }
    else
    {
        for (int i = 0; i < moveList.count; ++i)
        {
            Move* move = moveList.moves[i];
            int beginPiece = mainBoard[move->beginPlace];
            move->value = MoveLogic::Exchange(whiteAttacker.pieceCounts[move->endPlace], blackAttacker.pieceCounts[move->endPlace], move->endPlace, beginPiece, move->endPiece % 8, move->promotionPiece);
            move->value += blackAttacker.orderingScores[move->beginPlace] + blackAttacker.orderingScores[move->endPlace];
            move->value += Option::MoveOrderingValueWhite[state][beginPiece][move->endPlace];
        }
    }
    for (int i = 0; i < moveList.count; ++i)
    {
        Move* move = moveList.moves[i];
        move->depth = depth;
        move->depthGone = depthGone;
        move->moveCount = Search::moveCount;
    }
    std::sort(moveList.moves, moveList.moves + moveList.count, [](const Move *a, const Move *b)
              { return b->value < a->value; });
}

MoveList MoveLogic::QSearchStage1Generator(Board &thisBoard, int depth, int depthGone, DeferredMove* deferredMoves, int& deferredCount, const Move& prevMove)
{
    deferredCount = 0;
    MoveList fullList = MoveGenerator(thisBoard, depth, depthGone, true);

    AttackerState whiteAttacker = SetWhiteAttacker(thisBoard);
    AttackerState blackAttacker = SetBlackAttacker(thisBoard);
    const int* mainBoard = thisBoard.mainBoard;
    static const int pieceValue100[15] = {0, 100, 350, 350, 550, 975, 2500, 0, 0, 100, 350, 350, 550, 975, 2500};

    MoveList stage1List;
    for (int i = 0; i < fullList.count; ++i)
    {
        Move* m = fullList.moves[i];
        bool isPromotion = (m->promotionPiece > 0);
        bool isCapture = (m->endPiece > 0);

        if (isPromotion)
        {
            stage1List.moves[stage1List.count++] = m;
        }
        else if (isCapture)
        {
            int beginPiece = thisBoard.sideToMove ? (mainBoard[m->beginPlace] % 8) : mainBoard[m->beginPlace];
            int endPiece = thisBoard.sideToMove ? m->endPiece : (m->endPiece % 8);
            int exch = thisBoard.sideToMove
                ? MoveLogic::Exchange(blackAttacker.pieceCounts[m->endPlace], whiteAttacker.pieceCounts[m->endPlace], m->endPlace, beginPiece, endPiece, m->promotionPiece)
                : MoveLogic::Exchange(whiteAttacker.pieceCounts[m->endPlace], blackAttacker.pieceCounts[m->endPlace], m->endPlace, beginPiece, endPiece, m->promotionPiece);

            if (exch > 0)
            {
                stage1List.moves[stage1List.count++] = m;
            }
            else if (exch == 0)
            {
                // Search ONLY if destination is the destination square of the immediately previous move
                if (prevMove.endPlace >= 0 && m->endPlace == prevMove.endPlace)
                {
                    stage1List.moves[stage1List.count++] = m;
                }
                else
                {
                    delete m;
                }
            }
            else
            {
                // Discard capture with Exchange < 0 completely
                delete m;
            }
        }
        else
        {
            // Quiet checking candidate
            DeferredMove dm;
            dm.templateMove = m;
            dm.endPiece = 0;
            deferredMoves[deferredCount++] = dm;
        }
    }

    // Free deferred heap moves and reset template pointers to null since we recreate them in Stage 2 if needed
    // Or simpler: let deferred moves keep the allocated Move object, or materialize on demand.
    // Wait! Since fullList already allocated them via MoveGenerator, to AVOID holding heap or to do proper 2-stage:
    // Notice MoveGenerator can be called, OR we score & sort only stage1 moves!
    // But requirement: "Avoid fully allocating, Exchange scoring and sorting moves that are unlikely to be reached before beta cutoff."
    // Let's ensure Stage 1 scores & sorts ONLY stage1 moves:
    ScoreAndSortMoves(thisBoard, stage1List, depth, depthGone, whiteAttacker, blackAttacker);

    return stage1List;
}

MoveList MoveLogic::MaterializeStage2(Board &thisBoard, int depth, int depthGone, const DeferredMove* deferredMoves, int deferredCount)
{
    MoveList stage2List;
    for (int i = 0; i < deferredCount; ++i)
    {
        stage2List.moves[stage2List.count++] = const_cast<Move*>(deferredMoves[i].templateMove);
    }

    AttackerState whiteAttacker = SetWhiteAttacker(thisBoard);
    AttackerState blackAttacker = SetBlackAttacker(thisBoard);
    ScoreAndSortMoves(thisBoard, stage2List, depth, depthGone, whiteAttacker, blackAttacker);

    return stage2List;
}
// NOTE: You must also replace all Moves->push_back and ComplicatedMoves->push_back in the body with the array logic as described above.
// The rest of the function logic remains the same, just replace vector operations with array operations.

AttackerState MoveLogic::SetWhiteAttacker(Board &thisBoard)
{
    AttackerState whiteAttacker;
    long long whitePieces = thisBoard.whitePieces;
    long long blackPieces = thisBoard.blackPieces;
    int *mainBoard = thisBoard.mainBoard;
    long long wholeBoard = whitePieces | blackPieces;

    for (int piece = 6; piece > 0; piece--)
    {
        switch (piece)
        {
        case 6:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                int endPlace = piecePosition + 7;
                if (PieceMoves::WhiteKingMoves[piecePosition][0] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition + 8;
                if (PieceMoves::WhiteKingMoves[piecePosition][2] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition + 9;
                if (PieceMoves::WhiteKingMoves[piecePosition][4] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition + 1;
                if (PieceMoves::WhiteKingMoves[piecePosition][6] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 7;
                if (PieceMoves::WhiteKingMoves[piecePosition][8] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 8;
                if (PieceMoves::WhiteKingMoves[piecePosition][10] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 9;
                if (PieceMoves::WhiteKingMoves[piecePosition][12] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 1;
                if (PieceMoves::WhiteKingMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
            }
            break;
        case 5:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][8][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][10][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][12][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][14][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;
        case 4:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;

        case 3:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;

        case 2:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                int endPlace = piecePosition + 17;
                if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 10;
                if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 15;
                if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 6;
                if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 10;
                if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 17;
                if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 15;
                if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 6;
                if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
            }
            break;

        case 1:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                if (PieceMoves::WhitePawnMoves[piecePosition][8] != nullptr || PieceMoves::WhitePawnMoves[piecePosition][9] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[piecePosition + 7], 1);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 7]];
                    whiteAttacker.orderingScores[piecePosition + 7] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 7]];
                }
                if (PieceMoves::WhitePawnMoves[piecePosition][13] != nullptr || PieceMoves::WhitePawnMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[piecePosition + 9], 1);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 9]];
                    whiteAttacker.orderingScores[piecePosition + 9] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 9]];
                }
            }
            break;
        }
    }
    return whiteAttacker;
}

AttackerState MoveLogic::SetBlackAttacker(Board &thisBoard)
{
    AttackerState blackAttacker;
    long long whitePieces = thisBoard.whitePieces;
    long long blackPieces = thisBoard.blackPieces;
    int *mainBoard = thisBoard.mainBoard;
    long long wholeBoard = whitePieces | blackPieces;

    for (int piece = 14; piece > 8; piece--)
    {
        switch (piece)
        {
        case 14:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                int endPlace = piecePosition + 7;
                if (PieceMoves::BlackKingMoves[piecePosition][0] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 8;
                if (PieceMoves::BlackKingMoves[piecePosition][2] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 9;
                if (PieceMoves::BlackKingMoves[piecePosition][4] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 1;
                if (PieceMoves::BlackKingMoves[piecePosition][6] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 7;
                if (PieceMoves::BlackKingMoves[piecePosition][8] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 8;
                if (PieceMoves::BlackKingMoves[piecePosition][10] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 9;
                if (PieceMoves::BlackKingMoves[piecePosition][12] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 1;
                if (PieceMoves::BlackKingMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
            }
            break;
        case 13:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][8][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][10][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][12][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][14][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;
        case 12:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;
        case 11:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;
        case 10:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                int endPlace = piecePosition + 17;
                if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 10;
                if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 15;
                if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 6;
                if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 10;
                if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 17;
                if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 15;
                if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 6;
                if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
            }
            break;
        case 9:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                if (PieceMoves::BlackPawnMoves[piecePosition][8] != nullptr || PieceMoves::BlackPawnMoves[piecePosition][9] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[piecePosition - 7], 1);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 7]];
                    blackAttacker.orderingScores[piecePosition - 7] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 7]];
                }
                if (PieceMoves::BlackPawnMoves[piecePosition][13] != nullptr || PieceMoves::BlackPawnMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[piecePosition - 9], 1);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 9]];
                    blackAttacker.orderingScores[piecePosition - 9] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 9]];
                }
            }
            break;
        }
    }
    return blackAttacker;
}

Move *MoveLogic::MoveCopy(Move *move)
{
    Move *newMove = new Move();
    newMove->beginPlace = move->beginPlace;
    newMove->CastleFlag = move->CastleFlag;
    newMove->endPlace = move->endPlace;
    newMove->promotionPiece = move->promotionPiece;
    newMove->PublicFlag = move->PublicFlag;
    newMove->unpassentPlace = move->unpassentPlace;
    newMove->moveCount = move->moveCount;
    return newMove;
}

int MoveLogic::Exchange(std::uint32_t attacker, std::uint32_t defender, int attackPlace, int beginPiece, int endPiece, int promotionPiece)
{
    const std::uint64_t exchangeHash =
        MakeExchangeKey(attacker, defender, beginPiece, endPiece, promotionPiece);
    endPiece = NormalizeExchangePiece(endPiece);
    std::optional<int> exchangeSavedValue;
    exchangeSavedValue = ExchangeCache.getFromCache(exchangeHash);
    if (exchangeSavedValue.has_value())
    {
        return exchangeSavedValue.value();
    }
    else
    {
        if (endPiece == 6)
        {
            return 200;
        }
        int beginPieceTemp = beginPiece;
        double exchangeValue = 0;
        std::uint32_t attackerTemp = attacker;
        std::uint32_t defenderTemp = defender;
        if (promotionPiece != 0)
        {
            exchangeValue = pieceValue[promotionPiece] - 1;
        }
        exchangeValue += pieceValue[endPiece];
        double attackList[30]; int attackListCount = 0;
        double defendList[30]; int defendListCount = 0;
        attackList[attackListCount++] = exchangeValue;
        bool attackerRemove = false;
        while (true)
        {
            if (defenderTemp == 0)
            {
                defendList[defendListCount++] = exchangeValue;
                break;
            }
            else if (IsSoleAttacker(defenderTemp, 6) && attackerTemp > 0 &&
                     !IsSoleAttacker(attackerTemp, beginPiece))
            {
                defendList[defendListCount++] = exchangeValue;
                break;
            }
            else
            {
                endPiece = PopLeastValuableAttacker(defenderTemp);
            }
            exchangeValue -= pieceValue[beginPiece];
            defendList[defendListCount++] = exchangeValue;
            if (attackerTemp == 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else if (IsSoleAttacker(attackerTemp, 6) && defenderTemp > 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else
            {
                beginPiece = PopLeastValuableAttacker(attackerTemp);
                if (!attackerRemove && beginPiece == beginPieceTemp)
                {
                    attackerRemove = true;
                    if (attackerTemp == 0)
                    {
                        attackList[attackListCount++] = exchangeValue;
                        break;
                    }
                    else if (IsSoleAttacker(attackerTemp, 6) && defenderTemp > 0)
                    {
                        attackList[attackListCount++] = exchangeValue;
                        break;
                    }
                    else
                    {
                        beginPiece = PopLeastValuableAttacker(attackerTemp);
                    }
                }
                exchangeValue += pieceValue[endPiece];
                attackList[attackListCount++] = exchangeValue;
            }
        }
        int attackExchangePlace = 0;
        int defendExchangePlace = 0;
        double attackValue = 1000;
        double defendValue = -1000;

        for (int counter = 0; counter < attackListCount; ++counter)
        {
            if (attackList[counter] < attackValue)
            {
                attackExchangePlace = counter;
                attackValue = attackList[counter];
            }
        }

        for (int counter = 0; counter < defendListCount; ++counter)
        {
            if (defendList[counter] > defendValue)
            {
                defendExchangePlace = counter;
                defendValue = defendList[counter];
            }
        }

        int exchangeValueTemp;
        if (defendExchangePlace == attackExchangePlace)
        {
            exchangeValueTemp = static_cast<int>(attackList[attackExchangePlace] * 100);
        }
        else if (defendExchangePlace > attackExchangePlace)
        {
            exchangeValueTemp = static_cast<int>(attackList[attackExchangePlace] * 100);
        }
        else
        {
            exchangeValueTemp = static_cast<int>(defendList[defendExchangePlace] * 100);
        }

        ExchangeCache.addToCache(exchangeHash, exchangeValueTemp);
        return exchangeValueTemp;
    }
}

int MoveLogic::ExchangeWithoutBeginPiece(std::uint32_t attacker, std::uint32_t defender, int attackPlace, int beginPiece, int endPiece, int promotionPiece)
{
    const std::uint64_t exchangeHash =
        MakeExchangeKey(attacker, defender, beginPiece, endPiece, promotionPiece);
    endPiece = NormalizeExchangePiece(endPiece);
    std::optional<int> exchangeSavedValue;
    exchangeSavedValue = ExchangeCacheWithoutBeginPiece.getFromCache(exchangeHash);
    if (exchangeSavedValue.has_value())
    {
        return exchangeSavedValue.value();
    }
    else
    {
        if (endPiece == 6)
        {
            return 200;
        }

        double exchangeValue = 0;
        std::uint32_t attackerTemp = attacker;
        std::uint32_t defenderTemp = defender;
        if (promotionPiece != 0)
        {
            exchangeValue = pieceValue[promotionPiece] - 1;
        }
        exchangeValue += pieceValue[endPiece];
        double attackList[30]; int attackListCount = 0;
        double defendList[30]; int defendListCount = 0;
        attackList[attackListCount++] = exchangeValue;
        while (true)
        {
            if (defenderTemp == 0)
            {
                defendList[defendListCount++] = exchangeValue;
                break;
            }
            else if (IsSoleAttacker(defenderTemp, 6) && attackerTemp > 0)
            {
                defendList[defendListCount++] = exchangeValue;
                break;
            }
            else
            {
                endPiece = PopLeastValuableAttacker(defenderTemp);
            }
            exchangeValue -= pieceValue[beginPiece];
            defendList[defendListCount++] = exchangeValue;

            if (attackerTemp == 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else if (IsSoleAttacker(attackerTemp, 6) && defenderTemp > 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else
            {
                beginPiece = PopLeastValuableAttacker(attackerTemp);
                exchangeValue += pieceValue[endPiece];
                attackList[attackListCount++] = exchangeValue;
            }
        }
        int attackExchangePlace = 0;
        int defendExchangePlace = 0;
        double attackValue = 1000;
        double defendValue = -1000;

        for (int counter = 0; counter < attackListCount; ++counter)
        {
            if (attackList[counter] < attackValue)
            {
                attackExchangePlace = counter;
                attackValue = attackList[counter];
            }
        }

        for (int counter = 0; counter < defendListCount; ++counter)
        {
            if (defendList[counter] > defendValue)
            {
                defendExchangePlace = counter;
                defendValue = defendList[counter];
            }
        }

        int exchangeValueTemp;
        if (defendExchangePlace == attackExchangePlace)
        {
            exchangeValueTemp = static_cast<int>(attackList[attackExchangePlace] * 100);
        }
        else if (defendExchangePlace > attackExchangePlace)
        {
            exchangeValueTemp = static_cast<int>(attackList[attackExchangePlace] * 100);
        }
        else
        {
            exchangeValueTemp = static_cast<int>(defendList[defendExchangePlace] * 100);
        }

        ExchangeCacheWithoutBeginPiece.addToCache(exchangeHash, exchangeValueTemp);
        return exchangeValueTemp;
    }
}

bool MoveLogic::Same(Move &move2, Move &move3, Move &move4, Move &move)
{
    if (move2.beginPlace == move4.endPlace &&
        move3.beginPlace == move.endPlace &&
        move2.endPiece == move4.endPiece &&
        move3.endPiece == move.endPiece &&
        move2.endPlace == move4.endPlace &&
        move3.endPlace == move.endPlace)
    {
        Search::moveCount++;
        return true;
    }
    return false;
}

std::size_t MoveLogic::ExchangeCacheSize()
{
    return ExchangeCache.size();
}

std::size_t MoveLogic::ExchangeWithoutBeginPieceCacheSize()
{
    return ExchangeCacheWithoutBeginPiece.size();
}

ExchangeCacheStatistics MoveLogic::ExchangeCacheStats()
{
    return ExchangeCache.statistics();
}

ExchangeCacheStatistics MoveLogic::ExchangeWithoutBeginPieceCacheStats()
{
    return ExchangeCacheWithoutBeginPiece.statistics();
}

void MoveLogic::ResetExchangeCacheStats()
{
    ExchangeCache.resetStatistics();
    ExchangeCacheWithoutBeginPiece.resetStatistics();
}

bool MoveLogic::ResizeExchangeCache(std::size_t capacityBytes)
{
    return ExchangeCache.resize(capacityBytes);
}

bool MoveLogic::ResizeExchangeWithoutBeginPieceCache(std::size_t capacityBytes)
{
    return ExchangeCacheWithoutBeginPiece.resize(capacityBytes);
}

std::size_t MoveLogic::ExchangeCacheCapacityBytes()
{
    return ExchangeCache.capacityBytes();
}

std::size_t MoveLogic::ExchangeWithoutBeginPieceCacheCapacityBytes()
{
    return ExchangeCacheWithoutBeginPiece.capacityBytes();
}

#if HOWL_CORRECTNESS_TESTING
void MoveLogic::SetExchangeCacheAllocationFailureThresholdForTesting(
    std::size_t capacityBytes)
{
    ExchangeChessCache::SetAllocationFailureThresholdForTesting(capacityBytes);
}
#endif

void MoveLogic::Cleanup()
{
    // Clean up static cache objects
    ExchangeCache.clear();
    ExchangeCacheWithoutBeginPiece.clear();

    // Note: The main memory allocations in this class are temporary:
    // 1. whiteAttacker and blackAttacker arrays in SetWhiteAttacker/SetBlackAttacker
    //    - these are properly cleaned up in MoveGenerator function
    // 2. Move objects created in MoveCopy - these are managed by the caller
    // 3. The static cache objects above have been cleared
}
