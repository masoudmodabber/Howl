#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
// Search.cpp
#include "Search.h"
#include "MoveLogic.h"
#include "Option.h"
#include "BoardLogic.h"
#include <iostream>
#include "UCI.h"
#include "GameLogic.h"
#include "PVSSearch.h"
#include "ChessStringManipulation.h"
#include "MissingInfoAboutPrevStateFromMove.h"
#include "BoardInitializer.h"
#include "RepetitionHistory.h"
#include <algorithm>
#ifdef _WIN32
#include <crtdbg.h>
#endif
#include "KingSetup.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "DiagnosticLogger.h"
#include "MateScore.h"


std::atomic<bool> Search::active{false};
std::atomic<bool> Search::stopRequested{false};
time_t Search::beginTime{0};
std::chrono::high_resolution_clock::time_point Search::startTime;
double Search::allowedTime{0.0};
std::string Search::bestMove{""};
std::string Search::ponderMove{""};
std::string Search::completedBestMove{""};
std::string Search::completedPonderMove{""};
std::string Search::emergencyMove{""};
bool Search::finiteSearch{false};

int Search::maxDepth{-1};
int64_t Search::maxNodes{-1};
bool Search::isMoveTime{false};

int Search::overAllIteration = 0;
int Search::moveCount = 0;
int64_t Search::searchNodeCount = 0;
std::atomic<uint64_t> Search::tablebaseHits{0};
std::string Search::Score = "";
bool Search::mated = false;

void Search::CheckLimits()
{
    if (stopRequested.load(std::memory_order_relaxed))
        return;

    if (!active.load(std::memory_order_relaxed))
        return;

    if (maxNodes > 0 && searchNodeCount >= maxNodes)
    {
        stopRequested.store(true, std::memory_order_relaxed);
        active.store(false, std::memory_order_relaxed);
        return;
    }

    if (finiteSearch && allowedTime > 0)
    {
        auto now = std::chrono::high_resolution_clock::now();
        int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        if (elapsed >= allowedTime)
        {
            stopRequested.store(true, std::memory_order_relaxed);
            active.store(false, std::memory_order_relaxed);
        }
    }
}

namespace
{
    constexpr int FullSearchAlpha = -200000;
    constexpr int FullSearchBeta = 200000;
    constexpr int InitialAspirationDelta = 50;

    bool IsMateScore(int score)
    {
        return MateScore::IsMate(score);
    }

    SearchBound InvertBound(SearchBound bound)
    {
        if (bound == SearchBound::Lower)
            return SearchBound::Upper;
        if (bound == SearchBound::Upper)
            return SearchBound::Lower;
        return SearchBound::Exact;
    }

    bool AdvanceAspirationWindow(int score, bool exactMate, int& retriesUsed,
                                 int& alpha, int& beta)
    {
        if (exactMate || (score > alpha && score < beta))
            return false;

        const bool failLow = score <= alpha;
        const bool failHigh = score >= beta;
        if (!failLow && !failHigh)
            return false;

        if (retriesUsed == 0)
        {
            if (failHigh)
            {
                alpha = beta;
                beta = FullSearchBeta;
            }
            else
            {
                beta = alpha;
                alpha = FullSearchAlpha;
            }
            retriesUsed = 1;
            return true;
        }

        if (retriesUsed == 1)
        {
            alpha = FullSearchAlpha;
            beta = FullSearchBeta;
            retriesUsed = 2;
            return true;
        }

        return false;
    }
}

#if HOWL_CORRECTNESS_TESTING
std::vector<std::pair<int, int>> Search::AspirationWindowsForTesting(
    int previousScore, const std::vector<int>& searchScores)
{
    int alpha = std::max(FullSearchAlpha,
                         previousScore - InitialAspirationDelta);
    int beta = std::min(FullSearchBeta,
                        previousScore + InitialAspirationDelta);
    int retriesUsed = 0;
    std::vector<std::pair<int, int>> windows;
    for (int score : searchScores)
    {
        windows.emplace_back(alpha, beta);
        if (!AdvanceAspirationWindow(score, false, retriesUsed, alpha, beta))
            break;
    }
    return windows;
}
#endif

void Search::PrintBestMove()
{
    const std::string& outBest = !completedBestMove.empty() ? completedBestMove : (!bestMove.empty() ? bestMove : emergencyMove);
    const std::string& outPonder = !completedBestMove.empty() ? completedPonderMove : ponderMove;

    uint64_t sId = DiagnosticLogger::currentSearchId.load();
    if (outBest.empty())
    {
        DiagnosticLogger::Log("EMIT_BESTMOVE", "bestmove (none)", sId);
        std::cout << "bestmove (none)\n" << std::flush;
        return;
    }
    if (!outPonder.empty())
    {
        std::string bmStr = "bestmove " + outBest + " ponder " + outPonder;
        DiagnosticLogger::Log("EMIT_BESTMOVE", bmStr, sId);
        std::cout << bmStr << '\n' << std::flush;
    }
    else
    {
        std::string bmStr = "bestmove " + outBest;
        DiagnosticLogger::Log("EMIT_BESTMOVE", bmStr, sId);
        std::cout << bmStr << '\n' << std::flush;
    }
}

