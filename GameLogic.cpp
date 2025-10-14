#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "GameLogic.h"
#include "UCI.h"
#include "Search.h"
#include "Option.h"
#include <iostream>
#include "BoardInitializer.h"
#include <algorithm>

void GameLogic::HaveReachedToMoveSequence(Move &move, Move &prevMove, int depth, int depthGone)
{
    if (move.beginPlace == 8 && move.endPlace == 16 && depth == 1 && depthGone == 2)
    {
        int testCounter = 0;
        testCounter++;
    }
}

void GameLogic::DoMove(Board &thisBoard, Move &thisMove, Move &prevMove, int depth, int depthGone)
{
    if (UCI::IsTest())
        HaveReachedToMoveSequence(thisMove, prevMove, depth, depthGone);
    int beginPlace = thisMove.beginPlace;
    Search::moveCount++;
    if (thisMove.promotionPiece >= 0)
    {
        if ((thisMove.CastleFlag & Option::PowerTwo[3]) != 0)
        {
            WhiteRightCastle(thisBoard, beginPlace);
        }
        else if ((thisMove.CastleFlag & Option::PowerTwo[2]) != 0)
        {
            WhiteLeftCastle(thisBoard, beginPlace);
        }
        else if ((thisMove.CastleFlag & Option::PowerTwo[1]) != 0)
        {
            BlackRightCastle(thisBoard, beginPlace);
        }
        else if ((thisMove.CastleFlag & Option::PowerTwo[0]) != 0)
        {
            BlackLeftCastle(thisBoard, beginPlace);
        }
        else if ((thisMove.PublicFlag & Option::PowerTwo[6]) != 0)
        {
            Unpassent(thisBoard, thisMove);
        }
        else if (thisMove.promotionPiece > 0)
        {
            Promote(thisBoard, thisMove);
        }
        else
        {
            BoardPawnListsUpdate(thisBoard, thisMove);

            if ((thisMove.PublicFlag & Option::PowerTwo[7]) != 0)
            {
                Capture(thisBoard, thisMove);
            }
            else
            {
                SimpleMove(thisBoard, thisMove);
            }
        }
        UpdateBoardCastleRights(thisBoard, thisMove);
        SetUnpassent(thisBoard, thisMove);
        // if ((thisMove.PublicFlag & Option.PowerTwo[5]) != 0)
        //{
        //     thisBoard.fiftyMoveRule++;
        // }
        // else
        //{
        //     thisBoard.fiftyMoveRule = 0;
        // }
    }
    else
    {
        // thisBoard.fiftyMoveRule++;
    }
    ChangeSide(thisBoard);
    SetCastleFlags(thisBoard, thisMove);
    if (thisBoard.pieces[0].size() != 0)
    {
        std::cout << "Breakpoint here" << std::endl;
    }
}

void GameLogic::SetUnpassent(Board &thisBoard, Move &thisMove)
{
    if (thisBoard.unpassentPlace > 0)
    {
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeUnpassentPlace[thisBoard.unpassentPlace];
        thisBoard.unpassentPlace = 0;
    }
    if (thisMove.unpassentPlace > 0)
    {
        thisBoard.unpassentPlace = thisMove.unpassentPlace;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeUnpassentPlace[thisMove.unpassentPlace];
    }
}

void GameLogic::SetCastleFlags(Board &thisBoard, Move &thisMove)
{
    if ((thisMove.CastleFlag & 12) != 0)
    {
        if (thisBoard.whiteBigCastle)
        {
            thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[5];
            thisBoard.whiteBigCastle = false;
        }
        if (thisBoard.whiteSmallCastle)
        {
            thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[6];
            thisBoard.whiteSmallCastle = false;
        }
    }
    if ((thisMove.CastleFlag & 3) != 0)
    {
        if (thisBoard.blackBigCastle)
        {
            thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[3];
            thisBoard.blackBigCastle = false;
        }
        if (thisBoard.blackSmallCastle)
        {
            thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[4];
            thisBoard.blackSmallCastle = false;
        }
    }
}

