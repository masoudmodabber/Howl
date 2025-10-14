#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "QSearcher.h"
#include "Option.h"
#include "BoardLogic.h"
#include "EvaluationLogic.h"
#include "MoveLogic.h"
#include "UCI.h"
#include "GameLogic.h"
#include "MoveLogic.h"
#include "ChessStringManipulation.h"

int QSearcher::pieceValue100[15] = {
    0,    // pieceValue100[0]
    100,  // pieceValue100[1]
    350,  // pieceValue100[2]
    350,  // pieceValue100[3]
    550,  // pieceValue100[4]
    975,  // pieceValue100[5]
    2500, // pieceValue100[6]
    0,    // pieceValue100[7]
    0,    // pieceValue100[8]
    100,  // pieceValue100[9]
    350,  // pieceValue100[10]
    350,  // pieceValue100[11]
    550,  // pieceValue100[12]
    975,  // pieceValue100[13]
    2500, // pieceValue100[14]
};

MovePrintValue* QSearcher::QSearch(bool isPVNode, int alpha, int beta, Move& prevMove, int depthGone, int lastCheck, bool kick, int depth, Move& move1, Move& move2, Move& move3, Board& board4, bool MAtESearch, int depthQuisStarted, bool nullWindowSearch)
{
    MoveList moveList;
    Move* SelectedMove = nullptr;
    Board* boardCopy;
    int extention = Option::checkExtension;
    if (isPVNode) {
        extention = Option::checkExtensionNonPV;
    }
    
    MovePrintValue* retValue = new MovePrintValue();
    retValue->printString = "";
    
    MovePrintValue* MPValue = new MovePrintValue();
    MPValue->printString = "";
    
    int turn;
    if (!board4.sideToMove) {
        turn = 0;
    } else {
        turn = 1;
    }
    
    if (BoardLogic::UnderAttack(board4, board4.pieces[(1 - turn) * 8 + 6].front(), board4.sideToMove)) {
        retValue->value = 160000;
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    }
    
    if (depth == 0 && lastCheck == 0 && !kick && (depthGone - depthQuisStarted > extention || !BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))) {
        retValue->value = EvaluationLogic::Evaluate(board4);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    }
    
    if (BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove) && lastCheck < 2 && depthGone - depthQuisStarted < extention) {
        int valueTemp = EvaluationLogic::Evaluate(board4);
        if (valueTemp >= beta) {
            retValue->value = valueTemp;
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        delete retValue;
        retValue = nullptr;
        delete MPValue;
        MPValue = nullptr;
        return QSearch(isPVNode, alpha, beta, prevMove, depthGone, 2, false, 2, move1, move2, move3, board4, MAtESearch, depthQuisStarted, nullWindowSearch);
    }
    
    int nextLastCheck = 0;
    bool checkChecked = false;
    if (lastCheck > 0) {
        checkChecked = true;
        nextLastCheck = lastCheck - 1;
    } else if (kick) {
        // Delta Pruning
        if (prevMove.endPiece % 8 == 1 && depthGone - depthQuisStarted > extention) {
            retValue->value = EvaluationLogic::Evaluate(board4) - 50;
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        delete retValue;
        retValue = nullptr;
        delete MPValue;
        MPValue = nullptr;
        return QSearch(isPVNode, alpha, beta, prevMove, depthGone, 0, false, 1, move1, move2, move3, board4, MAtESearch, depthQuisStarted, nullWindowSearch);
    }
    if (depth == 0) {
        retValue->value = EvaluationLogic::Evaluate(board4);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    }
    if (!checkChecked) {
        int valueTemp2 = EvaluationLogic::Evaluate(board4);
        if (valueTemp2 >= beta) {
            retValue->value = valueTemp2;
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
    }
    int bestMoveValue = -200000;
    moveList = MoveLogic::MoveGenerator(board4, depth, depthGone);
    std::string SelectedPV = "";
    int availMoves = 0;
    int standPot = EvaluationLogic::Evaluate(board4);
    bool firstMove = true;
    for (int i = 0; i < moveList.count; ++i) {
        Move* move = moveList.moves[i];
        int pieceValueTemp = pieceValue100[move->endPiece];
        if (Option::SafetyMargin + standPot + pieceValueTemp <= alpha) {
            firstMove = false;
            move->value = Option::SafetyMargin + standPot + pieceValueTemp - 1;
            if (move->value > bestMoveValue) {
                bestMoveValue = move->value;
                SelectedMove = move;
            }
            availMoves++;
            continue;
        }
        if (firstMove) {
            boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
            MissingInfoAboutPrevStateFromMove* missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
            GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone);
            if (MoveLogic::Same(move2, move3, prevMove, *move)) {
                bestMoveValue = 0;
                SelectedMove = move;
                move->value = 0;
            } else {
                if (move->endPiece > 0 || move->promotionPiece > 0) {
                    delete MPValue;
                    MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, true, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                    bestMoveValue = -MPValue->value;
                    SelectedMove = move;
                    SelectedPV = MPValue->printString;
                } else {
                    delete MPValue;
                    MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, false, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                    bestMoveValue = -MPValue->value;
                    SelectedMove = move;
                    SelectedPV = MPValue->printString;
                }
                if (bestMoveValue != -160000) {
                    availMoves++;
                }
                if (bestMoveValue < -159800) {
                    bestMoveValue++;
                }
                move->value = bestMoveValue;
            }
            GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
            delete missingInfoAboutPrevStateFromMove;
            missingInfoAboutPrevStateFromMove = nullptr;
            if (UCI::IsTest()) {
                Board::AreBoardsEqual(board4, *boardCopy);
                delete boardCopy;
                boardCopy = nullptr;
            }
            firstMove = false;
            if (bestMoveValue > alpha) {
                if (bestMoveValue >= beta && ((bestMoveValue < 159800 && bestMoveValue > -159800) || !MAtESearch)) {
                    retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + MPValue->printString;
                    retValue->value = bestMoveValue;
                    deleteMoveList(moveList);
                    delete MPValue;
                    MPValue = nullptr;
                    return retValue;
                }
                alpha = bestMoveValue;
            }
        }
        else
        {
            bool tempRepeat = false;
            boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
            MissingInfoAboutPrevStateFromMove* missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
            GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone);
            int value;
            if (MoveLogic::Same(move2, move3, prevMove, *move)) {
                tempRepeat = true;
                value = 0;
                move->value = 0;
            } else {
                if (move->endPiece > 0 || move->promotionPiece > 0) {
                    delete MPValue;
                    MPValue = QSearch(false, -alpha - Option::nullWindowSize, -alpha, *move, depthGone + 1, nextLastCheck, true, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                    value = -MPValue->value;
                } else {
                    delete MPValue;
                    MPValue = QSearch(false, -alpha - Option::nullWindowSize, -alpha, *move, depthGone + 1, nextLastCheck, false, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                    value = -MPValue->value;
                }
                if (value != -160000) {
                    availMoves++;
                }
                if (value < -159800) {
                    value++;
                }
                move->value = value;
            }
            if (value > alpha /* && value < beta */) {
                if (!tempRepeat) {
                    if (move->endPiece > 0 || move->promotionPiece > 0) {
                        delete MPValue; 
                        MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, true, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                        value = -MPValue->value;
                    } else {
                        delete MPValue;
                        MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, false, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                        value = -MPValue->value;
                    }
                    if (value != -160000) {
                        availMoves++;
                    }
                    if (value < -159800) {
                        value++;
                    }
                    move->value = value;
                }
                if (value > alpha) {
                    alpha = value;
                }
            }
            GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
            delete missingInfoAboutPrevStateFromMove;
            missingInfoAboutPrevStateFromMove = nullptr;
            if (UCI::IsTest()) {
                Board::AreBoardsEqual(board4, *boardCopy);
                delete boardCopy;
                boardCopy = nullptr;
            }
            if (value > bestMoveValue) {
                if (value >= beta && ((value < 159800 && value > -159800) || !MAtESearch)) {
                    retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + MPValue->printString;
                    retValue->value = value;
                    deleteMoveList(moveList);
                    delete MPValue;
                    MPValue = nullptr;
                    return retValue;
                }
                bestMoveValue = value;
                SelectedMove = move;
                SelectedPV = MPValue->printString;
            }
        }
    }
    if (availMoves == 0 && !BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove)) {
        Move stalemateMove;
        stalemateMove.value = 0;
        stalemateMove.promotionPiece = -2;
        retValue->value = 0;
        deleteMoveList(moveList);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    } else if (availMoves == 0 && BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove)) {
        Move mateMove;
        mateMove.value = -159999;
        retValue->value = -159999;
        deleteMoveList(moveList);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    } else {
        retValue->value = bestMoveValue;
        retValue->printString = ChessStringManipulation::PVToString(*SelectedMove, 0, false, board4) + ' ' + SelectedPV;
        deleteMoveList(moveList);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    }
}

void QSearcher::deleteMoveList(MoveList moveList) {
    for (int i = 0; i < moveList.count; ++i) {
        delete moveList.moves[i];
    }
}