void Search::MainSearch(Move &move1, Move &move2, Move &move3, Move &move4, Board &board4)
{
    PVSSearch::ResetHistory();
    bestMove = "";
    ponderMove = "";
    completedBestMove = "";
    completedPonderMove = "";
    stopRequested.store(false, std::memory_order_relaxed);
    PVSSearch::ResetKillers();

    if (!active)
    {
        PrintBestMove();
        return;
    }
    if (RepetitionHistory::Size() == 0)
    {
        RepetitionHistory::ResetWithRoot(board4.ZobristHashCode);
    }
    int MultiPV = Option::MultiPV;
    MoveList moveList = MoveLogic::MoveGenerator(board4, -1, -1);
    if (moveList.count == 0)
    {
        active = false;
        PrintBestMove();
        return;
    }

    int rootTurn = board4.sideToMove ? 1 : 0;
    for (int i = 0; i < moveList.count; ++i)
    {
        Move *m = moveList.moves[i];
        MissingInfoAboutPrevStateFromMove undo(board4, *m);
        GameLogic::DoMove(board4, *m, move4, -2, -2, &undo);
        bool legal = !BoardLogic::UnderAttack(board4, board4.pieces[rootTurn * 8 + 6].front(), board4.sideToMove);
        GameLogic::UndoMove(board4, *m, undo);
        if (legal)
        {
            emergencyMove = ChessStringManipulation::PVToString(*m, 0, false, board4);
            break;
        }
    }

    int turn = board4.sideToMove ? 1 : 0;
    bool firstAssign = false;
    int recDepth = 1;
    int alpha = -200000;
    int beta = 200000;
    moveCount = 0;
    searchNodeCount = 0;
    tablebaseHits.store(0, std::memory_order_relaxed);

    bool previousMoveWasCheck = false;
    if (BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
    {
        previousMoveWasCheck = true;
    }
    
    bool depthOneExactMate = false;
    bool depthOneCompleted = SearchDepthZero(moveList, firstAssign, recDepth, alpha, beta, previousMoveWasCheck, move1, move2, move3, move4, board4, depthOneExactMate);

    if (depthOneCompleted)
    {
        completedBestMove = bestMove;
        completedPonderMove = ponderMove;
    }
    else
    {
        PrintBestMove();
        finiteSearch = false;
        active = false;
        PVSSearch::deleteMoveList(moveList);
        return;
    }

    if (!active || (maxDepth > 0 && maxDepth <= 1) || (maxNodes > 0 && moveCount >= maxNodes))
    {
        PrintBestMove();
        finiteSearch = false;
        active = false;
        PVSSearch::deleteMoveList(moveList);
        return;
    }

    if (finiteSearch && moveList.count == 1)
    {
        PrintBestMove();
        finiteSearch = false;
        active = false;
        PVSSearch::deleteMoveList(moveList);
        return;
    }

    if (finiteSearch && allowedTime > 0)
    {
        int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
        double limit = isMoveTime ? allowedTime : (allowedTime * 0.75);
        if (elapsed >= limit)
        {
            PrintBestMove();
            finiteSearch = false;
            active = false;
            PVSSearch::deleteMoveList(moveList);
            return;
        }
    }

    recDepth = 2;
    alpha = 0;
    beta = 0;
    int value = -200000;
    int prevCompletedScore = moveList.moves[0]->value;

    struct LastCompletedRootResult
    {
        int depth = 1;
        int score = 0;
        SearchBound bound = SearchBound::Exact;
        bool selective = true;
        bool exactMate = false;
        std::string pv;
        std::string bestMove;
        std::string ponderMove;
        std::string scoreText;
        bool mated = false;
    } lastCompletedRootResult;
    lastCompletedRootResult.score = prevCompletedScore;
    lastCompletedRootResult.pv = bestMove;
    lastCompletedRootResult.bestMove = bestMove;
    lastCompletedRootResult.ponderMove = ponderMove;
    lastCompletedRootResult.scoreText = Score;
    lastCompletedRootResult.mated = mated;

    const auto extractPv = [](const std::string &info)
    {
        const std::size_t pvStart = info.find(" pv ");
        const std::size_t scoreStart = info.rfind(" score ");
        return pvStart != std::string::npos && scoreStart != std::string::npos &&
                scoreStart > pvStart + 4
            ? info.substr(pvStart + 4, scoreStart - (pvStart + 4))
            : info;
    };

    MovePrintValue *MPValue = new MovePrintValue();
    MPValue->printString = "";
    struct CommittedMateResult
    {
        bool available = false;
        int score = 0;
        int mateDistance = 0;
        SearchBound bound = SearchBound::Exact;
        bool exactMate = false;
        std::string bestMove;
        std::string pv;
        std::string ponderMove;
        std::string formattedScore;
        bool mated = false;
    } committedMateResult;
    if (depthOneExactMate)
    {
        committedMateResult.available = true;
        committedMateResult.score = prevCompletedScore;
        committedMateResult.mateDistance = MateScore::MovesFromRootScore(prevCompletedScore);
        committedMateResult.exactMate = true;
        committedMateResult.bestMove = bestMove;
        committedMateResult.pv = bestMove;
        committedMateResult.ponderMove = ponderMove;
        committedMateResult.formattedScore = Score;
        committedMateResult.mated = mated;
        lastCompletedRootResult.selective = false;
        lastCompletedRootResult.exactMate = true;
    }

    while (active)
    {
        if (maxDepth > 0 && recDepth > maxDepth)
        {
            break;
        }

        SearchForCheckUpdate();

        std::vector<Move *> completedRootOrder(moveList.moves,
                                               moveList.moves + moveList.count);
        std::vector<Move> completedRootMoves;
        completedRootMoves.reserve(moveList.count);
        for (Move *rootMove : completedRootOrder)
            completedRootMoves.push_back(*rootMove);
        const auto restoreCompletedRootState = [&]()
        {
            for (int i = 0; i < moveList.count; ++i)
            {
                *completedRootOrder[i] = completedRootMoves[i];
                moveList.moves[i] = completedRootOrder[i];
            }
            bestMove = lastCompletedRootResult.bestMove;
            ponderMove = lastCompletedRootResult.ponderMove;
            Score = lastCompletedRootResult.scoreText;
            mated = lastCompletedRootResult.mated;
            completedBestMove = bestMove;
            completedPonderMove = ponderMove;
        };

        int aspAlpha = -200000;
        int aspBeta = +200000;
        if (Option::MultiPV <= 1 && prevCompletedScore > -159800 && prevCompletedScore < 159800)
        {
            aspAlpha = std::max(-200000,
                                prevCompletedScore - InitialAspirationDelta);
            aspBeta = std::min(200000,
                               prevCompletedScore + InitialAspirationDelta);
        }
        int aspirationRetriesUsed = 0;
        bool iterationCompleted = false;

        while (active.load(std::memory_order_relaxed))
        {
            if (Option::MultiPV > 1)
            {
                int K = std::min(Option::MultiPV, moveList.count);
                std::vector<MovePrintValue *> topMoves;
                int KthBestValue = -200000;
                bool allRootMovesCompleted = true;

                for (int counter = 0; counter < moveList.count; counter++)
                {
                    if (stopRequested.load(std::memory_order_relaxed) || !active.load(std::memory_order_relaxed))
                    {
                        stopRequested = true;
                        allRootMovesCompleted = false;
                        break;
                    }
                    if (maxNodes > 0 && moveCount >= maxNodes)
                    {
                        stopRequested = true;
                        allRootMovesCompleted = false;
                        break;
                    }
                    if (finiteSearch && allowedTime > 0)
                    {
                        int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
                        if (elapsed >= allowedTime)
                        {
                            active.store(false, std::memory_order_relaxed);
                            stopRequested = true;
                            allRootMovesCompleted = false;
                            break;
                        }
                    }

                    Move *move = moveList.moves[counter];
                    bool rootMoveExactMate = false;
                    bool rootMoveRepetitionResult = false;
                    int value = -200000;

                    Board *boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                    MissingInfoAboutPrevStateFromMove *missingInfo = new MissingInfoAboutPrevStateFromMove(board4, *move);
                    GameLogic::DoMove(board4, *move, move4, -1, -1, missingInfo);

                    if (counter < K)
                    {
                        if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                        {
                            value = 0;
                            move->value = 0;
                            rootMoveRepetitionResult = true;
                        }
                        else
                        {
                            delete MPValue;
                            MPValue = PVSSearch::PVS(true, -200000, 200000, recDepth - 1, *move, move2, move3, move4, board4, false, true, 1, false, false);
                            rootMoveExactMate = MPValue->bound == SearchBound::Exact &&
                                !MPValue->selective && IsMateScore(-MPValue->value);
                            value = -MPValue->value;
                            move->value = value;
                        }

                        GameLogic::UndoMove(board4, *move, *missingInfo);
                        delete missingInfo;
                        missingInfo = nullptr;
                        if (UCI::IsTest())
                        {
                            Board::AreBoardsEqual(board4, *boardCopy);
                            delete boardCopy;
                            boardCopy = nullptr;
                        }

                        CalculateAndDisplayScore(move->value, rootMoveExactMate);
                        MovePrintValue *movePrint = new MovePrintValue();
                        movePrint->value = move->value;
                        movePrint->bound = rootMoveRepetitionResult ? SearchBound::Exact : InvertBound(MPValue->bound);
                        movePrint->selective = MPValue ? MPValue->selective : false;
                        int64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
                        int64_t safeNodeCount = searchNodeCount;
                        int64_t nps = (elapsed_ms > 0) ? (safeNodeCount * 1000LL / elapsed_ms) : 0;
                        movePrint->depth = recDepth;
                        movePrint->elapsed_ms = elapsed_ms;
                        movePrint->nodes = safeNodeCount;
                        movePrint->nps = nps;
                        movePrint->scoreText = Score;
                        std::string rootMoveStr = ChessStringManipulation::PVToString(*move, 0, false, board4);
                        std::string childPV = (MPValue && !MPValue->printString.empty()) ? MPValue->printString : "";
                        movePrint->pv = childPV.empty() ? rootMoveStr : (rootMoveStr + " " + childPV);
                        movePrint->printString = "info depth " + std::to_string(recDepth) + " time " +
                            std::to_string(elapsed_ms) + " nodes " + std::to_string(safeNodeCount) +
                            " nps " + std::to_string(nps) + " pv " + movePrint->pv + " score " + Score;

                        topMoves.push_back(movePrint);

                        if (counter == K - 1)
                        {
                            std::sort(topMoves.begin(), topMoves.end(), [](const MovePrintValue *a, const MovePrintValue *b) {
                                return b->value < a->value;
                            });
                            KthBestValue = topMoves.back()->value;
                            bestMove = Parse(topMoves[0]->pv, 1);
                            ponderMove = Parse(topMoves[0]->pv, 2);
                            PrintKBest(topMoves, K, finiteSearch);
                        }
                    }
                    else
                    {
                        bool needFullSearch = false;
                        if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                        {
                            value = 0;
                            move->value = 0;
                            if (value > KthBestValue)
                            {
                                needFullSearch = true;
                                rootMoveRepetitionResult = true;
                            }
                        }
                        else
                        {
                            delete MPValue;
                            MPValue = PVSSearch::PVS(false, -KthBestValue - Option::nullWindowSize, -KthBestValue, recDepth - 1, *move, move2, move3, move4, board4, false, true, 1, false, true);
                            value = -MPValue->value;
                            move->value = value;
                            if (value > KthBestValue)
                            {
                                needFullSearch = true;
                            }
                        }

                        if (needFullSearch && !rootMoveRepetitionResult)
                        {
                            delete MPValue;
                            MPValue = PVSSearch::PVS(true, -200000, 200000, recDepth - 1, *move, move2, move3, move4, board4, false, true, 1, false, false);
                            rootMoveExactMate = MPValue->bound == SearchBound::Exact &&
                                !MPValue->selective && IsMateScore(-MPValue->value);
                            value = -MPValue->value;
                            move->value = value;
                        }

                        GameLogic::UndoMove(board4, *move, *missingInfo);
                        delete missingInfo;
                        missingInfo = nullptr;
                        if (UCI::IsTest())
                        {
                            Board::AreBoardsEqual(board4, *boardCopy);
                            delete boardCopy;
                            boardCopy = nullptr;
                        }

                        if (needFullSearch && move->value > KthBestValue)
                        {
                            CalculateAndDisplayScore(move->value, rootMoveExactMate);
                            MovePrintValue *movePrint = new MovePrintValue();
                            movePrint->value = move->value;
                            movePrint->bound = rootMoveRepetitionResult ? SearchBound::Exact : InvertBound(MPValue->bound);
                            movePrint->selective = MPValue ? MPValue->selective : false;
                            int64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
                            int64_t safeNodeCount = searchNodeCount;
                            int64_t nps = (elapsed_ms > 0) ? (safeNodeCount * 1000LL / elapsed_ms) : 0;
                            movePrint->depth = recDepth;
                            movePrint->elapsed_ms = elapsed_ms;
                            movePrint->nodes = safeNodeCount;
                            movePrint->nps = nps;
                            movePrint->scoreText = Score;
                            std::string rootMoveStr = ChessStringManipulation::PVToString(*move, 0, false, board4);
                            std::string childPV = (MPValue && !MPValue->printString.empty()) ? MPValue->printString : "";
                            movePrint->pv = childPV.empty() ? rootMoveStr : (rootMoveStr + " " + childPV);
                            movePrint->printString = "info depth " + std::to_string(recDepth) + " time " +
                                std::to_string(elapsed_ms) + " nodes " + std::to_string(safeNodeCount) +
                                " nps " + std::to_string(nps) + " pv " + movePrint->pv + " score " + Score;

                            delete topMoves.back();
                            topMoves.back() = movePrint;

                            std::sort(topMoves.begin(), topMoves.end(), [](const MovePrintValue *a, const MovePrintValue *b) {
                                return b->value < a->value;
                            });
                            KthBestValue = topMoves.back()->value;
                            bestMove = Parse(topMoves[0]->pv, 1);
                            ponderMove = Parse(topMoves[0]->pv, 2);
                            PrintKBest(topMoves, K, finiteSearch);
                        }
                    }
                }

                if (stopRequested || !allRootMovesCompleted || !active.load(std::memory_order_relaxed))
                {
                    deleteMovesPrintValue(topMoves);
                    break;
                }

                PrintKBest(topMoves, K, finiteSearch);

                completedBestMove = Parse(topMoves[0]->pv, 1);
                completedPonderMove = Parse(topMoves[0]->pv, 2);
                bestMove = completedBestMove;
                ponderMove = completedPonderMove;

                lastCompletedRootResult.depth = recDepth;
                lastCompletedRootResult.score = topMoves[0]->value;
                lastCompletedRootResult.bound = SearchBound::Exact;
                lastCompletedRootResult.selective = false;
                lastCompletedRootResult.exactMate = false;
                lastCompletedRootResult.pv = topMoves[0]->pv;
                lastCompletedRootResult.bestMove = completedBestMove;
                lastCompletedRootResult.ponderMove = completedPonderMove;
                lastCompletedRootResult.scoreText = topMoves[0]->scoreText;

                deleteMovesPrintValue(topMoves);
                prevCompletedScore = lastCompletedRootResult.score;
                iterationCompleted = true;

                std::sort(moveList.moves, moveList.moves + moveList.count, [](Move *a, Move *b) {
                    return b->value < a->value;
                });
                break;
            }

            alpha = aspAlpha;
            beta = aspBeta;
            value = -200000;
            std::vector<MovePrintValue *> movesPrintValue;
            int KthBestValue = (Option::MultiPV <= 1) ? aspAlpha : -200000;
            std::string bestPVString = "";
            bool iterationSelective = false;
            bool iterationHasAuthoritativeResult = false;
            int authoritativeIterationScore = -200000;
            bool allRootMovesCompleted = true;

            for (int counter = 0; counter < moveList.count; counter++)
            {
                if (stopRequested.load(std::memory_order_relaxed) || !active.load(std::memory_order_relaxed))
                {
                    stopRequested = true;
                    allRootMovesCompleted = false;
                    break;
                }
                if (maxNodes > 0 && moveCount >= maxNodes)
                {
                    stopRequested = true;
                    allRootMovesCompleted = false;
                    break;
                }
                if (finiteSearch && allowedTime > 0)
                {
                    int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
                    if (elapsed >= allowedTime)
                    {
                        active.store(false, std::memory_order_relaxed);
                        stopRequested = true;
                        allRootMovesCompleted = false;
                        break;
                    }
                }

                Move *move = moveList.moves[counter];
                bool rootMoveReceivedFullSearch = false;
                bool rootMoveExactMate = false;
                bool rootMoveAuthoritativeResult = false;
                bool rootMoveRepetitionResult = false;
                if (recDepth == 2 && move->beginPlace == 17 && move->endPlace == 53)
                {
                    overAllIteration++;
                }

                if (counter < MultiPV)
                {
                    Board *boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                    MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4, *move);
                    GameLogic::DoMove(board4, *move, move4, -1, -1, missingInfoAboutPrevStateFromMove);
                    if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                    {
                        value = 0;
                        move->value = 0;
                        rootMoveAuthoritativeResult = true;
                        rootMoveRepetitionResult = true;
                    }
                    else
                    {
                        delete MPValue;
                        MPValue = PVSSearch::PVS(true, -beta, -alpha, recDepth - 1, *move, move2, move3, move4, board4, false, true, 1, false, false);
                        rootMoveReceivedFullSearch = true;
                        rootMoveExactMate = MPValue->bound == SearchBound::Exact &&
                            !MPValue->selective && IsMateScore(-MPValue->value);
                        rootMoveAuthoritativeResult = true;
                        value = -MPValue->value;
                        move->value = value;
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
                if (value > alpha)
                {
                    alpha = value;
                    iterationSelective = rootMoveReceivedFullSearch && MPValue->selective;
                    bestMove = ChessStringManipulation::PVToString(*move, 0, false, board4);
                    if (MPValue->printString.length() > 1)
                    {
                        ponderMove = Parse(MPValue->printString, 1);
                    }
                    else
                    {
                        ponderMove = "";
                    }
                }
                CalculateAndDisplayScore(move->value, rootMoveExactMate);
                MovePrintValue *movePrint = new MovePrintValue();
                movePrint->value = move->value;
                movePrint->bound = rootMoveRepetitionResult
                    ? SearchBound::Exact
                    : InvertBound(MPValue->bound);
                movePrint->selective = rootMoveReceivedFullSearch && MPValue->selective;
                int64_t elapsed_ms = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count());
                int64_t safeNodeCount = searchNodeCount;
                int64_t nps = (elapsed_ms > 0) ? (safeNodeCount * 1000LL / elapsed_ms) : 0;
                if (nps < 0) {
                    std::cerr << "[DEBUG] Negative nps detected! searchNodeCount=" << safeNodeCount << ", elapsed_ms=" << elapsed_ms << std::endl;
                }
                movePrint->depth = recDepth;
                movePrint->elapsed_ms = elapsed_ms;
                movePrint->nodes = searchNodeCount;
                movePrint->nps = nps;
                movePrint->scoreText = Score;
                movePrint->pv = ChessStringManipulation::PVToString(*move, 1, mated, board4) + ' ' + MPValue->printString;
                movePrint->printString = "info depth " + std::to_string(recDepth) + " time " +
                    std::to_string(elapsed_ms) +
                    " nodes " + std::to_string(searchNodeCount) + " nps " +
                    std::to_string(nps) +
                    " pv " + movePrint->pv + " score " + Score;
                if (ChessStringManipulation::PVToString(*move, 0, false, board4) == bestMove)
                {
                    bestPVString = movePrint->printString;
                }
                movesPrintValue.push_back(movePrint);
                if (Option::MultiPV > 1)
                {
                    KthBestValue = PrintKBest(movesPrintValue, MultiPV, finiteSearch);
                }
                else if (moveList.count == 1)
                {
                    KthBestValue = value;
                }
                else
                {
                    KthBestValue = value;
                }
            }
            else
            {
                Board *boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4, *move);
                GameLogic::DoMove(board4, *move, move4, -1, -1, missingInfoAboutPrevStateFromMove);
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                {
                    value = 0;
                    move->value = 0;
                    rootMoveAuthoritativeResult = true;
                    rootMoveRepetitionResult = true;
                }
                else
                {
                    bool tempPVNode = false;
                    delete MPValue;
                    MPValue = PVSSearch::PVS(tempPVNode, -KthBestValue - Option::nullWindowSize, -KthBestValue, recDepth - 1, *move, move2, move3, move4, board4, false, true, 1, false, true);
                    rootMoveReceivedFullSearch = true;
                    const int initialRootValue = -MPValue->value;
                    value = initialRootValue;
                    move->value = value;
                    if (Option::MultiPV > 1)
                    {
                        if (value > KthBestValue)
                        {
                            delete MPValue;
                            MPValue = PVSSearch::PVS(true, -200000, 200000, recDepth - 1, *move, move2, move3, move4, board4, false, true, 1, false, false);
                            rootMoveReceivedFullSearch = true;
                            rootMoveExactMate = MPValue->bound == SearchBound::Exact &&
                                !MPValue->selective && IsMateScore(-MPValue->value);
                            value = -MPValue->value;
                            move->value = value;
                        }
                    }
                    else if (value > KthBestValue)
                    {
                        delete MPValue;
                        MPValue = PVSSearch::PVS(true, -beta, -alpha, recDepth - 1, *move, move2, move3, move4, board4, false, true, 1, false, false);
                        rootMoveReceivedFullSearch = true;
                        rootMoveExactMate = MPValue->bound == SearchBound::Exact &&
                            !MPValue->selective && IsMateScore(-MPValue->value);
                        value = -MPValue->value;
                        move->value = value;
                    }
                    rootMoveAuthoritativeResult = true;
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
                mated = false;
                Score = "";
                if (rootMoveExactMate && move->value > MateScore::Threshold)
                {
                    Score = "mate " + std::to_string(MateScore::MovesFromRootScore(move->value));
                    mated = true;
                }
                else if (rootMoveExactMate && move->value < -MateScore::Threshold)
                {
                    Score = "mate -" + std::to_string(MateScore::MovesFromRootScore(move->value));
                    mated = true;
                }
                else
                {
                    Score = "cp " + std::to_string(move->value);
                }
                MovePrintValue *movePrint = new MovePrintValue();
                movePrint->value = move->value;
                movePrint->bound = rootMoveRepetitionResult
                    ? SearchBound::Exact
                    : InvertBound(MPValue->bound);
                movePrint->selective = rootMoveReceivedFullSearch && MPValue->selective;
                int64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
                int64_t safeNodeCount = searchNodeCount;
                int64_t nps = (elapsed_ms > 0) ? (safeNodeCount * 1000LL / elapsed_ms) : 0;
                movePrint->depth = recDepth;
                movePrint->elapsed_ms = elapsed_ms;
                movePrint->nodes = searchNodeCount;
                movePrint->nps = nps;
                movePrint->scoreText = Score;
                movePrint->pv = ChessStringManipulation::PVToString(*move, 1, mated, board4) + ' ' + MPValue->printString;
                movePrint->printString = "info depth " + std::to_string(recDepth) + " time " +
                    std::to_string(elapsed_ms) +
                    " nodes " + std::to_string(searchNodeCount) + " nps " +
                    std::to_string(nps) +
                    " pv " + movePrint->pv + " score " + Score;
                movesPrintValue.push_back(movePrint);
                if (value > alpha)
                {
                    alpha = value;
                    iterationSelective = rootMoveReceivedFullSearch && MPValue->selective;
                    bestMove = ChessStringManipulation::PVToString(*move, 0, false, board4);
                    bestPVString = movePrint->printString;
                    if (MPValue->printString.length() > 7)
                    {
                        ponderMove = Parse(MPValue->printString, 1);
                    }
                    else
                    {
                        ponderMove = "";
                    }
                }
                if (Option::MultiPV > 1)
                {
                    if (move->value > KthBestValue)
                    {
                        KthBestValue = PrintKBest(movesPrintValue, MultiPV, finiteSearch);
                    }
                }
                else
                {
                    if (value > KthBestValue)
                    {
                        KthBestValue = value;
                    }
                }
            }

            if (rootMoveAuthoritativeResult)
            {
                if (!iterationHasAuthoritativeResult || move->value > authoritativeIterationScore)
                    authoritativeIterationScore = move->value;
                iterationHasAuthoritativeResult = true;
            }


            if (stopRequested.load(std::memory_order_relaxed) || !active.load(std::memory_order_relaxed))
            {
                stopRequested = true;
                allRootMovesCompleted = false;
                break;
            }
            if (maxNodes > 0 && moveCount >= maxNodes)
            {
                stopRequested = true;
                allRootMovesCompleted = false;
                break;
            }
            if (finiteSearch && allowedTime > 0)
            {
                int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
                if (elapsed >= allowedTime)
                {
                    active.store(false, std::memory_order_relaxed);
                    stopRequested = true;
                    allRootMovesCompleted = false;
                    break;
                }
            }
        }
        if (stopRequested || !allRootMovesCompleted || !active.load(std::memory_order_relaxed))
        {
            deleteMovesPrintValue(movesPrintValue);
            break;
        }
        if (!stopRequested && Option::MultiPV > 1)
        {
            if (!movesPrintValue.empty())
            {
                PrintKBest(movesPrintValue, MultiPV, finiteSearch);
            }
        }

            int iterScore = iterationHasAuthoritativeResult
                ? authoritativeIterationScore
                : moveList.moves[0]->value;
            bool iterationMateExact = IsMateScore(iterScore);
            bool exactBestMoveFound = false;
            for (const MovePrintValue *rootResult : movesPrintValue)
            {
                if (rootResult->selective ||
                    (rootResult->bound != SearchBound::Exact &&
                     rootResult->bound != SearchBound::Upper) ||
                    rootResult->value > iterScore)
                {
                    iterationMateExact = false;
                }
                if (!rootResult->selective &&
                    rootResult->bound == SearchBound::Exact &&
                    rootResult->value == iterScore)
                {
                    exactBestMoveFound = true;
                }
            }
            iterationMateExact = iterationMateExact && exactBestMoveFound &&
                movesPrintValue.size() == static_cast<std::size_t>(moveList.count);

            if (Option::MultiPV <= 1 && !iterationMateExact)
            {
                if (AdvanceAspirationWindow(iterScore, false,
                                            aspirationRetriesUsed,
                                            aspAlpha, aspBeta))
                {
                    deleteMovesPrintValue(movesPrintValue);
                    continue;
                }
            }

            const bool completedExactMate = iterationMateExact;
            CalculateAndDisplayScore(iterScore, completedExactMate);
            std::string completedInfo = !bestPVString.empty()
                ? bestPVString
                : (!movesPrintValue.empty() ? movesPrintValue[0]->printString : "");
            const std::size_t completedScoreStart = completedInfo.rfind(" score ");
            if (completedScoreStart != std::string::npos)
            {
                completedInfo.erase(completedScoreStart);
                completedInfo += " score " + Score;
            }

            bool currentMateAccepted = false;
            if (completedExactMate)
            {
                const int currentMateDistance = MateScore::MovesFromRootScore(iterScore);
                const bool sameSign = committedMateResult.available &&
                    ((iterScore > 0) == (committedMateResult.score > 0));
                const bool sameOrShorter = sameSign &&
                    currentMateDistance <= committedMateResult.mateDistance;
                if (!committedMateResult.available || sameOrShorter)
                {
                    currentMateAccepted = true;
                    committedMateResult.available = true;
                    committedMateResult.score = iterScore;
                    committedMateResult.mateDistance = currentMateDistance;
                    committedMateResult.bound = SearchBound::Exact;
                    committedMateResult.exactMate = true;
                    committedMateResult.bestMove = bestMove;
                    committedMateResult.pv = extractPv(completedInfo);
                    committedMateResult.ponderMove = ponderMove;
                    committedMateResult.formattedScore = Score;
                    committedMateResult.mated = mated;
                }
            }

            int emittedScore = iterScore;
            bool emittedExactMate = completedExactMate;
            bool emittedSelective = iterationSelective;
            if (committedMateResult.available && !currentMateAccepted)
            {
                emittedScore = committedMateResult.score;
                emittedExactMate = committedMateResult.exactMate;
                emittedSelective = false;
                bestMove = committedMateResult.bestMove;
                ponderMove = committedMateResult.ponderMove;
                Score = committedMateResult.formattedScore;
                mated = committedMateResult.mated;

                const int64_t elapsed_ms = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() - startTime).count());
                const int64_t nps = elapsed_ms > 0 ? searchNodeCount * 1000LL / elapsed_ms : 0;
                completedInfo = "info depth " + std::to_string(recDepth) +
                    " time " + std::to_string(elapsed_ms) +
                    " nodes " + std::to_string(searchNodeCount) +
                    " nps " + std::to_string(nps) +
                    " pv " + committedMateResult.pv +
                    " score " + Score;
            }

            if (Option::MultiPV <= 1)
            {
                completedInfo += " tbhits " + std::to_string(
                    tablebaseHits.load(std::memory_order_relaxed));
                DiagnosticLogger::Log("EMIT_INFO", completedInfo, DiagnosticLogger::currentSearchId.load());
                std::cout << completedInfo << '\n';
            }

            lastCompletedRootResult.depth = recDepth;
            lastCompletedRootResult.score = emittedScore;
            lastCompletedRootResult.bound = committedMateResult.available && !currentMateAccepted
                ? committedMateResult.bound
                : SearchBound::Exact;
            lastCompletedRootResult.selective = emittedSelective;
            lastCompletedRootResult.exactMate = emittedExactMate;
            lastCompletedRootResult.pv = extractPv(completedInfo);
            lastCompletedRootResult.bestMove = bestMove;
            lastCompletedRootResult.ponderMove = ponderMove;
            lastCompletedRootResult.scoreText = Score;
            lastCompletedRootResult.mated = mated;
            deleteMovesPrintValue(movesPrintValue);

            prevCompletedScore = emittedScore;
            iterationCompleted = true;
            break;
        }

        if (stopRequested || !active.load(std::memory_order_relaxed) || !iterationCompleted)
        {
            restoreCompletedRootState();
            break;
        }
        completedBestMove = bestMove;
        completedPonderMove = ponderMove;
        std::sort(moveList.moves, moveList.moves + moveList.count, [](Move *a, Move *b)
                  { return b->value < a->value; });
        if (!bestMove.empty())
        {
            for (int i = 0; i < moveList.count; ++i)
            {
                if (ChessStringManipulation::PVToString(*moveList.moves[i], 0, false, board4) == bestMove)
                {
                    if (i != 0)
                    {
                        std::swap(moveList.moves[0], moveList.moves[i]);
                    }
                    break;
                }
            }
        }

        if (finiteSearch && allowedTime > 0)
        {
            int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
            double softLimit = isMoveTime ? allowedTime : (allowedTime * 0.75);
            if (elapsed >= softLimit)
            {
                break;
            }
        }

        if (maxDepth > 0 && recDepth >= maxDepth)
        {
            break;
        }

        recDepth++;
    }

    PrintBestMove();
    finiteSearch = false;
    active = false;
    PVSSearch::deleteMoveList(moveList);
    if (MPValue != nullptr)
    {
        delete MPValue;
        MPValue = nullptr;
    }
}