void GameLogic::ChangeSide(Board &thisBoard)
{
    if (!thisBoard.sideToMove)
    {
        thisBoard.sideToMove = true;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[7];
    }
    else
    {
        thisBoard.sideToMove = false;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[7];
        thisBoard.moveNumber++;
    }
}

void GameLogic::SimpleMove(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[beginPlace];
    int endPiece = thisMove.endPiece;
    thisBoard.mainBoard[beginPlace] = 0;
    thisBoard.mainBoard[endPlace] = beginPiece;
    auto &pieceList = thisBoard.pieces[beginPiece];
    pieceList.erase(beginPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][beginPlace];
    thisBoard.pieces[beginPiece].push_back(endPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][endPlace];
    if (!thisBoard.sideToMove)
    {
        thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace];
        thisBoard.whitePieces |= Option::PowerTwo[endPlace];
    }
    else
    {
        thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace];
        thisBoard.blackPieces |= Option::PowerTwo[endPlace];
    }
}

void GameLogic::Capture(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[beginPlace];
    int endPiece = thisMove.endPiece;
    thisBoard.mainBoard[beginPlace] = 0;
    thisBoard.mainBoard[endPlace] = beginPiece;

    auto &beginPieceList = thisBoard.pieces[beginPiece];
    beginPieceList.erase(beginPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][beginPlace];
    thisBoard.pieces[beginPiece].push_back(endPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][endPlace];

    auto &endPieceList = thisBoard.pieces[endPiece];
    endPieceList.erase(endPlace);
    // todo: are these erases correct?
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[endPiece][endPlace];
    if (!thisBoard.sideToMove)
    {
        thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace];
        thisBoard.whitePieces |= Option::PowerTwo[endPlace];
        thisBoard.blackPieces &= Option::PowerTwoComplement[endPlace];
    }
    else
    {
        thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace];
        thisBoard.blackPieces |= Option::PowerTwo[endPlace];
        thisBoard.whitePieces &= Option::PowerTwoComplement[endPlace];
    }
}

void GameLogic::BoardPawnListsUpdate(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[beginPlace];
    int endPiece = thisMove.endPiece;
    if (beginPiece == 1)
    {
        thisBoard.whitePawns &= Option::PowerTwoComplement[beginPlace];
        if (thisMove.promotionPiece == 0)
        {
            thisBoard.whitePawns |= Option::PowerTwo[endPlace];
        }
    }
    else if (beginPiece == 9)
    {
        thisBoard.blackPawns &= Option::PowerTwoComplement[beginPlace];
        if (thisMove.promotionPiece == 0)
        {
            thisBoard.blackPawns |= Option::PowerTwo[endPlace];
        }
    }
    if (endPiece == 1)
    {
        thisBoard.whitePawns &= Option::PowerTwoComplement[endPlace];
    }
    else if (endPiece == 9)
    {
        thisBoard.blackPawns &= Option::PowerTwoComplement[endPlace];
    }
}

void GameLogic::UpdateBoardCastleRights(Board &thisBoard, Move &thisMove)
{
    if ((thisMove.CastleFlag & Option::PowerTwo[6]) != 0 && thisBoard.whiteBigCastle)
    {
        thisBoard.whiteBigCastle = false;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[5];
    }
    if ((thisMove.CastleFlag & Option::PowerTwo[7]) != 0 && thisBoard.whiteSmallCastle)
    {
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[6];
        thisBoard.whiteSmallCastle = false;
    }
    if ((thisMove.CastleFlag & Option::PowerTwo[4]) != 0 && thisBoard.blackBigCastle)
    {
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[3];
        thisBoard.blackBigCastle = false;
    }
    if ((thisMove.CastleFlag & Option::PowerTwo[5]) != 0 && thisBoard.blackSmallCastle)
    {
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[4];
        thisBoard.blackSmallCastle = false;
    }
}

