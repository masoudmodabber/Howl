#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "BoardLogic.h"
#include "MyList.h"

bool BoardLogic::UnderAttack(Board &thisBoard, int position, bool attackerSide)
{
    long long wholeBoard = thisBoard.whitePieces | thisBoard.blackPieces;
    const long long posBit = Option::PowerTwo[position];
    if (!attackerSide)
    {
        for (int piece = 1; piece < 7; piece++)
        {
            if (thisBoard.pieces[piece].count == 0) continue;
            switch (piece)
            {
            case 1:
                if ((AttackPlaces::BlackPawnAttackPlaces[position] & thisBoard.whitePawns) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::WhitePawnAttackPlaces[piecePosition] & posBit) != 0)
                    {
                        return true;
                    }
                }
                break;
            case 2:
                if ((AttackPlaces::KnightAttackPlaces[position] & thisBoard.whitePieces) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::KnightAttackPlaces[piecePosition] & posBit) != 0)
                    {
                        return true;
                    }
                }
                break;
            case 3:
                if ((AttackPlaces::BishopPseudoAttacks[position] & thisBoard.whitePieces) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::BishopAttack[piecePosition][position] & wholeBoard) == posBit)
                    {
                        return true;
                    }
                }
                break;
            case 4:
                if ((AttackPlaces::RookPseudoAttacks[position] & thisBoard.whitePieces) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::RookAttack[piecePosition][position] & wholeBoard) == posBit)
                    {
                        return true;
                    }
                }
                break;
            case 5:
                if ((AttackPlaces::QueenPseudoAttacks[position] & thisBoard.whitePieces) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::QueenAttack[piecePosition][position] & wholeBoard) == posBit)
                    {
                        return true;
                    }
                }
                break;
            case 6:
                if ((AttackPlaces::KingAttackPlaces[position] & thisBoard.whitePieces) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::KingAttackPlaces[piecePosition] & posBit) != 0)
                    {
                        return true;
                    }
                }
                break;
            }
        }
    }
    // else part
    else
    {
        for (int piece = 9; piece < 15; piece++)
        {
            if (thisBoard.pieces[piece].count == 0) continue;
            switch (piece)
            {
            case 9:
                if ((AttackPlaces::WhitePawnAttackPlaces[position] & thisBoard.blackPawns) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::BlackPawnAttackPlaces[piecePosition] & posBit) != 0)
                    {
                        return true;
                    }
                }
                break;
            case 10:
                if ((AttackPlaces::KnightAttackPlaces[position] & thisBoard.blackPieces) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::KnightAttackPlaces[piecePosition] & posBit) != 0)
                    {
                        return true;
                    }
                }
                break;
            case 11:
                if ((AttackPlaces::BishopPseudoAttacks[position] & thisBoard.blackPieces) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::BishopAttack[piecePosition][position] & wholeBoard) == posBit)
                    {
                        return true;
                    }
                }
                break;
            case 12:
                if ((AttackPlaces::RookPseudoAttacks[position] & thisBoard.blackPieces) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::RookAttack[piecePosition][position] & wholeBoard) == posBit)
                    {
                        return true;
                    }
                }
                break;
            case 13:
                if ((AttackPlaces::QueenPseudoAttacks[position] & thisBoard.blackPieces) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::QueenAttack[piecePosition][position] & wholeBoard) == posBit)
                    {
                        return true;
                    }
                }
                break;
            case 14:
                if ((AttackPlaces::KingAttackPlaces[position] & thisBoard.blackPieces) == 0) break;
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::KingAttackPlaces[piecePosition] & posBit) != 0)
                    {
                        return true;
                    }
                }
                break;
            }
        }
    }
    return false;
}