#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "Move.h"
#include "Board.h"
#include "Search.h"
#include "MissingInfoAboutPrevStateFromMove.h"

class GameLogic
{
public:

    static void DoMove(Board& thisBoard, Move& thisMove, Move& prevMove, int depth, int depthGone);
    static void HaveReachedToMoveSequence(Move& move, Move& prevMove, int depth, int depthGone);
    static void UndoMove(Board& thisBoard, Move& thisMove, MissingInfoAboutPrevStateFromMove& missingInfo);

private:
    static void SetUnpassent(Board& thisBoard, Move& thisMove);
    static void SetCastleFlags(Board& thisBoard, Move& thisMove);  
    static void ChangeSide(Board& thisBoard);
    static void SimpleMove(Board& thisBoard, Move& thisMove);
    static void Capture(Board& thisBoard, Move& thisMove);
    static void BoardPawnListsUpdate(Board& thisBoard, Move& thisMove);
    static void Promote(Board& thisBoard, Move& thisMove);
    static void Unpassent(Board& thisBoard, Move& thisMove);
    static void BlackRightCastle(Board& thisBoard, int beginPlace);
    static void BlackLeftCastle(Board& thisBoard, int beginPlace);  
    static void WhiteLeftCastle(Board& thisBoard, int beginPlace);
    static void WhiteRightCastle(Board& thisBoard, int beginPlace);
    static void UnSetUnpassentPlace(Board& thisBoard, Move& thisMove, int previousUnpassentPlace);
    static void UnSideChange(Board& thisBoard, Move& thisMove);
    static void UnSetCastleFlags(Board& thisBoard, Move& thisMove, bool previousWhiteBigCastle, bool previousWhiteSmallCastle, bool previousBlackBigCastle, bool previousBlackSmallCastle);
    static void UnBoardPawnListsUpdate(Board& thisBoard, Move& thisMove);
    static void UnSimpleMove(Board& thisBoard, Move& thisMove);
    static void UnCapture(Board& thisBoard, Move& thisMove);
    static void UnPromote(Board& thisBoard, Move& thisMove);
    static void UnUnpassent(Board& thisBoard, Move& thisMove);
    static void UnBlackLeftCastle(Board& thisBoard, Move& thisMove);
    static void UnBlackRightCastle(Board& thisBoard, Move& thisMove);
    static void UnWhiteLeftCastle(Board& thisBoard, Move& thisMove);
    static void UnWhiteRightCastle(Board& thisBoard, Move& thisMove);
    static void UpdateBoardCastleRights(Board& thisBoard, Move& thisMove);

};

#endif