void GameLogic::Promote(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[beginPlace];
    int endPiece = thisMove.endPiece;
    if (beginPiece == 1)
    {
        thisBoard.whitePawns &= Option::PowerTwoComplement[beginPlace];
    }
    else if (beginPiece == 9)
    {
        thisBoard.blackPawns &= Option::PowerTwoComplement[beginPlace];
    }
    if ((thisMove.PublicFlag & Option::PowerTwo[7]) != 0)
    {
        thisBoard.mainBoard[beginPlace] = 0;
        thisBoard.mainBoard[endPlace] = thisMove.promotionPiece;
        auto &beginPieceList = thisBoard.pieces[beginPiece];
        beginPieceList.erase(beginPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][beginPlace];
        thisBoard.pieces[thisMove.promotionPiece].push_back(endPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[thisMove.promotionPiece][endPlace];

        auto &endPieceList = thisBoard.pieces[endPiece];
        endPieceList.erase(endPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[endPiece][endPlace];
        if (!thisBoard.sideToMove)
        {
            thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace];
            thisBoard.whitePieces |= Option::PowerTwo[endPlace];
            thisBoard.blackPieces &= Option::PowerTwoComplement[endPlace];
        }
        else
        {
            thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace];
            thisBoard.blackPieces |= Option::PowerTwo[endPlace];
            thisBoard.whitePieces &= Option::PowerTwoComplement[endPlace];
        }
    }
    else
    {
        thisBoard.mainBoard[beginPlace] = 0;
        thisBoard.mainBoard[endPlace] = thisMove.promotionPiece;
        auto &pieceList = thisBoard.pieces[beginPiece];
        pieceList.erase(beginPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][beginPlace];
        thisBoard.pieces[thisMove.promotionPiece].push_back(endPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[thisMove.promotionPiece][endPlace];
        if (!thisBoard.sideToMove)
        {
            thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace];
            thisBoard.whitePieces |= Option::PowerTwo[endPlace];
        }
        else
        {
            thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace];
            thisBoard.blackPieces |= Option::PowerTwo[endPlace];
        }
    }
}

void GameLogic::UnSimpleMove(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[endPlace];
    int endPiece = thisMove.endPiece;
    thisBoard.mainBoard[beginPlace] = beginPiece;
    thisBoard.mainBoard[endPlace] = 0;
    thisBoard.pieces[beginPiece].push_back(beginPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][beginPlace];
    thisBoard.pieces[beginPiece].erase(endPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][endPlace];
    if (!thisBoard.sideToMove)
    {
        thisBoard.whitePieces |= Option::PowerTwo[beginPlace];
        thisBoard.whitePieces &= Option::PowerTwoComplement[endPlace];
    }
    else
    {
        thisBoard.blackPieces |= Option::PowerTwo[beginPlace];
        thisBoard.blackPieces &= Option::PowerTwoComplement[endPlace];
    }
}