void Search::deleteMovesPrintValue(std::vector<MovePrintValue *> &movesPrintValue)
{
    for (MovePrintValue *movePrintValue : movesPrintValue)
    {
        delete movePrintValue;
    }
    movesPrintValue.clear();
}

bool Search::SearchDepthZero(MoveList &moveList, bool &firstAssign, int &recDepth, int &alpha, int &beta, bool &previousMoveWasCheck, Move &move1, Move &move2, Move &move3, Move &move4, Board &board4, bool &exactMate)
{
    std::vector<Move> movesToDelete;
    int turn = board4.sideToMove ? 1 : 0;
    exactMate = false;
    bool completedAll = true;
    for (int i = 0; i < moveList.count; ++i)
    {
        if (!active.load(std::memory_order_relaxed))
        {
            completedAll = false;
            break;
        }
        Move *move = moveList.moves[i];
        auto boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
        MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4, *move);
        GameLogic::DoMove(board4, *move, move4, -2, -2, missingInfoAboutPrevStateFromMove);
        if (!BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), board4.sideToMove))
        {
            if (!firstAssign)
            {
                bestMove = ChessStringManipulation::PVToString(*move, 0, false, board4);
                firstAssign = true;
            }

            int curAlpha = (Option::MultiPV > 1) ? -200000 : alpha;
            if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
            {
                move->value = 0;
                if (move->value > alpha)
                    exactMate = false;
            }
            else
            {
                MovePrintValue *tempRetValLocal = PVSSearch::PVS(true, -beta, -curAlpha, 0, *move, move2, move3, move4, board4, false, true, 1, previousMoveWasCheck, false);
                move->value = -tempRetValLocal->value;
                if (move->value > alpha)
                {
                    exactMate = tempRetValLocal->bound == SearchBound::Exact &&
                        !tempRetValLocal->selective && IsMateScore(move->value);
                }
                delete tempRetValLocal;
                tempRetValLocal = nullptr;
            }

            if (Option::MultiPV <= 1 && move->value > alpha)
            {
                alpha = move->value;
            }
            // No need to update value in array, already updated
        }
        else
        {
            movesToDelete.push_back(*move);
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
        if (!active.load(std::memory_order_relaxed))
        {
            completedAll = false;
            break;
        }
    }

    if (!completedAll)
    {
        return false;
    }

    // Remove and delete moves marked for deletion
    for (const Move& delMove : movesToDelete) {
        for (int i = 0; i < moveList.count; ) {
            Move* m = moveList.moves[i];
            if (m->beginPlace == delMove.beginPlace && m->endPlace == delMove.endPlace && m->promotionPiece == delMove.promotionPiece) {
                delete m;
                for (int j = i; j < moveList.count - 1; ++j) {
                    moveList.moves[j] = moveList.moves[j + 1];
                }
                --moveList.count;
            } else {
                ++i;
            }
        }
    }

    std::sort(moveList.moves, moveList.moves + moveList.count, [](const Move *a, const Move *b)
              { return b->value < a->value; });

    bestMove = ChessStringManipulation::PVToString(*(moveList.moves[0]), 0, false, board4);
    CalculateAndDisplayScore(moveList.moves[0]->value, exactMate);

    int64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
    int64_t safeNodeCount = searchNodeCount;
    int64_t nps = (elapsed_ms > 0) ? (safeNodeCount * 1000LL / elapsed_ms) : 0;
    if (Option::MultiPV > 1)
    {
        int K = std::min(Option::MultiPV, moveList.count);
        for (int i = 0; i < K; i++)
        {
            CalculateAndDisplayScore(moveList.moves[i]->value, false);
            std::cout << "info depth 1 multipv " << (i + 1)
                      << " score " << Score
                      << " time " << elapsed_ms
                      << " nodes " << safeNodeCount
                      << " tbhits " << tablebaseHits.load(std::memory_order_relaxed)
                      << " nps " << nps
                      << " pv " << ChessStringManipulation::PVToString(*(moveList.moves[i]), 0, false, board4) << '\n';
        }
    }
    else
    {
        std::string infoStr = "info depth 1 time " + std::to_string(elapsed_ms) + " nodes " + std::to_string(searchNodeCount) + " tbhits " + std::to_string(tablebaseHits.load(std::memory_order_relaxed)) + " nps " + std::to_string(nps) + " pv " + ChessStringManipulation::PVToString(*(moveList.moves[0]), 1, mated, board4) + " score " + Score;
        DiagnosticLogger::Log("EMIT_INFO", infoStr, DiagnosticLogger::currentSearchId.load());
        std::cout << infoStr << '\n';
    }
    return true;
}

