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
#include <algorithm>

double MoveLogic::pieceValue[15];
int MoveLogic::pieceMoveStack[15];
ChessCache MoveLogic::ExchangeCache;
ChessCache MoveLogic::ExchangeCacheWithoutBeginPiece;

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

MoveList MoveLogic::MoveGenerator(Board &thisBoard, int depth, int depthGone)
{
    MoveList moveList;
    Move* complicatedMoves[80];
    int complicatedCount = 0;
    if (Search::moveCount == 14962)
    {
        int x = 1;
    }
    int **whiteAttacker = SetWhiteAttacker(thisBoard);
    int **blackAttacker = SetBlackAttacker(thisBoard);
    long long whitePieces = thisBoard.whitePieces;
    long long blackPieces = thisBoard.blackPieces;
    int *mainBoard = thisBoard.mainBoard;
    long long wholeBoard = whitePieces | blackPieces;

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
                    if (PieceMoves::WhitePawnMoves[piecePosition][0] != nullptr)
                    {
                        Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][0]);
                        if ((PieceMoves::pawnTwoMove[piecePosition] & wholeBoard) == 0)
                        {
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(whiteAttacker[0][newMove->endPlace], blackAttacker[0][newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                            complicatedMoves[complicatedCount++] = newMove;
                        }
                        else
                        {
                            delete newMove;
                            newMove = nullptr;
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][1] != nullptr && (Option::PowerTwo[piecePosition + 8] & wholeBoard) == 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][1]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                        newMove->value = ExchangeWithoutBeginPiece(whiteAttacker[0][newMove->endPlace], blackAttacker[0][newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                        complicatedMoves[complicatedCount++] = newMove;
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][2] != nullptr && (Option::PowerTwo[piecePosition + 8] & wholeBoard) == 0)
                    {
                        for (int i = 2; i <= 5; i++)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhitePawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(whiteAttacker[0][newMove->endPlace], blackAttacker[0][newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 5 - (i - 2));
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
                    int endPlace = piecePosition + 17;
                    if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][0]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][2]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][4]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][6]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][8]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][10]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][12]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][14]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                    int endPlace;
                    endPlace = piecePosition + 7;
                    if (PieceMoves::WhiteKingMoves[piecePosition][0] != nullptr && blackAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][0]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 8;
                    if (PieceMoves::WhiteKingMoves[piecePosition][2] != nullptr && blackAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][2]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 9;
                    if (PieceMoves::WhiteKingMoves[piecePosition][4] != nullptr && blackAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][4]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 1;
                    if (PieceMoves::WhiteKingMoves[piecePosition][6] != nullptr && blackAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][6]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 7;
                    if (PieceMoves::WhiteKingMoves[piecePosition][8] != nullptr && blackAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][8]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 8;
                    if (PieceMoves::WhiteKingMoves[piecePosition][10] != nullptr && blackAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][10]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 9;
                    if (PieceMoves::WhiteKingMoves[piecePosition][12] != nullptr && blackAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][12]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 1;
                    if (PieceMoves::WhiteKingMoves[piecePosition][14] != nullptr && blackAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][14]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    if (thisBoard.whiteSmallCastle && blackAttacker[0][4] == 0 && blackAttacker[0][5] == 0 && blackAttacker[0][6] == 0 && mainBoard[5] == 0 && mainBoard[6] == 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::WhiteKingMoves[piecePosition][16]);
                        newMove->value = 50;
                        complicatedMoves[complicatedCount++] = newMove;
                    }
                    if (thisBoard.whiteBigCastle && blackAttacker[0][4] == 0 && blackAttacker[0][3] == 0 && blackAttacker[0][2] == 0 && mainBoard[3] == 0 && mainBoard[2] == 0 && mainBoard[1] == 0)
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
                    int exchangeInPlace = -ExchangeWithoutBeginPiece(blackAttacker[0][piecePosition], whiteAttacker[0][piecePosition], piecePosition, piece - 8, 0, 0);
                    if (PieceMoves::BlackPawnMoves[piecePosition][0] != nullptr)
                    {
                        if ((PieceMoves::pawnTwoMove[piecePosition] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][0]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(blackAttacker[0][newMove->endPlace], whiteAttacker[0][newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                            complicatedMoves[complicatedCount++] = newMove;
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][1] != nullptr && (Option::PowerTwo[piecePosition - 8] & wholeBoard) == 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][1]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                        newMove->value = ExchangeWithoutBeginPiece(blackAttacker[0][newMove->endPlace], whiteAttacker[0][newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                        complicatedMoves[complicatedCount++] = newMove;
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][2] != nullptr && (Option::PowerTwo[piecePosition - 8] & wholeBoard) == 0)
                    {
                        for (int i = 2; i <= 5; i++)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackPawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(blackAttacker[0][newMove->endPlace], whiteAttacker[0][newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 5 - (i - 2));
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
                    int endPlace = piecePosition + 17;
                    if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][0]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][2]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][4]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][6]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][8]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][10]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][12]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                            Move *newMove = MoveCopy(PieceMoves::KnightMoves[piecePosition][14]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
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
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BishopMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::RookMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = MoveCopy(PieceMoves::QueenMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                            moveList.moves[moveList.count++] = newMove;
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
                    int endPlace;
                    endPlace = piecePosition + 7;
                    if (PieceMoves::BlackKingMoves[piecePosition][0] != nullptr && whiteAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][0]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 8;
                    if (PieceMoves::BlackKingMoves[piecePosition][2] != nullptr && whiteAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][2]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 9;
                    if (PieceMoves::BlackKingMoves[piecePosition][4] != nullptr && whiteAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][4]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition + 1;
                    if (PieceMoves::BlackKingMoves[piecePosition][6] != nullptr && whiteAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][6]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 7;
                    if (PieceMoves::BlackKingMoves[piecePosition][8] != nullptr && whiteAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][8]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 8;
                    if (PieceMoves::BlackKingMoves[piecePosition][10] != nullptr && whiteAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][10]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 9;
                    if (PieceMoves::BlackKingMoves[piecePosition][12] != nullptr && whiteAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][12]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    endPlace = piecePosition - 1;
                    if (PieceMoves::BlackKingMoves[piecePosition][14] != nullptr && whiteAttacker[0][endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][14]);
                            moveList.moves[moveList.count++] = newMove;
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            moveList.moves[moveList.count++] = newMove;
                        }
                    }
                    if (thisBoard.blackSmallCastle && whiteAttacker[0][60] == 0 && whiteAttacker[0][61] == 0 && whiteAttacker[0][62] == 0 && mainBoard[61] == 0 && mainBoard[62] == 0)
                    {
                        Move *newMove = MoveCopy(PieceMoves::BlackKingMoves[piecePosition][16]);
                        newMove->value = 25;
                        complicatedMoves[complicatedCount++] = newMove;
                    }
                    if (thisBoard.blackBigCastle && whiteAttacker[0][60] == 0 && whiteAttacker[0][59] == 0 && whiteAttacker[0][58] == 0 && mainBoard[59] == 0 && mainBoard[58] == 0 && mainBoard[57] == 0)
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
    // --- BEGIN REPLACEMENT OF VECTOR USAGE ---
    // Replace all usages of Moves and ComplicatedMoves with moveList and complicatedMoves arrays
    // ...
    // At the end, merge complicatedMoves into moveList
    for (int i = 0; i < complicatedCount; ++i) {
        moveList.moves[moveList.count++] = complicatedMoves[i];
    }
    // --- END REPLACEMENT OF VECTOR USAGE ---

    // Replace for (Move *move : *Moves) with for (int i = 0; i < moveList.count; ++i) { Move* move = moveList.moves[i]; ... }
    int state = 0;
    if (thisBoard.sideToMove)
    {
        for (int i = 0; i < moveList.count; ++i)
        {
            Move* move = moveList.moves[i];
            int beginPiece = mainBoard[move->beginPlace] % 8;
            move->value = Exchange(blackAttacker[0][move->endPlace], whiteAttacker[0][move->endPlace], move->endPlace, beginPiece, move->endPiece, move->promotionPiece);
            move->value += whiteAttacker[1][move->beginPlace] + whiteAttacker[1][move->endPlace];
            move->value += Option::MoveOrderingValueBlack[state][beginPiece][move->endPlace];
        }
    }
    else
    {
        for (int i = 0; i < moveList.count; ++i)
        {
            Move* move = moveList.moves[i];
            int beginPiece = mainBoard[move->beginPlace];
            move->value = Exchange(whiteAttacker[0][move->endPlace], blackAttacker[0][move->endPlace], move->endPlace, beginPiece, move->endPiece % 8, move->promotionPiece);
            move->value += blackAttacker[1][move->beginPlace] + blackAttacker[1][move->endPlace];
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
    // Sort moves by value descending
    std::sort(moveList.moves, moveList.moves + moveList.count, [](const Move *a, const Move *b)
              { return b->value < a->value; });

    for (int i = 0; i < 2; ++i)
    {
        delete[] whiteAttacker[i];
        delete[] blackAttacker[i];
    }
    delete[] whiteAttacker;
    delete[] blackAttacker;

    return moveList;
}
// NOTE: You must also replace all Moves->push_back and ComplicatedMoves->push_back in the body with the array logic as described above.
// The rest of the function logic remains the same, just replace vector operations with array operations.

int **MoveLogic::SetWhiteAttacker(Board &thisBoard)
{
    int **whiteAttacker = new int *[2];
    for (int i = 0; i < 2; ++i)
    {
        whiteAttacker[i] = new int[64]();
    }
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
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 6;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition + 8;
                if (PieceMoves::WhiteKingMoves[piecePosition][2] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 6;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition + 9;
                if (PieceMoves::WhiteKingMoves[piecePosition][4] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 6;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition + 1;
                if (PieceMoves::WhiteKingMoves[piecePosition][6] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 6;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 7;
                if (PieceMoves::WhiteKingMoves[piecePosition][8] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 6;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 8;
                if (PieceMoves::WhiteKingMoves[piecePosition][10] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 6;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 9;
                if (PieceMoves::WhiteKingMoves[piecePosition][12] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 6;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 1;
                if (PieceMoves::WhiteKingMoves[piecePosition][14] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 6;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
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
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][8][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][10][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][12][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][14][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 5;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
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
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 4;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 4;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 4;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 4;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 4;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 4;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 4;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 4;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
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
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 3;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 3;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 3;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 3;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 3;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 3;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 3;
                    }
                    else
                    {
                        whiteAttacker[0][endPos] = whiteAttacker[0][endPos] * 8 + 3;
                        whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
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
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 2;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 10;
                if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 2;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 15;
                if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 2;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 6;
                if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 2;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 10;
                if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 2;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 17;
                if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 2;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 15;
                if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 2;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 6;
                if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                {
                    whiteAttacker[0][endPlace] = whiteAttacker[0][endPlace] * 8 + 2;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
            }
            break;

        case 1:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                if (PieceMoves::WhitePawnMoves[piecePosition][8] != nullptr)
                {
                    whiteAttacker[0][piecePosition + 7] = whiteAttacker[0][piecePosition + 7] * 8 + 1;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 7]];
                    whiteAttacker[1][piecePosition + 7] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 7]];
                }
                if (PieceMoves::WhitePawnMoves[piecePosition][13] != nullptr)
                {
                    whiteAttacker[0][piecePosition + 9] = whiteAttacker[0][piecePosition + 9] * 8 + 1;
                    whiteAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 9]];
                    whiteAttacker[1][piecePosition + 9] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 9]];
                }
            }
            break;
        }
    }
    return whiteAttacker;
}

