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

std::atomic<bool> Search::active{false};
time_t Search::beginTime{0};
std::chrono::high_resolution_clock::time_point Search::startTime;
double Search::allowedTime{0.0};
std::string Search::bestMove{""};
std::string Search::ponderMove{""};
std::string Search::completedBestMove{""};
std::string Search::completedPonderMove{""};
bool Search::finiteSearch{false};

int Search::maxDepth{-1};
int64_t Search::maxNodes{-1};
bool Search::isMoveTime{false};

int Search::overAllIteration = 0;
int Search::moveCount = 0;
std::string Search::Score = "";
bool Search::mated = false;

void Search::PrintBestMove()
{
    const std::string& outBest = !completedBestMove.empty() ? completedBestMove : bestMove;
    const std::string& outPonder = !completedBestMove.empty() ? completedPonderMove : ponderMove;

    if (outBest.empty())
    {
        std::cout << "bestmove (none)\n" << std::flush;
        return;
    }
    if (!outPonder.empty())
    {
        std::cout << "bestmove " << outBest << " ponder " << outPonder << '\n' << std::flush;
    }
    else
    {
        std::cout << "bestmove " << outBest << '\n' << std::flush;
    }
}

void Search::MainSearch(Move &move1, Move &move2, Move &move3, Move &move4, Board &board4)
{
    bestMove = "";
    ponderMove = "";
    completedBestMove = "";
    completedPonderMove = "";

    if (!active)
    {
        PrintBestMove();
        return;
    }
    if (RepetitionHistory::Size() == 0)
    {
        RepetitionHistory::ResetWithRoot(board4.ZobristHashCode);
    }
    int MultiPV = finiteSearch ? 1 : Option::MultiPV;
    MoveList moveList = MoveLogic::MoveGenerator(board4, -1, -1);
    if (moveList.count == 0)
    {
        active = false;
        PrintBestMove();
        return;
    }

    bool MATESearch;
    int turn = board4.sideToMove ? 1 : 0;
    bool firstAssign = false;
    int recDepth = 1;
    int alpha = -200000;
    int beta = 200000;
    moveCount = 0;

    bool previousMoveWasCheck = false;
    if (BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), !board4.sideToMove))
    {
        previousMoveWasCheck = true;
    }
    
    SearchDepthZero(moveList, firstAssign, recDepth, alpha, beta, previousMoveWasCheck, move1, move2, move3, move4, board4);
    completedBestMove = bestMove;
    completedPonderMove = ponderMove;

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

    MovePrintValue *MPValue = new MovePrintValue();
    MPValue->printString = "";
    bool stopRequested = false;

    while (active)
    {
        if (maxDepth > 0 && recDepth > maxDepth)
        {
            break;
        }
        SearchForCheckUpdate();

        alpha = -200000;
        beta = +200000;
        value = -200000;
        std::vector<MovePrintValue *> movesPrintValue;
        int KthBestValue = -200000;
        bool pvPrintedThisDepth = false;
        std::string bestPVString = "";

        for (int counter = 0; counter < moveList.count; counter++)
        {
            if (!active)
            {
                stopRequested = true;
                break;
            }
            if (maxNodes > 0 && moveCount >= maxNodes)
            {
                stopRequested = true;
                break;
            }

            Move *move = moveList.moves[counter];
            if (recDepth == 2 && move->beginPlace == 17 && move->endPlace == 53)
            {
                overAllIteration++;
            }
            if (counter < MultiPV)
            {
                Board *boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
                MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
                GameLogic::DoMove(board4, *move, move4, -1, -1);
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                {
                    move->value = 0;
                }
                else
                {
                    if (move->value > 159800 || move->value < -159800)
                    {
                        MATESearch = true;
                    }
                    else
                    {
                        MATESearch = false;
                    }
                    delete MPValue;
                    MPValue = PVSSearch::PVS(true, -200000, 200000, recDepth - 1, *move, move2, move3, move4, board4, MATESearch, true, 1, false, false);
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
                CalculateAndDisplayScore(move->value);
                MovePrintValue *movePrint = new MovePrintValue();
                movePrint->value = move->value;
                int64_t elapsed_ms = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count());
                int64_t safeMoveCount = static_cast<int64_t>(moveCount);
                int64_t nps = (elapsed_ms > 0) ? (safeMoveCount * 1000LL / elapsed_ms) : 0;
                if (nps < 0) {
                    std::cerr << "[DEBUG] Negative nps detected! moveCount=" << safeMoveCount << ", elapsed_ms=" << elapsed_ms << std::endl;
                }
                movePrint->printString = "info depth " + std::to_string(recDepth) + " time " +
                    std::to_string(elapsed_ms) +
                    " nodes " + std::to_string(moveCount) + " nps " +
                    std::to_string(nps) +
                    " pv " + ChessStringManipulation::PVToString(*move, 1, mated, board4) + ' ' + MPValue->printString + " score " + Score;
                if (ChessStringManipulation::PVToString(*move, 0, false, board4) == bestMove)
                {
                    bestPVString = movePrint->printString;
                }
                movesPrintValue.push_back(movePrint);
                if (!finiteSearch && Option::MultiPV > 1)
                {
                    KthBestValue = PrintKBest(movesPrintValue, MultiPV, finiteSearch);
                }
                else if (moveList.count == 1)
                {
                    std::cout << movePrint->printString << '\n';
                    pvPrintedThisDepth = true;
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
                MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
                GameLogic::DoMove(board4, *move, move4, -1, -1);
                if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
                {
                    value = 0;
                    move->value = 0;
                }
                else
                {
                    bool tempPVNode = false;
                    if (move->value > 159800 || move->value < -159800)
                    {
                        MATESearch = true;
                    }
                    else
                    {
                        MATESearch = false;
                    }
                    delete MPValue;
                    MPValue = PVSSearch::PVS(tempPVNode, -KthBestValue - Option::nullWindowSize, -KthBestValue, recDepth - 1, *move, move2, move3, move4, board4, MATESearch, true, 1, false, true);
                    value = -MPValue->value;
                    move->value = value;
                    if ((value > KthBestValue /* && value < beta*/) || move->value > 159800 || move->value < -159800 || (!finiteSearch && Option::MultiPV > 1))
                    {
                        delete MPValue;
                        MPValue = PVSSearch::PVS(true, -beta, -alpha, recDepth - 1, *move, move2, move3, move4, board4, MATESearch, true, 1, false, false);
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
                if (move->value > 159800 && move->value != 160000)
                {
                    int plies = 159999 - move->value;
                    int mateMoves = (plies + 2) / 2;
                    Score = "mate " + std::to_string(mateMoves);
                    mated = true;
                }
                else if (move->value < -159800 && move->value != -160000)
                {
                    int plies = move->value - (-159999);
                    int mateMoves = (plies + 2) / 2;
                    Score = "mate -" + std::to_string(mateMoves);
                    mated = true;
                }
                else
                {
                    Score = "cp " + std::to_string(move->value);
                }
                MovePrintValue *movePrint = new MovePrintValue();
                movePrint->value = move->value;
                int64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
                int64_t nps = (elapsed_ms > 0) ? (static_cast<int64_t>(moveCount) * 1000LL / elapsed_ms) : 0;
                movePrint->printString = "info depth " + std::to_string(recDepth) + " time " +
                    std::to_string(elapsed_ms) +
                    " nodes " + std::to_string(moveCount) + " nps " +
                    std::to_string(nps) +
                    " pv " + ChessStringManipulation::PVToString(*move, 1, mated, board4) + ' ' + MPValue->printString + " score " + Score;
                movesPrintValue.push_back(movePrint);
                if (value > alpha)
                {
                    alpha = value;
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
                if (!finiteSearch && Option::MultiPV > 1)
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
                        if (!bestPVString.empty())
                        {
                            std::cout << bestPVString << '\n';
                            pvPrintedThisDepth = true;
                        }
                    }
                }
            }

            if (!active)
            {
                stopRequested = true;
                break;
            }
            if (maxNodes > 0 && moveCount >= maxNodes)
            {
                stopRequested = true;
                break;
            }
            if (finiteSearch && allowedTime > 0)
            {
                int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
                double limit = isMoveTime ? allowedTime : ((counter == moveList.count - 1) ? (allowedTime * 0.75) : (allowedTime * 0.90));
                if (elapsed >= limit)
                {
                    stopRequested = true;
                    break;
                }
            }
        }
        if (!stopRequested && !pvPrintedThisDepth)
        {
            if (!finiteSearch && Option::MultiPV > 1)
            {
                if (!movesPrintValue.empty())
                {
                    PrintKBest(movesPrintValue, MultiPV, finiteSearch);
                }
            }
            else if (!bestPVString.empty())
            {
                std::cout << bestPVString << '\n';
            }
            else if (!movesPrintValue.empty())
            {
                std::cout << movesPrintValue[0]->printString << '\n';
            }
        }
        if (stopRequested)
        {
            deleteMovesPrintValue(movesPrintValue);
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
        deleteMovesPrintValue(movesPrintValue);

        if (maxDepth > 0 && recDepth >= maxDepth)
        {
            break;
        }

        recDepth++;
        
        if (recDepth == 114)
        {
#ifdef _WIN32
            // Clean up ALL known leak sources before exit
            std::cout << "\n=== CLEANING UP BEFORE EXIT ===\n";

            // Clean up MPValue
            if (MPValue)
            {
                delete MPValue;
                MPValue = nullptr;
                std::cout << "Deleted MPValue\n";
            }

            // Clean up moveList
            PVSSearch::deleteMoveList(moveList);
            std::cout << "Deleted moveList\n";

            // Clean up movesPrintValue (already cleaned at end of loop, but ensure it's empty)
            if (!movesPrintValue.empty())
            {
                deleteMovesPrintValue(movesPrintValue);
                std::cout << "Deleted movesPrintValue\n";
            }

            // Clean up global initialization resources
            std::cout << "Cleaning up global initialization resources...\n";

            // Clean up Option static resources
            // Option::Cleanup();
            std::cout << "Cleaned up Option\n";

            // Clean up AttackPlaces static resources
            AttackPlaces::Cleanup();
            std::cout << "Cleaned up AttackPlaces\n";

            // Clean up BoardInitializer static resources
            BoardInitializer::Cleanup();
            std::cout << "Cleaned up BoardInitializer\n";

            // Clean up PieceMoves static resources
            PieceMoves::Cleanup();
            std::cout << "Cleaned up PieceMoves\n";

            // Clean up MoveLogic static resources
            MoveLogic::Cleanup();
            std::cout << "Cleaned up MoveLogic\n";

            // Clean up KingSetup static resources
            KingSetup::Cleanup();
            std::cout << "Cleaned up KingSetup\n";

            // Clean up PassedPawnSetup static resources
            PassedPawnSetup::Cleanup();
            std::cout << "Cleaned up PassedPawnSetup\n";

            std::cout << "All local and global resources cleaned up\n";

            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
            std::cout << "\n=== MEMORY LEAK DETECTION ===\n";
            _CrtDumpMemoryLeaks();
            std::cout << "=== END LEAK DETECTION ===\n";
#endif
            exit(0);
        }
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

void Search::SearchDepthZero(MoveList &moveList, bool &firstAssign, int &recDepth, int &alpha, int &beta, bool &previousMoveWasCheck, Move &move1, Move &move2, Move &move3, Move &move4, Board &board4)
{
    std::vector<Move> movesToDelete;
    int turn = board4.sideToMove ? 1 : 0;
    for (int i = 0; i < moveList.count; ++i)
    {
        if (!active)
        {
            break;
        }
        Move *move = moveList.moves[i];
        auto boardCopy = UCI::IsRelease ? nullptr : board4.MakeCopy();
        MissingInfoAboutPrevStateFromMove *missingInfoAboutPrevStateFromMove = new MissingInfoAboutPrevStateFromMove(board4);
        GameLogic::DoMove(board4, *move, move4, -2, -2);
        if (!BoardLogic::UnderAttack(board4, board4.pieces[turn * 8 + 6].front(), board4.sideToMove))
        {
            if (!firstAssign)
            {
                bestMove = ChessStringManipulation::PVToString(*move, 0, false, board4);
                firstAssign = true;
            }

            if (RepetitionHistory::IsRepetition(board4.ZobristHashCode))
            {
                move->value = 0;
            }
            else
            {
                MovePrintValue *tempRetValLocal = PVSSearch::PVS(true, -beta, -alpha, 0, *move, move2, move3, move4, board4, false, true, 1, previousMoveWasCheck, false);
                move->value = - tempRetValLocal->value;
                delete tempRetValLocal;
                tempRetValLocal = nullptr;
            }

            if (move->value > alpha)
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
    CalculateAndDisplayScore(moveList.moves[0]->value);

    int64_t elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
    int64_t safeMoveCount = static_cast<int64_t>(moveCount);
    int64_t nps = (elapsed_ms > 0) ? (safeMoveCount * 1000LL / elapsed_ms) : 0;
    std::cout << "info depth 1 time "
        << elapsed_ms
        << " nodes " << moveCount
        << " nps " << nps
        << " pv " << ChessStringManipulation::PVToString(*(moveList.moves[0]), 1, mated, board4)
        << " score " << Score << '\n';
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

void Search::CalculateAndDisplayScore(int value)
{
    Score = "";
    mated = false;
    if (value > 159800 && value != 160000)
    {
        int plies = 159999 - value;
        int mateMoves = (plies + 2) / 2;
        Score = "mate " + std::to_string(mateMoves);
        mated = true;
    }
    else if (value < -159800 && value != -160000)
    {
        int plies = value - (-159999);
        int mateMoves = (plies + 2) / 2;
        Score = "mate -" + std::to_string(mateMoves);
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
    // multipv 4;
    std::sort(movesPrintValue.begin(), movesPrintValue.end(), [](const MovePrintValue *a, const MovePrintValue *b)
              { return b->value < a->value; });
    int printNumber;
    if (!finiteSearch)
    {
        if (movesPrintValue.size() > KBest)
        {
            printNumber = KBest;
        }
        else
        {
            printNumber = movesPrintValue.size();
        }
        for (int counter = 0; counter < printNumber; counter++)
        {
            std::cout << (*(movesPrintValue[counter])).printString << " multipv " << (counter + 1) << '\n';
        }
    }
    else
    {
        printNumber = 1;
        std::cout << (*(movesPrintValue[0])).printString << '\n';
    }
    return (*(movesPrintValue[printNumber - 1])).value;
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