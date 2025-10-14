#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "BoardLogic.h"
#include "MyList.h"

bool BoardLogic::UnderAttack(Board &thisBoard, int position, bool attackerSide)
{
    long long wholeBoard = thisBoard.whitePieces | thisBoard.blackPieces;
    if (!attackerSide)
    {
        for (int piece = 1; piece < 7; piece++)
        {
            switch (piece)
            {
            case 1:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::WhitePawnAttackPlaces[piecePosition] & Option::PowerTwo[position]) != 0)
                    {
                        return true;
                    }
                }
                break;
            case 2:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::KnightAttackPlaces[piecePosition] & Option::PowerTwo[position]) != 0)
                    {
                        return true;
                    }
                }
                break;
            case 3:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::BishopAttack[piecePosition][position] & wholeBoard) == Option::PowerTwo[position])
                    {
                        return true;
                    }
                }
                break;
            case 4:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::RookAttack[piecePosition][position] & wholeBoard) == Option::PowerTwo[position])
                    {
                        return true;
                    }
                }
                break;
            case 5:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::QueenAttack[piecePosition][position] & wholeBoard) == Option::PowerTwo[position])
                    {
                        return true;
                    }
                }
                break;
            case 6:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::KingAttackPlaces[piecePosition] & Option::PowerTwo[position]) != 0)
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
            switch (piece)
            {
            case 9:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::BlackPawnAttackPlaces[piecePosition] & Option::PowerTwo[position]) != 0)
                    {
                        return true;
                    }
                }
                break;
            case 10:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::KnightAttackPlaces[piecePosition] & Option::PowerTwo[position]) != 0)
                    {
                        return true;
                    }
                }
                break;
            case 11:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::BishopAttack[piecePosition][position] & wholeBoard) == Option::PowerTwo[position])
                    {
                        return true;
                    }
                }
                break;
            case 12:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::RookAttack[piecePosition][position] & wholeBoard) == Option::PowerTwo[position])
                    {
                        return true;
                    }
                }
                break;
            case 13:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::QueenAttack[piecePosition][position] & wholeBoard) == Option::PowerTwo[position])
                    {
                        return true;
                    }
                }
                break;
            case 14:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    if ((AttackPlaces::KingAttackPlaces[piecePosition] & Option::PowerTwo[position]) != 0)
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