void GameLogic::UnCapture(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[endPlace];
    int endPiece = thisMove.endPiece;
    thisBoard.mainBoard[beginPlace] = beginPiece;
    thisBoard.mainBoard[endPlace] = endPiece;
    thisBoard.pieces[beginPiece].push_back(beginPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][beginPlace];
    thisBoard.pieces[beginPiece].erase(endPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][endPlace];
    thisBoard.pieces[endPiece].push_back(endPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[endPiece][endPlace];
    if (!thisBoard.sideToMove)
    {
        thisBoard.whitePieces |= Option::PowerTwo[beginPlace];
        thisBoard.whitePieces &= Option::PowerTwoComplement[endPlace];
        thisBoard.blackPieces |= Option::PowerTwo[endPlace];
    }
    else
    {
        thisBoard.blackPieces |= Option::PowerTwo[beginPlace];
        thisBoard.blackPieces &= Option::PowerTwoComplement[endPlace];
        thisBoard.whitePieces |= Option::PowerTwo[endPlace];
    }
}

void GameLogic::UnPromote(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[endPlace];
    beginPiece = beginPiece > 8 ? 9 : 1;
    int endPiece = thisMove.endPiece;
    if (beginPiece == 1)
    {
        thisBoard.whitePawns |= Option::PowerTwo[beginPlace];
    }
    else if (beginPiece == 9)
    {
        thisBoard.blackPawns |= Option::PowerTwo[beginPlace];
    }
    if ((thisMove.PublicFlag & Option::PowerTwo[7]) != 0)
    {
        thisBoard.mainBoard[endPlace] = endPiece;
        thisBoard.pieces[thisMove.promotionPiece].erase(endPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[thisMove.promotionPiece][endPlace];
        thisBoard.pieces[endPiece].push_back(endPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[endPiece][endPlace];
        if (!thisBoard.sideToMove)
        {
            thisBoard.mainBoard[beginPlace] = 1;
            thisBoard.pieces[1].push_back(beginPlace);
            thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[1][beginPlace];
            thisBoard.whitePieces |= Option::PowerTwo[beginPlace];
            thisBoard.whitePieces &= Option::PowerTwoComplement[endPlace];
            thisBoard.blackPieces |= Option::PowerTwo[endPlace];
        }
        else
        {
            thisBoard.mainBoard[beginPlace] = 9;
            thisBoard.pieces[9].push_back(beginPlace);
            thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[9][beginPlace];
            thisBoard.blackPieces |= Option::PowerTwo[beginPlace];
            thisBoard.blackPieces &= Option::PowerTwoComplement[endPlace];
            thisBoard.whitePieces |= Option::PowerTwo[endPlace];
        }
    }
    else
    {
        thisBoard.mainBoard[endPlace] = 0;
        thisBoard.pieces[thisMove.promotionPiece].erase(endPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[thisMove.promotionPiece][endPlace];
        if (!thisBoard.sideToMove)
        {
            thisBoard.mainBoard[beginPlace] = 1;
            thisBoard.pieces[1].push_back(beginPlace);
            thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[1][beginPlace];
            thisBoard.whitePieces |= Option::PowerTwo[beginPlace];
            thisBoard.whitePieces &= Option::PowerTwoComplement[endPlace];
        }
        else
        {
            thisBoard.mainBoard[beginPlace] = 9;
            thisBoard.pieces[9].push_back(beginPlace);
            thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[9][beginPlace];
            thisBoard.blackPieces |= Option::PowerTwo[beginPlace];
            thisBoard.blackPieces &= Option::PowerTwoComplement[endPlace];
        }
    }
}

void GameLogic::UnUnpassent(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[endPlace];
    int endPiece = thisMove.endPiece;
    if (beginPiece == 1)
    {
        thisBoard.whitePawns |= Option::PowerTwo[beginPlace];
        thisBoard.whitePawns &= Option::PowerTwoComplement[endPlace];
        thisBoard.blackPawns |= Option::PowerTwo[endPlace - 8];
        thisBoard.mainBoard[beginPlace] = 1;
        thisBoard.mainBoard[endPlace] = 0;
        thisBoard.mainBoard[endPlace - 8] = 9;
        thisBoard.whitePieces |= Option::PowerTwo[beginPlace];
        thisBoard.whitePieces &= Option::PowerTwoComplement[endPlace];
        thisBoard.pieces[beginPiece].push_back(beginPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][beginPlace];
        thisBoard.pieces[beginPiece].erase(endPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][endPlace];
        thisBoard.blackPieces |= Option::PowerTwo[endPlace - 8];
        thisBoard.pieces[9].push_back(endPlace - 8);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[9][endPlace - 8];
    }
    else
    {
        thisBoard.blackPawns |= Option::PowerTwo[beginPlace];
        thisBoard.blackPawns &= Option::PowerTwoComplement[endPlace];
        thisBoard.whitePawns |= Option::PowerTwo[endPlace + 8];
        thisBoard.mainBoard[beginPlace] = 9;
        thisBoard.mainBoard[endPlace] = 0;
        thisBoard.mainBoard[endPlace + 8] = 1;
        thisBoard.blackPieces |= Option::PowerTwo[beginPlace];
        thisBoard.blackPieces &= Option::PowerTwoComplement[endPlace];
        thisBoard.pieces[beginPiece].push_back(beginPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][beginPlace];
        thisBoard.pieces[beginPiece].erase(endPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][endPlace];
        thisBoard.whitePieces |= Option::PowerTwo[endPlace + 8];
        thisBoard.pieces[1].push_back(endPlace + 8);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[1][endPlace + 8];
    }
}

void GameLogic::UnBlackLeftCastle(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[endPlace];
    int endPiece = thisMove.endPiece;
    thisBoard.mainBoard[beginPlace] = 14;
    thisBoard.mainBoard[beginPlace - 4] = 12;
    thisBoard.mainBoard[beginPlace - 2] = 0;
    thisBoard.mainBoard[beginPlace - 1] = 0;
    thisBoard.blackPieces |= Option::PowerTwo[beginPlace];
    thisBoard.blackPieces |= Option::PowerTwo[beginPlace - 4];
    thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace - 2];
    thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace - 1];
    thisBoard.pieces[12].push_back(beginPlace - 4);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[12][beginPlace - 4];
    thisBoard.pieces[12].erase(beginPlace - 1);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[12][beginPlace - 1];
    thisBoard.pieces[14].push_back(beginPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[14][beginPlace];
    thisBoard.pieces[14].erase(beginPlace - 2);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[14][beginPlace - 2];
}

void GameLogic::UnBlackRightCastle(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[endPlace];
    int endPiece = thisMove.endPiece;
    thisBoard.mainBoard[beginPlace] = 14;
    thisBoard.mainBoard[beginPlace + 3] = 12;
    thisBoard.mainBoard[beginPlace + 2] = 0;
    thisBoard.mainBoard[beginPlace + 1] = 0;
    thisBoard.blackPieces |= Option::PowerTwo[beginPlace];
    thisBoard.blackPieces |= Option::PowerTwo[beginPlace + 3];
    thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace + 2];
    thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace + 1];
    thisBoard.pieces[12].push_back(beginPlace + 3);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[12][beginPlace + 3];
    thisBoard.pieces[12].erase(beginPlace + 1);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[12][beginPlace + 1];
    thisBoard.pieces[14].push_back(beginPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[14][beginPlace];
    thisBoard.pieces[14].erase(beginPlace + 2);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[14][beginPlace + 2];
}

void GameLogic::UnWhiteLeftCastle(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[endPlace];
    int endPiece = thisMove.endPiece;
    thisBoard.mainBoard[beginPlace] = 6;
    thisBoard.mainBoard[beginPlace - 4] = 4;
    thisBoard.mainBoard[beginPlace - 2] = 0;
    thisBoard.mainBoard[beginPlace - 1] = 0;
    thisBoard.whitePieces |= Option::PowerTwo[beginPlace];
    thisBoard.whitePieces |= Option::PowerTwo[beginPlace - 4];
    thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace - 2];
    thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace - 1];
    thisBoard.pieces[4].push_back(beginPlace - 4);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[4][beginPlace - 4];
    thisBoard.pieces[4].erase(beginPlace - 1);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[4][beginPlace - 1];
    thisBoard.pieces[6].push_back(beginPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[6][beginPlace];
    thisBoard.pieces[6].erase(beginPlace - 2);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[6][beginPlace - 2];
}

void GameLogic::UnWhiteRightCastle(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[endPlace];
    int endPiece = thisMove.endPiece;
    thisBoard.mainBoard[beginPlace] = 6;
    thisBoard.mainBoard[beginPlace + 3] = 4;
    thisBoard.mainBoard[beginPlace + 2] = 0;
    thisBoard.mainBoard[beginPlace + 1] = 0;
    thisBoard.whitePieces |= Option::PowerTwo[beginPlace];
    thisBoard.whitePieces |= Option::PowerTwo[beginPlace + 3];
    thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace + 2];
    thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace + 1];
    thisBoard.pieces[4].push_back(beginPlace + 3);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[4][beginPlace + 3];
    thisBoard.pieces[4].erase(beginPlace + 1);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[4][beginPlace + 1];
    thisBoard.pieces[6].push_back(beginPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[6][beginPlace];
    thisBoard.pieces[6].erase(beginPlace + 2);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[6][beginPlace + 2];
}

