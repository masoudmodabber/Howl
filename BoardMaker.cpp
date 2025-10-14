#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "BoardMaker.h"
#include "UCI.h"
#include "MoveLogic.h"
#include "LastFourMoves.h"
#include "Option.h"
#include "BoardInitializer.h"

Board *BoardMaker::MakeInitialBoard(std::string position)
{
    Board *thisBoard;
    int boardArray[130];
    int counter = 0;
    int row = 8;
    int column = 1;
    for (int boardCounter = 0; boardCounter < 130; boardCounter++)
    {
        boardArray[boardCounter] = -1;
    }
    while (position[counter] != ' ')
    {
        switch (position[counter])
        {
        case '1':
            boardArray[(row + 1) * 10 + column] = 0;
            column++;
            break;
        case '2':
            boardArray[(row + 1) * 10 + column] = 0;
            boardArray[(row + 1) * 10 + column + 1] = 0;
            column += 2;
            break;
        case '3':
            boardArray[(row + 1) * 10 + column] = 0;
            boardArray[(row + 1) * 10 + column + 1] = 0;
            boardArray[(row + 1) * 10 + column + 2] = 0;
            column += 3;
            break;
        case '4':
            boardArray[(row + 1) * 10 + column] = 0;
            boardArray[(row + 1) * 10 + column + 1] = 0;
            boardArray[(row + 1) * 10 + column + 2] = 0;
            boardArray[(row + 1) * 10 + column + 3] = 0;
            column += 4;
            break;
        case '5':
            boardArray[(row + 1) * 10 + column] = 0;
            boardArray[(row + 1) * 10 + column + 1] = 0;
            boardArray[(row + 1) * 10 + column + 2] = 0;
            boardArray[(row + 1) * 10 + column + 3] = 0;
            boardArray[(row + 1) * 10 + column + 4] = 0;
            column += 5;
            break;
        case '6':
            boardArray[(row + 1) * 10 + column] = 0;
            boardArray[(row + 1) * 10 + column + 1] = 0;
            boardArray[(row + 1) * 10 + column + 2] = 0;
            boardArray[(row + 1) * 10 + column + 3] = 0;
            boardArray[(row + 1) * 10 + column + 4] = 0;
            boardArray[(row + 1) * 10 + column + 5] = 0;
            column += 6;
            break;
        case '7':
            boardArray[(row + 1) * 10 + column] = 0;
            boardArray[(row + 1) * 10 + column + 1] = 0;
            boardArray[(row + 1) * 10 + column + 2] = 0;
            boardArray[(row + 1) * 10 + column + 3] = 0;
            boardArray[(row + 1) * 10 + column + 4] = 0;
            boardArray[(row + 1) * 10 + column + 5] = 0;
            boardArray[(row + 1) * 10 + column + 6] = 0;
            column += 7;
            break;
        case '8':
            boardArray[(row + 1) * 10 + column] = 0;
            boardArray[(row + 1) * 10 + column + 1] = 0;
            boardArray[(row + 1) * 10 + column + 2] = 0;
            boardArray[(row + 1) * 10 + column + 3] = 0;
            boardArray[(row + 1) * 10 + column + 4] = 0;
            boardArray[(row + 1) * 10 + column + 5] = 0;
            boardArray[(row + 1) * 10 + column + 6] = 0;
            boardArray[(row + 1) * 10 + column + 7] = 0;
            column += 8;
            break;
        case 'p':
            boardArray[(row + 1) * 10 + column] = 9;
            column++;
            break;
        case 'n':
            boardArray[(row + 1) * 10 + column] = 10;
            column++;
            break;
        case 'b':
            boardArray[(row + 1) * 10 + column] = 11;
            column++;
            break;
        case 'r':
            boardArray[(row + 1) * 10 + column] = 12;
            column++;
            break;
        case 'q':
            boardArray[(row + 1) * 10 + column] = 13;
            column++;
            break;
        case 'k':
            boardArray[(row + 1) * 10 + column] = 14;
            column++;
            break;
        case 'P':
            boardArray[(row + 1) * 10 + column] = 1;
            column++;
            break;
        case 'N':
            boardArray[(row + 1) * 10 + column] = 2;
            column++;
            break;
        case 'B':
            boardArray[(row + 1) * 10 + column] = 3;
            column++;
            break;
        case 'R':
            boardArray[(row + 1) * 10 + column] = 4;
            column++;
            break;
        case 'Q':
            boardArray[(row + 1) * 10 + column] = 5;
            column++;
            break;
        case 'K':
            boardArray[(row + 1) * 10 + column] = 6;
            column++;
            break;
        case '/':
            column = 1;
            row--;
            break;
        }
        counter++;
    }
    counter++;
    if (position[counter] == 'w')
    {
        boardArray[120] = 0;
    }
    else
    {
        boardArray[120] = 1;
    }
    counter += 2;
    if (position[counter] == '-')
    {
        boardArray[121] = 0;
        boardArray[122] = 0;
        boardArray[123] = 0;
        boardArray[124] = 0;
        counter++;
    }
    else
    {
        switch (position[counter])
        {
        case 'K':
            boardArray[121] = 1;
            break;
        case 'Q':
            boardArray[122] = 1;
            break;
        case 'k':
            boardArray[123] = 1;
            break;
        case 'q':
            boardArray[124] = 1;
            break;
        }
        counter++;
        while (position[counter] != ' ')
        {
            switch (position[counter])
            {
            case 'K':
                boardArray[121] = 1;
                break;
            case 'Q':
                boardArray[122] = 1;
                break;
            case 'k':
                boardArray[123] = 1;
                break;
            case 'q':
                boardArray[124] = 1;
                break;
            }
            counter++;
        }
    }
    counter++;
    if (position[counter] != '-')
    {
        boardArray[125] = position[counter] - 'a' + 1 + (position[counter + 1] - '1' + 2) * 10;
        counter += 3;
    }
    else
    {
        boardArray[125] = 0;
        counter += 2;
    }
    {
        int temp = 0;
        while (counter < position.length() && position[counter] != ' ')
        {
            temp = temp * 10 + position[counter] - '1' + 1;
            counter++;
        }
        boardArray[126] = temp;
    }
    counter++;
    thisBoard = MakeBoard(boardArray);
    {
        int temp = 0;
        while (counter < position.length() && position[counter] <= '9' && position[counter] >= '0')
        {
            temp = temp * 10 + position[counter] - '1' + 1;
            counter++;
        }
        thisBoard->moveNumber = temp;
    }
    if (counter < position.length())
    {
        LastFourMoves *lastMoves = UCI::MakeMoves(position.substr(counter + 1, position.length() - counter - 1), *thisBoard);
        UCI::move1 = lastMoves->Move1;
        UCI::move2 = lastMoves->Move2;
        UCI::move3 = lastMoves->Move3;
        UCI::move4 = lastMoves->Move4;
        delete lastMoves;
        lastMoves = nullptr;
    }
    else
    {
        Move *move = new Move();
        UCI::move1 = MoveLogic::MoveCopy(move);
        UCI::move2 = MoveLogic::MoveCopy(move);
        UCI::move3 = MoveLogic::MoveCopy(move);
        UCI::move4 = MoveLogic::MoveCopy(move);
        delete move;
        move = nullptr;
    }
    return thisBoard;
}