int **MoveLogic::SetBlackAttacker(Board &thisBoard)
{
    int **blackAttacker = new int *[2];
    for (int i = 0; i < 2; ++i)
    {
        blackAttacker[i] = new int[64];
    }
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
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 6;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 8;
                if (PieceMoves::BlackKingMoves[piecePosition][2] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 6;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 9;
                if (PieceMoves::BlackKingMoves[piecePosition][4] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 6;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 1;
                if (PieceMoves::BlackKingMoves[piecePosition][6] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 6;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 7;
                if (PieceMoves::BlackKingMoves[piecePosition][8] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 6;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 8;
                if (PieceMoves::BlackKingMoves[piecePosition][10] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 6;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 9;
                if (PieceMoves::BlackKingMoves[piecePosition][12] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 6;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 1;
                if (PieceMoves::BlackKingMoves[piecePosition][14] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 6;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
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
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][8][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][10][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][12][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][14][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 5;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
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
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 4;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 4;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 4;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 4;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 4;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 4;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 4;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 4;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
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
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 3;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 3;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 3;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 3;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 3;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 3;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 3;
                    }
                    else
                    {
                        blackAttacker[0][endPos] = blackAttacker[0][endPos] * 8 + 3;
                        blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker[1][endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
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
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 2;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 10;
                if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 2;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 15;
                if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 2;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 6;
                if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 2;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 10;
                if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 2;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 17;
                if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 2;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 15;
                if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 2;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 6;
                if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                {
                    blackAttacker[0][endPlace] = blackAttacker[0][endPlace] * 8 + 2;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker[1][endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
            }
            break;
        case 9:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                if (PieceMoves::BlackPawnMoves[piecePosition][8] != nullptr)
                {
                    blackAttacker[0][piecePosition - 7] = blackAttacker[0][piecePosition - 7] * 8 + 1;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 7]];
                    blackAttacker[1][piecePosition - 7] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 7]];
                }
                if (PieceMoves::BlackPawnMoves[piecePosition][13] != nullptr)
                {
                    blackAttacker[0][piecePosition - 9] = blackAttacker[0][piecePosition - 9] * 8 + 1;
                    blackAttacker[1][piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 9]];
                    blackAttacker[1][piecePosition - 9] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 9]];
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
    newMove->moveCount = move->moveCount;
    return newMove;
}

int MoveLogic::Exchange(int attacker, int defender, int attackPlace, int beginPiece, int endPiece, int promotionPiece)
{
    long long exchangeHash;
    int count = 3;
    if (endPiece > 8)
    {
        endPiece -= 8;
    }
    while (Option::PowerTwo[count] < defender)
    {
        count += 3;
    }
    exchangeHash = defender + Option::PowerTwo[count] * 7 + Option::PowerTwo[count + 3] * attacker;
    int countAttack = 3;
    while (Option::PowerTwo[countAttack] < attacker)
    {
        countAttack += 3;
    }
    exchangeHash += Option::PowerTwo[count + countAttack] * 7 + Option::PowerTwo[count + countAttack] * endPiece;
    exchangeHash += Option::PowerTwo[count + countAttack + 3] * 7 + Option::PowerTwo[count + countAttack + 6] * beginPiece;
    std::optional<int> exchangeSavedValue = ExchangeCache.getFromCache(exchangeHash);
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
        int attackerTemp = attacker;
        int defenderTemp = defender;
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
            else if (defenderTemp == 6 && attackerTemp > 0 && attackerTemp != beginPiece)
            {
                defendList[defendListCount++] = exchangeValue;
                break;
            }
            else
            {
                endPiece = defenderTemp % 8;
                defenderTemp = (defenderTemp - endPiece) / 8;
            }
            exchangeValue -= pieceValue[beginPiece];
            defendList[defendListCount++] = exchangeValue;
            if (attackerTemp == 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else if (attackerTemp == 6 && defenderTemp > 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else
            {
                beginPiece = attackerTemp % 8;
                attackerTemp = (attackerTemp - beginPiece) / 8;
                if (!attackerRemove && beginPiece == beginPieceTemp)
                {
                    attackerRemove = true;
                    if (attackerTemp == 0)
                    {
                        attackList[attackListCount++] = exchangeValue;
                        break;
                    }
                    else if (attackerTemp == 6 && defenderTemp > 0)
                    {
                        attackList[attackListCount++] = exchangeValue;
                        break;
                    }
                    else
                    {
                        beginPiece = attackerTemp % 8;
                        attackerTemp = (attackerTemp - beginPiece) / 8;
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

int MoveLogic::ExchangeWithoutBeginPiece(int attacker, int defender, int attackPlace, int beginPiece, int endPiece, int promotionPiece)
{
    long long exchangeHash;
    int count = 3;
    if (endPiece > 8)
    {
        endPiece -= 8;
    }
    while (Option::PowerTwo[count] < defender)
    {
        count += 3;
    }
    exchangeHash = defender + Option::PowerTwo[count] * 7 + Option::PowerTwo[count + 3] * attacker;
    int countAttack = 3;
    while (Option::PowerTwo[countAttack] < attacker)
    {
        countAttack += 3;
    }
    exchangeHash += Option::PowerTwo[count + countAttack] * 7 + Option::PowerTwo[count + countAttack] * endPiece;
    exchangeHash += Option::PowerTwo[count + countAttack + 3] * 7 + Option::PowerTwo[count + countAttack + 6] * beginPiece;
    std::optional<int> exchangeSavedValue = ExchangeCache.getFromCache(exchangeHash);
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
        int attackerTemp = attacker;
        int defenderTemp = defender;
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
            else if (defenderTemp == 6 && attackerTemp > 0)
            {
                defendList[defendListCount++] = exchangeValue;
                break;
            }
            else
            {
                endPiece = defenderTemp % 8;
                defenderTemp = (defenderTemp - endPiece) / 8;
            }
            exchangeValue -= pieceValue[beginPiece];
            defendList[defendListCount++] = exchangeValue;

            if (attackerTemp == 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else if (attackerTemp == 6 && defenderTemp > 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else
            {
                beginPiece = attackerTemp % 8;
                attackerTemp = (attackerTemp - beginPiece) / 8;
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

void MoveLogic::Cleanup()
{
    // Clean up static cache objects
    ExchangeCache.evalCache.clear();
    ExchangeCache.currentMemoryUsage = 0;

    ExchangeCacheWithoutBeginPiece.evalCache.clear();
    ExchangeCacheWithoutBeginPiece.currentMemoryUsage = 0;

    // Note: The main memory allocations in this class are temporary:
    // 1. whiteAttacker and blackAttacker arrays in SetWhiteAttacker/SetBlackAttacker
    //    - these are properly cleaned up in MoveGenerator function
    // 2. Move objects created in MoveCopy - these are managed by the caller
    // 3. The static cache objects above have been cleared
}