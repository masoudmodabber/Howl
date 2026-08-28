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
#include "RepetitionHistory.h"
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

#ifdef HOWL_CORRECTNESS_TESTING
namespace {
QSearchTestStatistics qSearchTestStatistics;
}

void QSearcher::ResetTestStatistics() {
    qSearchTestStatistics = {};
}

QSearchTestStatistics QSearcher::TestStatistics() {
    return qSearchTestStatistics;
}
#endif

MovePrintValue* QSearcher::QSearch(bool isPVNode, int alpha, int beta, Move& prevMove, int depthGone, int lastCheck, bool kick, int depth, Move& move1, Move& move2, Move& move3, Board& board4, bool MAtESearch, int depthQuisStarted, bool nullWindowSearch)
{
    Search::searchNodeCount++;
    MoveList moveList;
    Move* SelectedMove = nullptr;
    Move selectedMoveStorage{};
    Board* boardCopy;
    int extention = Option::checkExtensionNonPV;
    if (isPVNode) {
        extention = Option::checkExtension;
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
    
    bool currentSideInCheck = BoardLogic::UnderAttack(
        board4,
        board4.pieces[turn * 8 + 6].front(),
        !board4.sideToMove);

    if (depth == 0 && lastCheck == 0 && !kick && !currentSideInCheck) {
        retValue->value = EvaluationLogic::Evaluate(board4);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    }
    
    if (currentSideInCheck && lastCheck < 2 && depthGone - depthQuisStarted < extention) {
        delete retValue;
        retValue = nullptr;
        delete MPValue;
        MPValue = nullptr;
        return QSearch(isPVNode, alpha, beta, prevMove, depthGone, 2, false, 2, move1, move2, move3, board4, MAtESearch, depthQuisStarted, nullWindowSearch);
    }

    if (currentSideInCheck && depth == 0) {
        delete retValue;
        retValue = nullptr;
        delete MPValue;
        MPValue = nullptr;
        return QSearch(isPVNode, alpha, beta, prevMove, depthGone, 1, false, 1, move1, move2, move3, board4, MAtESearch, depthQuisStarted, nullWindowSearch);
    }
    
    int nextLastCheck = 0;
    bool checkChecked = false;
    if (lastCheck > 0) {
        checkChecked = true;
        nextLastCheck = lastCheck - 1;
    } else if (kick) {
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
    int valueTemp2 = -200000;
    if (!checkChecked && !currentSideInCheck) {
        valueTemp2 = EvaluationLogic::Evaluate(board4);
        if (valueTemp2 >= beta) {
            retValue->value = valueTemp2;
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
    }
    int bestMoveValue = -200000;
    DeferredMove deferredMoves[256];
    int deferredCount = 0;
    bool hasDeferredStage2 = false;

    if (!currentSideInCheck) {
        moveList = MoveLogic::QSearchStage1Generator(board4, depth, depthGone, deferredMoves, deferredCount, prevMove);
        hasDeferredStage2 = (deferredCount > 0);
    } else {
        moveList = MoveLogic::MoveGenerator(board4, depth, depthGone, false);
    }

#ifdef HOWL_CORRECTNESS_TESTING
    const bool testRootNode = depthGone == 0 && depthQuisStarted == 0;
    if (testRootNode) {
        qSearchTestStatistics.rootGeneratedMoves = moveList.count + deferredCount;
    }
#endif
    std::string SelectedPV = "";
    int availMoves = 0;
    int standPot = (!checkChecked && !currentSideInCheck) ? valueTemp2 : EvaluationLogic::Evaluate(board4);
    bool firstMove = true;

    int currentStage = 1;
    while (true) {
        for (int i = 0; i < moveList.count; ++i) {
            Move* move = moveList.moves[i];
            boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
            MissingInfoAboutPrevStateFromMove* missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);

            GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone);
            bool legalMove = !BoardLogic::UnderAttack(
                board4,
                board4.pieces[turn * 8 + 6].front(),
                board4.sideToMove);

            if (!legalMove) {
#ifdef HOWL_CORRECTNESS_TESTING
                if (testRootNode && firstMove) {
                    qSearchTestStatistics.rootIllegalMovesBeforeFirstSearch++;
                }
#endif
                GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                delete missingInfoAboutPrevStateFromMove;
                missingInfoAboutPrevStateFromMove = nullptr;
                if (UCI::IsTest()) {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                continue;
            }

            availMoves++;
#ifdef HOWL_CORRECTNESS_TESTING
            if (testRootNode) {
                qSearchTestStatistics.rootLegalMoves++;
            }
#endif
            bool moveGivesCheck = BoardLogic::UnderAttack(
                board4,
                board4.pieces[(1 - turn) * 8 + 6].front(),
                !board4.sideToMove);

            int pieceValueTemp = pieceValue100[move->endPiece];
            int promotionGain = (move->promotionPiece > 0)
                ? (pieceValue100[move->promotionPiece] - pieceValue100[1])
                : 0;

            bool deltaRejectsMaterial = (!currentSideInCheck &&
                Option::SafetyMargin + standPot + pieceValueTemp + promotionGain <= alpha);

#ifdef HOWL_CORRECTNESS_TESTING
            if (testRootNode && moveGivesCheck && deltaRejectsMaterial) {
                qSearchTestStatistics.checkingMovesExemptedFromDelta++;
            }
#endif

            bool isDeltaPruned = deltaRejectsMaterial && !moveGivesCheck;

            if (isDeltaPruned) {
                move->value = Option::SafetyMargin + standPot + pieceValueTemp + promotionGain - 1;
                if (move->value > bestMoveValue) {
                    bestMoveValue = move->value;
                    SelectedMove = move;
                    SelectedPV = "";
                }
                GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                delete missingInfoAboutPrevStateFromMove;
                missingInfoAboutPrevStateFromMove = nullptr;
                if (UCI::IsTest()) {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                continue;
            }

            if (firstMove) {
#ifdef HOWL_CORRECTNESS_TESTING
                if (testRootNode) {
                    qSearchTestStatistics.firstLegalSearchedMoveUsedFullWindow = true;
                }
#endif
                int value;
                std::string movePV = "";
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode)) {
                    value = 0;
                    move->value = 0;
                } else {
                    if (move->endPiece > 0 || move->promotionPiece > 0) {
                        delete MPValue;
                        MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, true, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                        value = -MPValue->value;
                        movePV = MPValue->printString;
                    } else {
                        delete MPValue;
                        MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, false, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                        value = -MPValue->value;
                        movePV = MPValue->printString;
                    }
                    if (value > 159800 && value != 160000) {
                        value--;
                    } else if (value < -159800 && value != -160000) {
                        value++;
                    }
                    move->value = value;
                }
                if (value > bestMoveValue) {
                    bestMoveValue = value;
                    SelectedMove = move;
                    SelectedPV = movePV;
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
                if (value > alpha) {
                    if (value >= beta && ((value < 159800 && value > -159800) || !MAtESearch)) {
                        retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + movePV;
                        retValue->value = value;
                        deleteMoveList(moveList);
                        if (hasDeferredStage2 && currentStage == 1) {
                            for (int d = 0; d < deferredCount; ++d) {
                                delete deferredMoves[d].templateMove;
                            }
                        }
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                    alpha = value;
                }
            }
            else
            {
                bool tempRepeat = false;
                int value;
                std::string movePV = "";
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode)) {
                    tempRepeat = true;
                    value = 0;
                    move->value = 0;
                } else {
                    if (move->endPiece > 0 || move->promotionPiece > 0) {
                        delete MPValue;
                        MPValue = QSearch(false, -alpha - Option::nullWindowSize, -alpha, *move, depthGone + 1, nextLastCheck, true, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                        value = -MPValue->value;
                        movePV = MPValue->printString;
                    } else {
                        delete MPValue;
                        MPValue = QSearch(false, -alpha - Option::nullWindowSize, -alpha, *move, depthGone + 1, nextLastCheck, false, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                        value = -MPValue->value;
                        movePV = MPValue->printString;
                    }
                    if (value > 159800 && value != 160000) {
                        value--;
                    } else if (value < -159800 && value != -160000) {
                        value++;
                    }
                    move->value = value;
                }
                if (value > alpha /* && value < beta */) {
                    if (!tempRepeat) {
#ifdef HOWL_CORRECTNESS_TESTING
                        if (testRootNode) {
                            qSearchTestStatistics.rootFullWindowResearches++;
                        }
#endif
                        if (move->endPiece > 0 || move->promotionPiece > 0) {
                            delete MPValue; 
                            MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, true, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                            value = -MPValue->value;
                            movePV = MPValue->printString;
                        } else {
                            delete MPValue;
                            MPValue = QSearch(isPVNode, -beta, -alpha, *move, depthGone + 1, nextLastCheck, false, depth - 1, move2, move3, prevMove, board4, false, depthQuisStarted, nullWindowSearch);
                            value = -MPValue->value;
                            movePV = MPValue->printString;
                        }
                        if (value > 159800 && value != 160000) {
                            value--;
                        } else if (value < -159800 && value != -160000) {
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
                        retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + movePV;
                        retValue->value = value;
                        deleteMoveList(moveList);
                        if (hasDeferredStage2 && currentStage == 1) {
                            for (int d = 0; d < deferredCount; ++d) {
                                delete deferredMoves[d].templateMove;
                            }
                        }
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                    bestMoveValue = value;
                    SelectedMove = move;
                    SelectedPV = movePV;
                }
            }
        }

        // If Stage 1 completed without beta cutoff, materialize Stage 2
        if (currentStage == 1 && hasDeferredStage2) {
            if (SelectedMove != nullptr) {
                selectedMoveStorage = *SelectedMove;
                SelectedMove = &selectedMoveStorage;
            }
            deleteMoveList(moveList);
            moveList = MoveLogic::MaterializeStage2(board4, depth, depthGone, deferredMoves, deferredCount);
            currentStage = 2;
            hasDeferredStage2 = false;
            continue;
        }

        break;
    }

#ifdef HOWL_CORRECTNESS_TESTING
    if (testRootNode) {
        qSearchTestStatistics.rootAvailableMoves = availMoves;
    }
#endif
    if (availMoves == 0 && !currentSideInCheck) {
        bool hasLegalMove = false;
        MoveList legalMoveList = MoveLogic::MoveGenerator(board4, depth, depthGone, false);
        for (int i = 0; i < legalMoveList.count && !hasLegalMove; ++i) {
            Move* move = legalMoveList.moves[i];
            MissingInfoAboutPrevStateFromMove undo(board4);
            GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone);
            hasLegalMove = !BoardLogic::UnderAttack(
                board4, board4.pieces[turn * 8 + 6].front(), board4.sideToMove);
            GameLogic::UndoMove(board4, *move, undo);
        }
        deleteMoveList(legalMoveList);
        retValue->value = hasLegalMove ? standPot : 0;
        deleteMoveList(moveList);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    } else if (availMoves == 0 && currentSideInCheck) {
        Move mateMove;
        mateMove.value = -159999;
        retValue->value = -159999;
        deleteMoveList(moveList);
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    } else {
        retValue->value = bestMoveValue;
        retValue->printString = (SelectedMove != nullptr) ? (ChessStringManipulation::PVToString(*SelectedMove, 0, false, board4) + ' ' + SelectedPV) : "";
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
