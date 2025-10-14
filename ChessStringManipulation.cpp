#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "ChessStringManipulation.h"

Move* ChessStringManipulation::ConvertTextToMove(const std::string& move, Board& thisBoard)
{
    Move* thisMove = ConvertTextToMoveWithoutFlags(move);
    int beginPiece = thisBoard.mainBoard[thisMove->beginPlace];
    thisMove->endPiece = thisBoard.mainBoard[thisMove->endPlace];

    if (thisMove->beginPlace / 8 == 1 && thisMove->endPlace / 8 == 3 && thisBoard.mainBoard[thisMove->beginPlace] == 1)
    {
        thisMove->unpassentPlace = thisMove->beginPlace + 8;
    }
    if (thisMove->beginPlace / 8 == 6 && thisMove->endPlace / 8 == 4 && thisBoard.mainBoard[thisMove->beginPlace] == 9)
    {
        thisMove->unpassentPlace = thisMove->beginPlace - 8;
    }
    if (thisMove->endPlace == 0 || thisMove->beginPlace == 0)
    {
        thisMove->CastleFlag += (char)64;
    }
    if (thisMove->endPlace == 7 || thisMove->beginPlace == 7)
    {
        thisMove->CastleFlag += (char)128;
    }
    if (thisMove->endPlace == 56 || thisMove->beginPlace == 56)
    {
        thisMove->CastleFlag += (char)16;
    }
    if (thisMove->endPlace == 63 || thisMove->beginPlace == 63)
    {
        thisMove->CastleFlag += (char)32;
    }
    if (thisMove->beginPlace == 4)
    {
        thisMove->CastleFlag += (char)64;
        thisMove->CastleFlag += (char)128;
    }
    if (thisMove->beginPlace == 60)
    {
        thisMove->CastleFlag += (char)16;
        thisMove->CastleFlag += (char)32;
    }
    if (thisMove->endPiece != 0)
    {
        thisMove->PublicFlag = (char)128;
    }
    if (thisMove->endPiece != 0 || beginPiece == 1 || beginPiece == 9)
    {
        thisMove->PublicFlag += (char)32;
    }
    if (thisMove->endPlace == thisBoard.unpassentPlace && (beginPiece == 1 || beginPiece == 9) && thisMove->endPlace != 0)
    {
        thisMove->PublicFlag += (char)64;
    }
    if (thisBoard.mainBoard[4] == 6 && thisMove->beginPlace == 4 && thisMove->endPlace == 6)
    {
        thisMove->CastleFlag = (char)(8 + 64 + 128);
    }
    if (thisBoard.mainBoard[4] == 6 && thisMove->beginPlace == 4 && thisMove->endPlace == 2)
    {
        thisMove->CastleFlag = (char)(4 + 64 + 128);
    }
    if (thisBoard.mainBoard[60] == 14 && thisMove->beginPlace == 60 && thisMove->endPlace == 62)
    {
        thisMove->CastleFlag = (char)(2 + 16 + 32);
    }
    if (thisBoard.mainBoard[60] == 14 && thisMove->beginPlace == 60 && thisMove->endPlace == 58)
    {
        thisMove->CastleFlag = (char)(1 + 16 + 32);
    }
    return thisMove;
}

Move* ChessStringManipulation::ConvertTextToMoveWithoutFlags(const std::string& move)
{
    Move* madeMove = new Move();
    madeMove->beginPlace = (move[1] - '1') * 8 + (move[0] - 'a');
    madeMove->endPlace = (move[3] - '1') * 8 + (move[2] - 'a');

    if (move.length() == 5)
    {
        switch (move[4])
        {
            case 'q':
                if (madeMove->endPlace > madeMove->beginPlace)
                {
                    madeMove->promotionPiece = 5;
                }
                else
                {
                    madeMove->promotionPiece = 13;
                }
                break;
            
            case 'r':
                if (madeMove->endPlace > madeMove->beginPlace)
                {
                    madeMove->promotionPiece = 4;
                }
                else
                {
                    madeMove->promotionPiece = 12;
                }                        
                break;
            case 'n':
                if (madeMove->endPlace > madeMove->beginPlace)
                {
                    madeMove->promotionPiece = 2;
                }
                else
                {
                    madeMove->promotionPiece = 10;
                }                        
                break;
            case 'b':
                if (madeMove->endPlace > madeMove->beginPlace)
                {
                    madeMove->promotionPiece = 3;
                }
                else
                {
                    madeMove->promotionPiece = 11;
                }                        
                break;
        }
    }
    return madeMove;
}