void GameLogic::BlackRightCastle(Board &thisBoard, int beginPlace)
{
    thisBoard.mainBoard[beginPlace] = 0;
    thisBoard.mainBoard[beginPlace + 3] = 0;
    thisBoard.mainBoard[beginPlace + 2] = 14;
    thisBoard.mainBoard[beginPlace + 1] = 12;
    thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace];
    thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace + 3];
    thisBoard.blackPieces |= Option::PowerTwo[beginPlace + 2];
    thisBoard.blackPieces |= Option::PowerTwo[beginPlace + 1];
    auto &rookList = thisBoard.pieces[12];
    rookList.erase(beginPlace + 3);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[12][beginPlace + 3];
    thisBoard.pieces[12].push_back(beginPlace + 1);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[12][beginPlace + 1];

    auto &kingList = thisBoard.pieces[14];
    kingList.erase(beginPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[14][beginPlace];
    thisBoard.pieces[14].push_back(beginPlace + 2);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[14][beginPlace + 2];
    if (thisBoard.blackSmallCastle)
    {
        thisBoard.blackSmallCastle = false;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[4];
    }
}

void GameLogic::BlackLeftCastle(Board &thisBoard, int beginPlace)
{
    thisBoard.mainBoard[beginPlace] = 0;
    thisBoard.mainBoard[beginPlace - 4] = 0;
    thisBoard.mainBoard[beginPlace - 2] = 14;
    thisBoard.mainBoard[beginPlace - 1] = 12;
    thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace];
    thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace - 4];
    thisBoard.blackPieces |= Option::PowerTwo[beginPlace - 2];
    thisBoard.blackPieces |= Option::PowerTwo[beginPlace - 1];
    auto &rookList = thisBoard.pieces[12];
    rookList.erase(beginPlace - 4);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[12][beginPlace - 4];
    thisBoard.pieces[12].push_back(beginPlace - 1);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[12][beginPlace - 1];

    auto &kingList = thisBoard.pieces[14];
    kingList.erase(beginPlace);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[14][beginPlace];
    thisBoard.pieces[14].push_back(beginPlace - 2);
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[14][beginPlace - 2];
    if (thisBoard.blackSmallCastle)
    {
        thisBoard.blackSmallCastle = false;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[4];
    }
    if (thisBoard.blackBigCastle)
    {
        thisBoard.blackBigCastle = false;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[3];
    }
}

