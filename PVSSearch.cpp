#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "PVSSearch.h"
#include "BoardLogic.h"
#include "EvaluationLogic.h"
#include "QSearcher.h"
#include "MoveLogic.h"
#include "UCI.h"
#include "GameLogic.h"
#include "ChessStringManipulation.h"
#include "MissingInfoAboutPrevStateFromMove.h"
#include "RepetitionHistory.h"
#include "TranspositionTable.h"
#include <iostream>
#include <algorithm>

int PVSSearch::moveOrderingDepth[20] = {
    1,
    2,
    2,
    3,
    3,
    4,
    4,
    5,
    5,
    6,
    6,
    7,
    7,
    8,
    8,
    9,
    9,
    10};

MovePrintValue *PVSSearch::PVS(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch)
{
    Move *SelectedMove = nullptr;
    Board *boardCopy = nullptr;

    MovePrintValue *retValue = new MovePrintValue();
    retValue->printString = "";

    MovePrintValue *MPValue = new MovePrintValue();
    MPValue->printString = "";

    int turn;
    if (!board4.sideToMove)
    {
        turn = 0;
    }
    else
    {
        turn = 1;
    }

    const int origAlpha = alpha;
    if (depth == 0)
    {
        delete retValue;
        retValue = nullptr;
        delete MPValue;
        MPValue = nullptr;
        return StartQSearch(isPVNode, alpha, beta, prevMove, depthGone, move1, move2, move3, board4, nullWindowSearch, previousMoveWasCheck);
    }

    if (BoardLogic::UnderAttack(board4, board4.pieces[(1 - turn) * 8 + 6].front(), board4.sideToMove))
    {
        retValue->value = 160000;
        delete MPValue;
        MPValue = nullptr;
        return retValue;
    }
    if (depth == 1 && BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
    {
        previousMoveWasCheck = true;
    }
    if (isNullMoveAllowed && !isPVNode && depth >= 4)
    {
        bool inCheck = BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove);
        if (!inCheck)
        {
            bool hasNonPawn = (board4.pieces[turn * 8 + 2].size() > 0 ||
                               board4.pieces[turn * 8 + 3].size() > 0 ||
                               board4.pieces[turn * 8 + 4].size() > 0 ||
                               board4.pieces[turn * 8 + 5].size() > 0);
            if (hasNonPawn)
            {
                int staticEval = EvaluationLogic::Evaluate(board4);
                if (staticEval >= beta)
                {
                    int R = (depth > 6) ? 4 : 3;
                    Move nullMove{};
                    nullMove.promotionPiece = -1;
                    MissingInfoAboutPrevStateFromMove undoInfo(board4);
                    GameLogic::DoMove(board4, nullMove, prevMove, depthGone, depthGone);

                    std::unique_ptr<MovePrintValue> nullRes(PVS(false, -beta, -beta + 1, depth - 1 - R, nullMove, move2, move3, prevMove, board4, MAtESearch, false, depthGone + 1, previousMoveWasCheck, true));
                    int nullScore = -nullRes->value;
                    if (nullScore > 159800 && nullScore != 160000)
                    {
                        nullScore--;
                    }
                    else if (nullScore < -159800 && nullScore != -160000)
                    {
                        nullScore++;
                    }

                    GameLogic::UndoMove(board4, nullMove, undoInfo);

                    if (nullScore >= beta)
                    {
                        retValue->value = (nullScore >= 159800) ? beta : nullScore;
                        retValue->printString = "null";
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                }
            }
        }
    }
    MoveList moveList = MoveLogic::MoveGenerator(board4, depth, depthGone);
    if constexpr (ProductionIGGEnabled)
    {
        IGG(isPVNode, alpha, beta, depth, prevMove, move1, move2, move3, board4, MAtESearch, isNullMoveAllowed, depthGone, previousMoveWasCheck, nullWindowSearch, moveList);
    }
    TTEntry ttEntry{};
    bool ttHit = TranspositionTable::Probe(board4.ZobristHashCode, ttEntry);
    if (TranspositionTable::CutoffsEnabled() && !isPVNode && ttHit && ttEntry.depth >= depth && TTFlagIsRigorous(ttEntry.flag))
    {
        if (ttEntry.flag == TT_EXACT)
        {
            TranspositionTable::RecordCutoff();
            retValue->value = ttEntry.score;
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else if (ttEntry.flag == TT_LOWER_BOUND && ttEntry.score >= beta)
        {
            TranspositionTable::RecordCutoff();
            retValue->value = ttEntry.score;
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else if (ttEntry.flag == TT_UPPER_BOUND && ttEntry.score <= alpha)
        {
            TranspositionTable::RecordCutoff();
            retValue->value = ttEntry.score;
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
    }
    if (ttHit && ttEntry.bestMove != 0)
    {
        int ttFrom = TTMoveHelper::UnpackFrom(ttEntry.bestMove);
        int ttTo = TTMoveHelper::UnpackTo(ttEntry.bestMove);
        int ttPromo = TTMoveHelper::UnpackPromotion(ttEntry.bestMove);
        for (int i = 0; i < moveList.count; ++i)
        {
            Move *m = moveList.moves[i];
            if (m->beginPlace == ttFrom && m->endPlace == ttTo &&
                (ttPromo == 0 ? (m->promotionPiece <= 0) : (m->promotionPiece == ttPromo)))
            {
                TranspositionTable::RecordHitStats(true, i == 0);
                if (i != 0)
                {
                    std::swap(moveList.moves[0], moveList.moves[i]);
                }
                break;
            }
        }
    }
    int inCheck = -1;
    int staticEval = -200000;
    int bestMoveValue = -200000;
    std::string SelectedPV = "";
    int availMoves = 0;
    int futilityPrunedCount = 0;
    int unverifiedLMRCount = 0;
    {
        bool firstMove = true;
        for (int i = 0; i < moveList.count; ++i)
        {
            Move *move = moveList.moves[i];
            int LMRDepth = 0;
            bool wasResearchedAtFullDepth = false;
            if (Search::overAllIteration == 1 && move->beginPlace == 60 && move->endPlace == 53)
            {
                //std::cout << "here" << std::endl;
                // std::cout << "Object is still in use after 15 seconds." << std::endl;
                //  throw std::runtime_error("Object is still in use after 15 seconds.");
            }
            if (move->beginPlace == 60 && move->endPlace == 53 && depth == 1 && depthGone == 1)
            {
                std::cout << "here" << std::endl;
                // std::cout << "Object is still in use after 15 seconds." << std::endl;
                //  throw std::runtime_error("Object is still in use after 15 seconds.");
            }
            if (firstMove)
            {
                boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
                GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone);
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                {
                    bestMoveValue = 0;
                    SelectedMove = move;
                    move->value = 0;
                }
                else
                {
                    bool tempPVNode = false;
                    if (isPVNode || move->isRefuteWithoutNullMove)
                    {
                        tempPVNode = true;
                    }
                    delete MPValue;
                    MPValue = PVS(tempPVNode, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, true, depthGone + 1, previousMoveWasCheck, nullWindowSearch);
                    bestMoveValue = -MPValue->value;
                    SelectedMove = move;
                    SelectedPV = MPValue->printString;
                    if (bestMoveValue != -160000)
                    {
                        availMoves++;
                    }
                    if (bestMoveValue > 159800 && bestMoveValue != 160000)
                    {
                        bestMoveValue--;
                    }
                    else if (bestMoveValue < -159800 && bestMoveValue != -160000)
                    {
                        bestMoveValue++;
                    }
                    move->value = bestMoveValue;
                }
                GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                delete missingInfoAboutPrevStateFromMove;
                missingInfoAboutPrevStateFromMove = nullptr;
                if (UCI::IsTest())
                {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                firstMove = false;
                if (bestMoveValue > alpha)
                {
                    if (bestMoveValue >= beta && ((bestMoveValue < 159800 && bestMoveValue > -159800) || !MAtESearch))
                    {
                        if (isPVNode)
                        {
                            move->isRefuteWithoutNullMove = true;
                        }
                        // TODO: remove rest of list from movelist for memory management
                        if (bestMoveValue != 0)
                        {
                            uint16_t packed = TTMoveHelper::PackMove(*move);
                            TranspositionTable::Store(board4.ZobristHashCode, move->value, depth, TT_LOWER_BOUND, packed);
                        }
                        retValue->value = move->value;
                        retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + MPValue->printString;
                        deleteMoveList(moveList);
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                    alpha = bestMoveValue;
                }
                move->isRefuteWithoutNullMove = false;
                // AddToTable(board4, move, depth, false, nullWindowSearch, depthGone);
            }
            else
            {
                if (!isPVNode && depth <= 2 && availMoves > 0 && move->endPiece == 0 && move->promotionPiece <= 0 &&
                    alpha > -159800 && beta < 159800)
                {
                    if (staticEval == -200000)
                    {
                        if (inCheck < 0)
                        {
                            inCheck = BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove) ? 1 : 0;
                        }
                        if (inCheck == 0)
                        {
                            staticEval = EvaluationLogic::Evaluate(board4);
                        }
                    }
                    if (inCheck == 0 && staticEval != -200000)
                    {
                        int futilityMargin = (depth == 1) ? 150 : 300;
                        if (staticEval + futilityMargin <= alpha)
                        {
                            futilityPrunedCount++;
                            continue;
                        }
                    }
                }

                bool tempRepeat = false;
                boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
                GameLogic::DoMove(board4, *move, prevMove, depth, depthGone);
                int value;
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                {
                    tempRepeat = true;
                    value = 0;
                    move->value = 0;
                }
                else
                {
                    bool tempPVNode = false;
                    if (move->isRefuteWithoutNullMove || (isPVNode && availMoves < 2))
                    {
                        tempPVNode = true;
                    }
                    bool exempt = false;
                    if (move->promotionPiece > 0)
                    {
                        exempt = true;
                    }
                    else if (move->endPiece > 0)
                    {
                        static const int pieceValLookup[8] = {0, 100, 320, 330, 500, 900, 20000, 0};
                        int movingPieceType = board4.mainBoard[move->endPlace] % 8;
                        int capturedPieceType = move->endPiece % 8;
                        if (pieceValLookup[capturedPieceType] >= pieceValLookup[movingPieceType])
                        {
                            exempt = true;
                        }
                    }
                    if (!tempPVNode && !exempt && depth >= 3)
                    {
                        LMRDepth = 1;
                    }
                    delete MPValue;
                    MPValue = PVS(tempPVNode, -alpha - Option::nullWindowSize, -alpha, depth - 1 - LMRDepth, *move, move2, move3, prevMove, board4, MAtESearch, true, depthGone + 1, previousMoveWasCheck, true);
                    value = -MPValue->value;
                    if (value != -160000)
                    {
                        availMoves++;
                    }
                    if (value > 159800 && value != 160000)
                    {
                        value--;
                    }
                    else if (value < -159800 && value != -160000)
                    {
                        value++;
                    }
                    move->value = value;

                    if (LMRDepth > 0 && value <= alpha)
                    {
                        unverifiedLMRCount++;
                    }
                }
                if (value > alpha /* && value < beta */)
                {
                    if (!tempRepeat)
                    {
                        bool tempPVNode = false;
                        if (isPVNode || move->isRefuteWithoutNullMove)
                        {
                            tempPVNode = true;
                        }
                        delete MPValue;
                        MPValue = PVS(tempPVNode, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, true, depthGone + 1, previousMoveWasCheck, nullWindowSearch);
                        wasResearchedAtFullDepth = true;
                        value = -MPValue->value;
                        if (value > 159800 && value != 160000)
                        {
                            value--;
                        }
                        else if (value < -159800 && value != -160000)
                        {
                            value++;
                        }
                        move->value = value;
                    }
                    if (value > alpha)
                    {
                        alpha = value;
                    }
                }
                GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                delete missingInfoAboutPrevStateFromMove;
                missingInfoAboutPrevStateFromMove = nullptr;
                if (UCI::IsTest())
                {
                    Board::AreBoardsEqual(board4, *boardCopy);
                    delete boardCopy;
                    boardCopy = nullptr;
                }
                if (value > bestMoveValue)
                {
                    if (value >= beta && ((value < 159800 && value > -159800) || !MAtESearch))
                    {
                        if (isPVNode)
                        {
                            move->isRefuteWithoutNullMove = true;
                        }
                        // TODO: delete movelist except first and second move
                        if (value != 0)
                        {
                            uint16_t packed = TTMoveHelper::PackMove(*move);
                            int storedDepth = (LMRDepth > 0 && !wasResearchedAtFullDepth) ? (depth - LMRDepth) : depth;
                            TranspositionTable::Store(board4.ZobristHashCode, move->value, storedDepth, TT_LOWER_BOUND, packed);
                        }
                        retValue->value = move->value;
                        retValue->printString = ChessStringManipulation::PVToString(*move, 0, false, board4) + ' ' + MPValue->printString;
                        deleteMoveList(moveList);
                        delete MPValue;
                        MPValue = nullptr;
                        return retValue;
                    }
                    move->isRefuteWithoutNullMove = false;
                    bestMoveValue = value;
                    SelectedMove = move;
                    SelectedPV = MPValue->printString;
                }
            }
        }
        if (availMoves == 0 && !BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
        {
            Move stalemateMove;
            stalemateMove.value = 0;
            stalemateMove.promotionPiece = -2;
            retValue->value = 0;
            deleteMoveList(moveList);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else if (availMoves == 0 && BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
        {
            Move mateMove;
            mateMove.value = -159999;
            retValue->value = -159999;
            deleteMoveList(moveList);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
        else
        {
            if (SelectedMove != nullptr && bestMoveValue != 0)
            {
                uint8_t flag = (bestMoveValue > origAlpha) ? TT_EXACT : TT_UPPER_BOUND;
                if (futilityPrunedCount > 0 || unverifiedLMRCount > 0)
                {
                    flag = static_cast<uint8_t>(flag + 4);
                }
                uint16_t packed = TTMoveHelper::PackMove(*SelectedMove);
                TranspositionTable::Store(board4.ZobristHashCode, bestMoveValue, depth, flag, packed);
            }
            retValue->value = bestMoveValue;
            retValue->printString = ChessStringManipulation::PVToString(*SelectedMove, 0, false, board4) + ' ' + SelectedPV;
            deleteMoveList(moveList);
            delete MPValue;
            MPValue = nullptr;
            return retValue;
        }
    }
}

void PVSSearch::deleteMoveList(MoveList moveList)
{
    for (int i = 0; i < moveList.count; ++i)
    {
        delete moveList.moves[i];
    }
}

void PVSSearch::NullMovePruning(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool mAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, MovePrintValue mPValue)
{
    /*if (depth == 1)
    {
        if (Evaluate(board4) + Option.futilityMargin <= alpha)
        {
            double valueRazored = qSearch(alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone);
            if (Evaluate(board4) + Option.futilityMargin > valueRazored)
            {
                return Evaluate(board4) + Option.futilityMargin;
            }
            else
            {
                return valueRazored;
            }
        }
    }
    if (depth == 2)
    {
        if (Evaluate(board4) + Option.extendedFutilityMargin <= alpha)
        {
            double valueRazored = qSearch(alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone);
            if (Evaluate(board4) + Option.extendedFutilityMargin > valueRazored)
            {
                return Evaluate(board4) + Option.extendedFutilityMargin;
            }
            else
            {
                return valueRazored;
            }
        }
    }
    if (depth == 3)
    {
        if (Evaluate(board4) + Option.superExtendedFutilityMargin <= alpha)
        {
            double valueRazored = qSearch(alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone);
            if (Evaluate(board4) + Option.superExtendedFutilityMargin > valueRazored)
            {
                return Evaluate(board4) + Option.superExtendedFutilityMargin;
            }
            else
            {
                return valueRazored;
            }
        }
    }
    int expectedValue = Evaluate(board4) + depth * 450;
    if (expectedValue <= alpha)
    {
        retValue.value = expectedValue;
        return retValue;
        //return expectedValue;
        //int valueRazored = qSearch(isPVNode, alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone, nullWindowSearch).value;
        //if (expectedValue > valueRazored)
        //{
        //    return expectedValue;
        //}
        //else
        //{
        //    return valueRazored;
        //}
    }*/
}

double PVSSearch::NullMoveReduction(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool mateSearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch)
{
    Move nullMove = Move();
    nullMove.promotionPiece = -1;

    Board *boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
    MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
    GameLogic::DoMove(board4, nullMove, prevMove, depthGone, depthGone);
    double valueReduced;
    std::unique_ptr<MovePrintValue> childResult;
    if (depth > 6)
    {
        childResult.reset(PVS(false, -beta, -beta + 1, depth - 4 - 1, nullMove, move2, move3, prevMove, board4, mateSearch, false, depthGone + 1, previousMoveWasCheck, true));
        valueReduced = -childResult->value;
    }
    else if (depth >= 4)
    {
        childResult.reset(PVS(false, -beta, -beta + 1, depth - 3 - 1, nullMove, move2, move3, prevMove, board4, mateSearch, false, depthGone + 1, previousMoveWasCheck, true));
        valueReduced = -childResult->value;
    }
    else
    {
        childResult.reset(QSearcher::QSearch(false, -beta, -beta + 1, nullMove, depthGone + 1, 0, true, 0, move2, move3, prevMove, board4, false, depthGone + 1, nullWindowSearch));
        valueReduced = -childResult->value;
    }
    GameLogic::UndoMove(board4, nullMove, *missingInfoAboutPrevStateFromMove);
    if (UCI::IsTest())
    {
        Board::AreBoardsEqual(board4, *boardCopy);
        delete boardCopy;
        boardCopy = nullptr;
    }
    delete missingInfoAboutPrevStateFromMove;
    missingInfoAboutPrevStateFromMove = nullptr;
    return valueReduced;
}

MovePrintValue *PVSSearch::StartQSearch(bool isPVNode, int alpha, int beta, Move &prevMove, int depthGone, Move &move1, Move &move2, Move &move3, Board &board4, bool nullWindowSearch, bool previousMoveWasCheck)
{
    if (previousMoveWasCheck)
    {
        return QSearcher::QSearch(isPVNode, alpha, beta, prevMove, depthGone, 1, false, 1, move1, move2, move3, board4, false, depthGone, nullWindowSearch);
    }
    // else if (evaluate(recDepth) > beta && isNullMoveAllowed)
    //{
    //     return evaluate(recDepth);
    // }
    if (prevMove.endPiece != 0 || prevMove.promotionPiece > 0)
    {
        return QSearcher::QSearch(isPVNode, alpha, beta, prevMove, depthGone, 0, true, 0, move1, move2, move3, board4, false, depthGone, nullWindowSearch);
    }
    else
    {
        return QSearcher::QSearch(isPVNode, alpha, beta, prevMove, depthGone, 0, false, 0, move1, move2, move3, board4, false, depthGone, nullWindowSearch);
    }
}

void PVSSearch::IGG(bool isPVNode, int alpha, int beta, int depth, Move &prevMove, Move &move1, Move &move2, Move &move3, Board &board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool lastCheck, bool nullWindowSearch, MoveList moveList)
{
    Board* boardCopy = nullptr; // <-- ADD THIS LINE
    MovePrintValue *MPValue = new MovePrintValue();
    int value = -200000;
    int tempDepth = depth;
    int bestMoveValue = -200000;
    depth = moveOrderingDepth[depth];
    {
        if (depth >= 3)
        {
            int availMovesIr = 0;
            bool firstMoveIr = true;
            {
                firstMoveIr = true;
                for (int i = 0; i < moveList.count; ++i)
                {
                    Move *move = moveList.moves[i];
                    if (firstMoveIr)
                    {
                        boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                        MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
                        GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone);
                        if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                        {
                            bestMoveValue = 0;
                            move->value = 0;
                        }
                        else
                        {
                            bool tempPVNode = false;
                            if (isPVNode || move->isRefuteWithoutNullMove)
                            {
                                tempPVNode = true;
                            }
                            delete MPValue;
                            MPValue = PVSSearch::PVS(tempPVNode, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, isNullMoveAllowed, depthGone + 1, lastCheck, nullWindowSearch);
                            bestMoveValue = -MPValue->value;
                            if (bestMoveValue != -160000)
                            {
                                availMovesIr++;
                            }
                            if (bestMoveValue > 159800 && bestMoveValue != 160000)
                            {
                                bestMoveValue--;
                            }
                            else if (bestMoveValue < -159800 && bestMoveValue != -160000)
                            {
                                bestMoveValue++;
                            }
                            move->value = bestMoveValue;
                        }
                        GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                        delete missingInfoAboutPrevStateFromMove;
                        missingInfoAboutPrevStateFromMove = nullptr;
                        if (UCI::IsTest())
                        {
                            Board::AreBoardsEqual(board4, *boardCopy);
                            delete boardCopy;
                            boardCopy = nullptr;
                        }
                        firstMoveIr = false;
                        if (bestMoveValue > alpha)
                        {
                            if (bestMoveValue >= beta && ((bestMoveValue < 159800 && bestMoveValue > -159800) || !MAtESearch))
                            {
                                if (isPVNode)
                                {
                                    move->isRefuteWithoutNullMove = true;
                                }
                            }
                            alpha = bestMoveValue;
                        }
                        move->isRefuteWithoutNullMove = false;
                        // AddToTable(board4, move, depth, false, nullWindowSearch, depthGone);
                    }
                    else
                    {
                        bool tempRepeat = false;
                        boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                        MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
                        GameLogic::DoMove(board4, *move, prevMove, depthGone, depthGone);
                        if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                        {
                            tempRepeat = true;
                            value = 0;
                            move->value = 0;
                        }
                        else
                        {
                            bool tempPVNode = false;
                            if (move->isRefuteWithoutNullMove || (isPVNode && availMovesIr < 2))
                            {
                                tempPVNode = true;
                            }
                            int LMRDepth = 0;
                            bool exempt = false;
                            if (move->promotionPiece > 0)
                            {
                                exempt = true;
                            }
                            else if (move->endPiece > 0)
                            {
                                static const int pieceValLookup[8] = {0, 100, 320, 330, 500, 900, 20000, 0};
                                int movingPieceType = board4.mainBoard[move->endPlace] % 8;
                                int capturedPieceType = move->endPiece % 8;
                                if (pieceValLookup[capturedPieceType] >= pieceValLookup[movingPieceType])
                                {
                                    exempt = true;
                                }
                            }
                            if (!tempPVNode && !exempt && depth >= 3)
                            {
                                LMRDepth = 1;
                            }
                            delete MPValue;
                            MPValue = PVSSearch::PVS(tempPVNode, -alpha - Option::nullWindowSize, -alpha, depth - 1 - LMRDepth, *move, move2, move3, prevMove, board4, MAtESearch, isNullMoveAllowed, depthGone + 1, lastCheck, true);
                            value = -MPValue->value;
                            if (value != -160000)
                            {
                                availMovesIr++;
                            }
                            if (value > 159800 && value != 160000)
                            {
                                value--;
                            }
                            else if (value < -159800 && value != -160000)
                            {
                                value++;
                            }
                            move->value = value;
                        }
                        if (value > alpha /* && value < beta */)
                        {
                            if (!tempRepeat)
                            {
                                bool tempPVNode = false;
                                if (isPVNode || move->isRefuteWithoutNullMove)
                                {
                                    tempPVNode = true;
                                }
                                delete MPValue;
                                MPValue = PVSSearch::PVS(tempPVNode, -beta, -alpha, depth - 1, *move, move2, move3, prevMove, board4, MAtESearch, isNullMoveAllowed, depthGone + 1, lastCheck, nullWindowSearch);
                                value = -MPValue->value;
                                if (value > 159800 && value != 160000)
                                {
                                    value--;
                                }
                                else if (value < -159800 && value != -160000)
                                {
                                    value++;
                                }
                                move->value = value;
                            }
                            if (value > alpha)
                            {
                                alpha = value;
                            }
                        }

                        GameLogic::UndoMove(board4, *move, *missingInfoAboutPrevStateFromMove);
                        delete missingInfoAboutPrevStateFromMove;
                        missingInfoAboutPrevStateFromMove = nullptr;
                        if (UCI::IsTest())
                        {
                            Board::AreBoardsEqual(board4, *boardCopy);
                            delete boardCopy;
                            boardCopy = nullptr;
                        }
                        if (value > bestMoveValue)
                        {
                            if (value >= beta && ((value < 159800 && value > -159800) || !MAtESearch))
                            {
                                if (isPVNode)
                                {
                                    move->isRefuteWithoutNullMove = true;
                                }
                            }
                            move->isRefuteWithoutNullMove = false;
                            // AddToTable(board4, move, depth, false, nullWindowSearch, depthGone);
                            bestMoveValue = value;
                        }
                    }
                }
                std::sort(moveList.moves, moveList.moves + moveList.count, [](Move *a, Move *b)
                          { return b->value > a->value; });
            }
        }
    }
    delete MPValue;
    MPValue = nullptr;
}