Board *BoardMaker::MakeBoard(int boardArray[])
{
    Board *madeBoard = new Board();
    int firstofRow = 21;
    for (int counter = firstofRow; counter < firstofRow + 8; counter++)
    {
        madeBoard->mainBoard[counter - firstofRow + (firstofRow - 20) / 10 * 8] = boardArray[counter];
        if (boardArray[counter] != 0)
        {
            madeBoard->pieces[boardArray[counter]].push_back(counter - firstofRow + (firstofRow - 20) / 10 * 8);
            if (boardArray[counter] < 8)
            {
                madeBoard->whitePieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
            else
            {
                madeBoard->blackPieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
        }
    }
    firstofRow += 10;
    for (int counter = firstofRow; counter < firstofRow + 8; counter++)
    {
        madeBoard->mainBoard[counter - firstofRow + (firstofRow - 20) / 10 * 8] = boardArray[counter];
        if (boardArray[counter] != 0)
        {
            madeBoard->pieces[boardArray[counter]].push_back(counter - firstofRow + (firstofRow - 20) / 10 * 8);
            if (boardArray[counter] < 8)
            {
                madeBoard->whitePieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
            else
            {
                madeBoard->blackPieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
        }
    }
    firstofRow += 10;
    for (int counter = firstofRow; counter < firstofRow + 8; counter++)
    {
        madeBoard->mainBoard[counter - firstofRow + (firstofRow - 20) / 10 * 8] = boardArray[counter];
        if (boardArray[counter] != 0)
        {
            madeBoard->pieces[boardArray[counter]].push_back(counter - firstofRow + (firstofRow - 20) / 10 * 8);
            if (boardArray[counter] < 8)
            {
                madeBoard->whitePieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
            else
            {
                madeBoard->blackPieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
        }
    }
    firstofRow += 10;
    for (int counter = firstofRow; counter < firstofRow + 8; counter++)
    {
        madeBoard->mainBoard[counter - firstofRow + (firstofRow - 20) / 10 * 8] = boardArray[counter];
        if (boardArray[counter] != 0)
        {
            madeBoard->pieces[boardArray[counter]].push_back(counter - firstofRow + (firstofRow - 20) / 10 * 8);
            if (boardArray[counter] < 8)
            {
                madeBoard->whitePieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
            else
            {
                madeBoard->blackPieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
        }
    }
    firstofRow += 10;
    for (int counter = firstofRow; counter < firstofRow + 8; counter++)
    {
        madeBoard->mainBoard[counter - firstofRow + (firstofRow - 20) / 10 * 8] = boardArray[counter];
        if (boardArray[counter] != 0)
        {
            madeBoard->pieces[boardArray[counter]].push_back(counter - firstofRow + (firstofRow - 20) / 10 * 8);
            if (boardArray[counter] < 8)
            {
                madeBoard->whitePieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
            else
            {
                madeBoard->blackPieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
        }
    }
    firstofRow += 10;
    for (int counter = firstofRow; counter < firstofRow + 8; counter++)
    {
        madeBoard->mainBoard[counter - firstofRow + (firstofRow - 20) / 10 * 8] = boardArray[counter];
        if (boardArray[counter] != 0)
        {
            madeBoard->pieces[boardArray[counter]].push_back(counter - firstofRow + (firstofRow - 20) / 10 * 8);
            if (boardArray[counter] < 8)
            {
                madeBoard->whitePieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
            else
            {
                madeBoard->blackPieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
        }
    }
    firstofRow += 10;
    for (int counter = firstofRow; counter < firstofRow + 8; counter++)
    {
        madeBoard->mainBoard[counter - firstofRow + (firstofRow - 20) / 10 * 8] = boardArray[counter];
        if (boardArray[counter] != 0)
        {
            madeBoard->pieces[boardArray[counter]].push_back(counter - firstofRow + (firstofRow - 20) / 10 * 8);
            if (boardArray[counter] < 8)
            {
                madeBoard->whitePieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
            else
            {
                madeBoard->blackPieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
        }
    }
    firstofRow += 10;
    for (int counter = firstofRow; counter < firstofRow + 8; counter++)
    {
        madeBoard->mainBoard[counter - firstofRow + (firstofRow - 20) / 10 * 8] = boardArray[counter];
        if (boardArray[counter] != 0)
        {
            madeBoard->pieces[boardArray[counter]].push_back(counter - firstofRow + (firstofRow - 20) / 10 * 8);
            if (boardArray[counter] < 8)
            {
                madeBoard->whitePieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
            else
            {
                madeBoard->blackPieces += Option::PowerTwo[counter - firstofRow + (firstofRow - 20) / 10 * 8];
            }
        }
    }
    if (boardArray[120] == 0)
    {
        madeBoard->sideToMove = false;
    }
    else
    {
        madeBoard->sideToMove = true;
        madeBoard->ZobristHashCode ^= BoardInitializer::ZCodeFlag[7];
    }
    if (boardArray[121] == 1)
    {
        madeBoard->whiteSmallCastle = true;
        madeBoard->ZobristHashCode ^= BoardInitializer::ZCodeFlag[6];
    }
    else
    {
        madeBoard->whiteSmallCastle = false;
    }
    if (boardArray[122] == 1)
    {
        madeBoard->whiteBigCastle = true;
        madeBoard->ZobristHashCode ^= BoardInitializer::ZCodeFlag[5];
    }
    else
    {
        madeBoard->whiteBigCastle = false;
    }
    if (boardArray[123] == 1)
    {
        madeBoard->blackSmallCastle = true;
        madeBoard->ZobristHashCode ^= BoardInitializer::ZCodeFlag[4];
    }
    else
    {
        madeBoard->blackSmallCastle = false;
    }
    if (boardArray[124] == 1)
    {
        madeBoard->blackBigCastle = true;
        madeBoard->ZobristHashCode ^= BoardInitializer::ZCodeFlag[3];
    }
    else
    {
        madeBoard->blackBigCastle = false;
    }
    madeBoard->unpassentPlace = boardArray[125];
    madeBoard->ZobristHashCode ^= BoardInitializer::ZCodeUnpassentPlace[boardArray[125]];
    madeBoard->fiftyMoveRule = boardArray[126];
    for (int whitePawnPlace : madeBoard->pieces[1])
    {
        madeBoard->whitePawns += Option::PowerTwo[whitePawnPlace];
    }
    for (int blackPawnPlace : madeBoard->pieces[9])
    {
        madeBoard->blackPawns += Option::PowerTwo[blackPawnPlace];
    }
    return madeBoard;
}