void GameLogic::WhiteLeftCastle(Board &thisBoard, int beginPlace)
{
    thisBoard.mainBoard[beginPlace] = 0;
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[6][beginPlace];
    thisBoard.mainBoard[beginPlace - 4] = 0;
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[4][beginPlace - 4];
    thisBoard.mainBoard[beginPlace - 2] = 6;
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[6][beginPlace - 2];
    thisBoard.mainBoard[beginPlace - 1] = 4;
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[4][beginPlace - 1];
    thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace];
    thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace - 4];
    thisBoard.whitePieces |= Option::PowerTwo[beginPlace - 2];
    thisBoard.whitePieces |= Option::PowerTwo[beginPlace - 1];
    auto &rookList = thisBoard.pieces[4];
    rookList.erase(beginPlace - 4);
    thisBoard.pieces[4].push_back(beginPlace - 1);
    auto &queenList = thisBoard.pieces[6];
    queenList.erase(beginPlace);
    thisBoard.pieces[6].push_back(beginPlace - 2);
    if (thisBoard.whiteSmallCastle)
    {
        thisBoard.whiteSmallCastle = false;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[6];
    }
    if (thisBoard.whiteBigCastle)
    {
        thisBoard.whiteBigCastle = false;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[5];
    }
}

void GameLogic::WhiteRightCastle(Board &thisBoard, int beginPlace)
{
    thisBoard.mainBoard[beginPlace] = 0;
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[6][beginPlace];
    thisBoard.mainBoard[beginPlace + 3] = 0;
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[4][beginPlace + 3];
    thisBoard.mainBoard[beginPlace + 2] = 6;
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[6][beginPlace + 2];
    thisBoard.mainBoard[beginPlace + 1] = 4;
    thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[4][beginPlace + 1];
    thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace];
    thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace + 3];
    thisBoard.whitePieces |= Option::PowerTwo[beginPlace + 2];
    thisBoard.whitePieces |= Option::PowerTwo[beginPlace + 1];
    auto &rookList = thisBoard.pieces[4];
    rookList.erase(beginPlace + 3);
    thisBoard.pieces[4].push_back(beginPlace + 1);
    auto &queenList = thisBoard.pieces[6];
    queenList.erase(beginPlace);
    thisBoard.pieces[6].push_back(beginPlace + 2);
    if (thisBoard.whiteSmallCastle)
    {
        thisBoard.whiteSmallCastle = false;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[6];
    }
    if (thisBoard.whiteBigCastle)
    {
        thisBoard.whiteBigCastle = false;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[5];
    }
}