std::string ChessStringManipulation::PVToString(const Move& move, int type, bool mated, const Board& thisBoard)
{
    std::string moveString = "";
    bool whiteKingInPlace = false;
    bool blackKingInPlace = false;
    int beginPlace = move.beginPlace;
    int endPlace = move.endPlace;

    if (thisBoard.pieces[6].front() == 4)
    {
        whiteKingInPlace = true;
    }
    if (thisBoard.pieces[14].front() == 60)
    {
        blackKingInPlace = true;
    }

    //whiteSmallCastle
    if (whiteKingInPlace && beginPlace == 4 && endPlace == 6)
    {
        return "e1g1";
    }
    //whiteBigCastle
    else if (whiteKingInPlace && beginPlace == 4 && endPlace == 2)
    {
        return "e1c1";
    }
    //blackSmallCastle
    else if (blackKingInPlace && beginPlace == 60 && endPlace == 62)
    {
        return "e8g8";
    }
    //blackBigCastle
    else if (blackKingInPlace && beginPlace == 60 && endPlace == 58)
    {
        return "e8c8";
    }
    char temp = (char)('a' + beginPlace % 8);
    moveString += temp;
    temp = (char)('1' + beginPlace / 8);
    moveString += temp;
    temp = (char)('a' + endPlace % 8);
    moveString += temp;
    temp = (char)('1' + endPlace / 8);
    moveString += temp;
    if (move.promotionPiece > 0)
    {
        switch (move.promotionPiece)
        {
            case 2:
                moveString += 'n';
                break;
            case 3:
                moveString += 'b';
                break;
            case 4:
                moveString += 'r';
                break;
            case 5:
                moveString += 'q';
                break;
            case 10:
                moveString += 'n';
                break;
            case 11:
                moveString += 'b';
                break;
            case 12:
                moveString += 'r';
                break;
            case 13:
                moveString += 'q';
                break;
        }
    }
    /*if (type == 1)
    {
        if (beginPlace == 4)
        {
            whiteKingInPlace = false;
        }
        if (beginPlace == 60)
        {
            blackKingInPlace = false;
        }
        String tempMoveString = "";
        char tempChar = 'a';
        List<Move> nextMoveList = move.childMoves;
        while (nextMoveList != null)
        {
            Move nextMove = nextMoveList.First();
            if (nextMove.promotionPiece == -2)
            {
                break;
            }
            if (nextMove.promotionPiece == -3)
            {
                break;
            }
            tempMoveString = "";
            tempChar = 'a';
            beginPlace = nextMove.beginPlace;
            endPlace = nextMove.endPlace;
            //whiteSmallCastle
            if (whiteKingInPlace && beginPlace == 4 && endPlace == 6)
            {
                tempMoveString =  "e1g1";
            }
            //whiteBigCastle
            else if (whiteKingInPlace && beginPlace == 4 && endPlace == 2)
            {
                tempMoveString =  "e1c1";
            }
            //blackSmallCastle
            else if (blackKingInPlace && beginPlace == 60 && endPlace == 62)
            {
                tempMoveString = "e8g8";
            }
            //blackBigCastle
            else if (blackKingInPlace && beginPlace == 60 && endPlace == 58)
            {
                tempMoveString = "e8c8";
            }
            tempChar = (char)('a' + beginPlace % 8);
            tempMoveString = tempMoveString + tempChar;
            tempChar = (char)('1' + beginPlace / 8);
            tempMoveString = tempMoveString + tempChar;
            tempChar = (char)('a' + endPlace % 8);
            tempMoveString = tempMoveString + tempChar;
            tempChar = (char)('1' + endPlace / 8);
            tempMoveString = tempMoveString + tempChar;
            if (nextMove.promotionPiece > 0)
            {
                switch (nextMove.promotionPiece)
                {
                    case 2:
                        tempMoveString = tempMoveString + 'n';
                        break;
                    case 3:
                        tempMoveString = tempMoveString + 'b';
                        break;
                    case 4:
                        tempMoveString = tempMoveString + 'r';
                        break;
                    case 5:
                        tempMoveString = tempMoveString + 'q';
                        break;
                    case 10:
                        tempMoveString = tempMoveString + 'n';
                        break;
                    case 11:
                        tempMoveString = tempMoveString + 'b';
                        break;
                    case 12:
                        tempMoveString = tempMoveString + 'r';
                        break;
                    case 13:
                        tempMoveString = tempMoveString + 'q';
                        break;
                }
            }
            if (beginPlace == 4)
            {
                whiteKingInPlace = false;
            }
            if (beginPlace == 60)
            {
                blackKingInPlace = false;
            }
            nextMoveList = nextMove.childMoves;
            moveString = moveString + ' ' + tempMoveString;
        }
    }*/
    return moveString;
}