void Search::SearchForCheckUpdate()
{
    /*
    if (recDepth >= 12)
    {
        Option::checkExtension = tempCheckExtension;
        Option::safetyMarginSet(tempSafetyMargin);
    }
    else
    {
        double nowCheckExtension = (tempCheckExtension / 12.0) * recDepth;
        double nowSafetyMargin = (tempSafetyMargin / 12.0) * recDepth;
        Option::checkExtension = static_cast<int>(std::floor(nowCheckExtension + .999));
        Option::safetyMarginSet(static_cast<int>(std::floor(nowSafetyMargin + .999)));
    }
    */
}

void Search::CalculateAndDisplayScore(int value, bool exactMate)
{
    Score = "";
    mated = false;
    if (exactMate && value > MateScore::Threshold)
    {
        Score = "mate " + std::to_string(MateScore::MovesFromRootScore(value));
        mated = true;
    }
    else if (exactMate && value < -MateScore::Threshold)
    {
        Score = "mate -" + std::to_string(MateScore::MovesFromRootScore(value));
        mated = true;
    }
    else
    {
        Score = "cp " + std::to_string(value);
    }
}

std::string Search::Parse(std::string p, int place)
{
    std::string move = "";
    int length = 0;
    for (int counter = 0; counter < place; counter++)
    {
        move = "";
        char a = 'a';
        while (a != ' ' && length < p.size())
        {
            a = p[length];
            if (a != ' ')
            {
                move += a;
            }
            length++;
        }
    }
    return move;
}