void GameLogic::Unpassent(Board &thisBoard, Move &thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[beginPlace];
    int endPiece = thisMove.endPiece;
    if (beginPiece == 1)
    {
        thisBoard.whitePawns &= Option::PowerTwoComplement[beginPlace];
        thisBoard.whitePawns |= Option::PowerTwo[endPlace];
        thisBoard.blackPawns &= Option::PowerTwoComplement[endPlace - 8];
        thisBoard.mainBoard[beginPlace] = 0;
        thisBoard.mainBoard[endPlace] = 1;
        thisBoard.mainBoard[endPlace - 8] = 0;
        thisBoard.whitePieces &= Option::PowerTwoComplement[beginPlace];
        thisBoard.whitePieces |= Option::PowerTwo[endPlace];
        auto &beginPieceList = thisBoard.pieces[beginPiece];
        beginPieceList.erase(beginPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][beginPlace];
        thisBoard.pieces[beginPiece].push_back(endPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][endPlace];
        thisBoard.blackPieces &= Option::PowerTwoComplement[endPlace - 8];

        auto &endPieceList = thisBoard.pieces[9];
        endPieceList.erase(endPlace - 8);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[9][endPlace - 8];
    }
    else
    {
        thisBoard.blackPawns &= Option::PowerTwoComplement[beginPlace];
        thisBoard.blackPawns |= Option::PowerTwo[endPlace];
        thisBoard.whitePawns &= Option::PowerTwoComplement[endPlace + 8];
        thisBoard.mainBoard[beginPlace] = 0;
        thisBoard.mainBoard[endPlace] = 9;
        thisBoard.mainBoard[endPlace + 8] = 0;
        thisBoard.blackPieces &= Option::PowerTwoComplement[beginPlace];
        thisBoard.blackPieces |= Option::PowerTwo[endPlace];
        auto &beginPieceList = thisBoard.pieces[beginPiece];
        beginPieceList.erase(beginPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][beginPlace];
        thisBoard.pieces[beginPiece].push_back(endPlace);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[beginPiece][endPlace];
        thisBoard.whitePieces &= Option::PowerTwoComplement[endPlace + 8];

        auto &endpPieceList = thisBoard.pieces[1];
        endpPieceList.erase(endPlace + 8);
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCode[1][endPlace + 8];
    }
}

void GameLogic::UndoMove(Board &thisBoard, Move &thisMove, MissingInfoAboutPrevStateFromMove &missingInfo)
{
    UnSideChange(thisBoard, thisMove);
    if (thisMove.promotionPiece >= 0)
    {
        int beginPlace = thisMove.beginPlace;
        int endPlace = thisMove.endPlace;
        int beginPiece = thisBoard.mainBoard[endPlace];
        int endPiece = thisMove.endPiece;
        if ((thisMove.CastleFlag & Option::PowerTwo[3]) != 0)
        {
            UnWhiteRightCastle(thisBoard, thisMove);
        }
        else if ((thisMove.CastleFlag & Option::PowerTwo[2]) != 0)
        {
            UnWhiteLeftCastle(thisBoard, thisMove);
        }
        else if ((thisMove.CastleFlag & Option::PowerTwo[1]) != 0)
        {
            UnBlackRightCastle(thisBoard, thisMove);
        }
        else if ((thisMove.CastleFlag & Option::PowerTwo[0]) != 0)
        {
            UnBlackLeftCastle(thisBoard, thisMove);
        }
        else if ((thisMove.PublicFlag & Option::PowerTwo[6]) != 0)
        {
            UnUnpassent(thisBoard, thisMove);
        }
        else if (thisMove.promotionPiece > 0)
        {
            UnPromote(thisBoard, thisMove);
        }
        else
        {
            UnBoardPawnListsUpdate(thisBoard, thisMove);
            if ((thisMove.PublicFlag & Option::PowerTwo[7]) != 0)
            {
                UnCapture(thisBoard, thisMove);
            }
            else
            {
                UnSimpleMove(thisBoard, thisMove);
            }
        }
        UnSetCastleFlags(thisBoard, thisMove, missingInfo.previousWhiteBigCastle, missingInfo.previousWhiteSmallCastle, missingInfo.previousBlackBigCastle, missingInfo.previousBlackSmallCastle);
        UnSetUnpassentPlace(thisBoard, thisMove, missingInfo.previousUnpassentPlace);
    }
    else
    {
        // thisBoard.fiftyMoveRule--;
    }
}

void GameLogic::UnSetUnpassentPlace(Board& thisBoard, Move& thisMove, int previousUnpassentPlace)
{
    if (thisBoard.unpassentPlace > 0)
    {
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeUnpassentPlace[thisBoard.unpassentPlace];
        thisBoard.unpassentPlace = 0;
    }
    if (previousUnpassentPlace > 0)
    {
        thisBoard.unpassentPlace = previousUnpassentPlace;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeUnpassentPlace[previousUnpassentPlace];
    }
}

void GameLogic::UnSideChange(Board& thisBoard, Move& thisMove)
{
    if (!thisBoard.sideToMove)
    {
        thisBoard.sideToMove = true;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[7];
    }
    else
    {
        thisBoard.sideToMove = false;
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[7];
        thisBoard.moveNumber++;
    }
}

void GameLogic::UnSetCastleFlags(Board& thisBoard, Move& thisMove, bool previousWhiteBigCastle, bool previousWhiteSmallCastle, bool previousBlackBigCastle, bool previousBlackSmallCastle)
{
    if (!thisBoard.whiteBigCastle && previousWhiteBigCastle)
    {
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[5];
        thisBoard.whiteBigCastle = true;
    }
    if (!thisBoard.whiteSmallCastle && previousWhiteSmallCastle)
    {
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[6];
        thisBoard.whiteSmallCastle = true;
    }

    if (!thisBoard.blackBigCastle && previousBlackBigCastle)
    {
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[3];
        thisBoard.blackBigCastle = true;
    }
    if (!thisBoard.blackSmallCastle && previousBlackSmallCastle)
    {
        thisBoard.ZobristHashCode ^= BoardInitializer::ZCodeFlag[4];
        thisBoard.blackSmallCastle = true;
    }
}

void GameLogic::UnBoardPawnListsUpdate(Board& thisBoard, Move& thisMove)
{
    int beginPlace = thisMove.beginPlace;
    int endPlace = thisMove.endPlace;
    int beginPiece = thisBoard.mainBoard[endPlace];
    int endPiece = thisMove.endPiece;
    if (beginPiece == 1)
    {
        thisBoard.whitePawns |= Option::PowerTwo[beginPlace];
        thisBoard.whitePawns &= Option::PowerTwoComplement[endPlace];
    }
    else if (beginPiece == 9)
    {
        thisBoard.blackPawns |= Option::PowerTwo[beginPlace];
        thisBoard.blackPawns &= Option::PowerTwoComplement[endPlace];
    }
    if (endPiece == 1)
    {
        thisBoard.whitePawns |= Option::PowerTwo[endPlace];
    }
    else if (endPiece == 9)
    {
        thisBoard.blackPawns |= Option::PowerTwo[endPlace];
    }
}