int Search::PrintKBest(std::vector<MovePrintValue *> &movesPrintValue, int KBest, bool finiteSearch)
{
    if (movesPrintValue.empty())
    {
        return -200000;
    }
    std::sort(movesPrintValue.begin(), movesPrintValue.end(), [](const MovePrintValue *a, const MovePrintValue *b)
              { return b->value < a->value; });
    int printNumber = std::min(KBest, static_cast<int>(movesPrintValue.size()));
    int64_t elapsed_ms = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count());
    int64_t safeNodeCount = searchNodeCount;
    int64_t nps = (elapsed_ms > 0) ? (safeNodeCount * 1000LL / elapsed_ms) : 0;
    for (int counter = 0; counter < printNumber; counter++)
    {
        const MovePrintValue *mpv = movesPrintValue[counter];
        std::cout << "info depth " << mpv->depth
                  << " multipv " << (counter + 1)
                  << " score " << mpv->scoreText
                  << " time " << elapsed_ms
                  << " nodes " << safeNodeCount
                  << " tbhits " << tablebaseHits.load(std::memory_order_relaxed)
                  << " nps " << nps
                  << " pv " << mpv->pv << '\n';
    }
    return movesPrintValue[printNumber - 1]->value;
}

void Search::deleteMoveList(std::vector<Move *> *moveList)
{
    for (Move *move : *moveList)
    {
        delete move;
    }
    moveList->clear();
    delete moveList;
}
