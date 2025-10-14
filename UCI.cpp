#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "UCI.h"
#include <iostream>
#include <string>
#include "BoardInitializer.h"
#include "LastFourMoves.h"
#include "ChessStringManipulation.h"
#include "MoveLogic.h"
#include "GameLogic.h"
#include "BoardMaker.h"

// Initialize static members
bool UCI::IsRelease = false;
bool UCI::TestOrderAvailable = false;
bool UCI::StartPosForTest = false;

Move *UCI::move1 = nullptr;
Move *UCI::move2 = nullptr;
Move *UCI::move3 = nullptr;
Move *UCI::move4 = nullptr;
Board *UCI::thisBoard = nullptr;
std::string UCI::order = ""; // std::string(7, 'c');

void UCI::MainSearchStart()
{
    std::thread thread(Search::MainSearch, std::ref(*move1), std::ref(*move2), std::ref(*move3), std::ref(*move4), std::ref(*thisBoard));
    thread.detach();
}

bool UCI::IsTest()
{
    return !IsRelease;
}

void UCI::MainAsync()
{
    int savedOrdersCount = 3;
    int savedOrderToProcessNo = 0;
    bool threadContinue = true;
    TestOrderAvailable = !IsRelease;
    while (threadContinue)
    {
        if (IsRelease || !TestOrderAvailable)
        {
            std::getline(std::cin, order);
        }
        else
        {

            SetAutomaticOrders(savedOrderToProcessNo);
            savedOrderToProcessNo++;
            if (savedOrderToProcessNo == savedOrdersCount)
            {
                TestOrderAvailable = false;
            }
        }
        if (order == "uci")
        {
            std::cout << "id name Howl5\n";
            std::cout << "id author Masoud Modabber\n";
            std::cout << "option name MultiPV type spin default 1 min 1 max 99\n";
            std::cout << "option name Hash type spin min 4 max 1024 default 40\n";
            std::cout << "uciok\n";
        }
        else if (order == "isready")
        {
            std::cout << "readyok\n";
        }
        else if (order == "ucinewgame")
        {
            // Do nothing
        }
        else if (order == "quit")
        {
            threadContinue = false;
        }
        else if (order == "stop")
        {
            if (Search::active)
            {
                if (!Search::ponderMove.empty())
                {
                    std::cout << "bestmove " << Search::bestMove << " ponder " << Search::ponderMove << '\n';
                }
                else
                {
                    std::cout << "bestmove " << Search::bestMove << '\n';
                }
            }
            Search::active = false;
            threadContinue = false;
        }
        if (!threadContinue)
            continue;
        if (order.length() > 8 && order.substr(0, 9) == "position ")
        {
            if (order.substr(9, 8) == "startpos")
            {
                delete thisBoard;
                thisBoard = BoardInitializer::beginBoard->MakeCopy();
                thisBoard->moveNumber = 1;
                if (order.length() > 17)
                {
                    LastFourMoves *lTemp = MakeMoves(order.substr(18), *thisBoard);
                    delete move1;
                    delete move2;
                    delete move3;
                    delete move4;
                    move1 = lTemp->Move1;
                    move2 = lTemp->Move2;
                    move3 = lTemp->Move3;
                    move4 = lTemp->Move4;
                    delete lTemp;
                    lTemp = nullptr;
                }
                else
                {
                    Move *move = new Move();
                    delete move1;
                    delete move2;
                    delete move3;
                    delete move4;
                    move1 = MoveLogic::MoveCopy(move);
                    move2 = MoveLogic::MoveCopy(move);
                    move3 = MoveLogic::MoveCopy(move);
                    move4 = MoveLogic::MoveCopy(move);
                    delete move;
                }
            }
            else
            {
                if (order.substr(9, 3) == "fen")
                {
                    thisBoard = BoardMaker::MakeInitialBoard(order.substr(13));
                }
                else
                {
                    thisBoard = BoardMaker::MakeInitialBoard(order.substr(9));
                }
            }
        }
        bool infiniteSearch;
        if (order == "ponderhit")
        {
            Search::beginTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            Search::finiteSearch = true;
        }
        if (order.length() > 9 && order.substr(0, 10) == "setoption ")
        {
            std::string tempString = order.substr(15);
            int counter = 0;
            std::string option = "";
            while (tempString[counter] != ' ')
            {
                option += tempString[counter];
                counter++;
            }
            if (option == "nullWindowSize")
            {
                Option::nullWindowSize = std::stoi(tempString.substr(counter + 7));
            }
            else if (option == "checkExtension")
            {
                Option::checkExtension = std::stoi(tempString.substr(counter + 7));
            }
            else if (option == "MultiPV")
            {
                Option::MultiPV = std::stoi(tempString.substr(counter + 7));
            }
            else if (option == "Hash")
            {
                Option::hashSize = std::stoi(tempString.substr(counter + 7));
                Option::EvalDictionaryShare = static_cast<int>(Option::EvalDictionarySharePercent * (Option::hashSize - 17) * 1024 * 1024 / Option::EvalDictionaryitemSize);
                Option::PawnDictionaryShare = static_cast<int>(Option::PawnDictionarySharePercent * (Option::hashSize - 17) * 1024 * 1024 / Option::PawnDictionaryitemSize);
            }
        }
        if (order.length() > 2 && order.substr(0, 3) == "go ")
        {
            int wTimeMill = 0;
            int bTimeMill = 0;
            int wIncMill = 0;
            int bIncMill = 0;
            int remainedMoves = 0;
            int fixedDepth = 0;
            int fixedNodes = 0;
            bool mateSearch = false;
            int fixedTime = 0;
            infiniteSearch = false;
            Search::finiteSearch = false;
            std::string goOrder = order.substr(3);
            std::string orderVar = "";
            int counter = 0;
            while (counter < goOrder.length())
            {
                if (counter < goOrder.length() && goOrder[counter] != ' ')
                {
                    orderVar += goOrder[counter];
                    counter++;
                }
                else
                {
                    counter++;
                    if (orderVar == "searchMove")
                    {
                        // TODO
                    }
                    else if (orderVar == "ponder")
                    {
                        infiniteSearch = true;
                    }
                    else if (orderVar == "wtime")
                    {
                        while (counter < goOrder.length() && goOrder[counter] != ' ')
                        {
                            wTimeMill = wTimeMill * 10 + goOrder[counter] - '1' + 1;
                            counter++;
                        }
                    }
                    else if (orderVar == "btime")
                    {
                        while (counter < goOrder.length() && goOrder[counter] != ' ')
                        {
                            bTimeMill = bTimeMill * 10 + goOrder[counter] - '1' + 1;
                            counter++;
                        }
                    }
                    else if (orderVar == "winc")
                    {
                        while (counter < goOrder.length() && goOrder[counter] != ' ')
                        {
                            wIncMill = wIncMill * 10 + goOrder[counter] - '1' + 1;
                            counter++;
                        }
                    }
                    else if (orderVar == "binc")
                    {
                        while (counter < goOrder.length() && goOrder[counter] != ' ')
                        {
                            bIncMill = bIncMill * 10 + goOrder[counter] - '1' + 1;
                            counter++;
                        }
                    }
                    else if (orderVar == "movestogo")
                    {
                        while (counter < goOrder.length() && goOrder[counter] != ' ')
                        {
                            remainedMoves = remainedMoves * 10 + goOrder[counter] - '1' + 1;
                            counter++;
                        }
                    }
                    else if (orderVar == "depth")
                    {
                        while (counter < goOrder.length() && goOrder[counter] != ' ')
                        {
                            fixedDepth = fixedDepth * 10 + goOrder[counter] - '1' + 1;
                            counter++;
                        }
                    }
                    else if (orderVar == "nodes")
                    {
                        while (counter < goOrder.length() && goOrder[counter] != ' ')
                        {
                            fixedNodes = fixedNodes * 10 + goOrder[counter] - '1' + 1;
                            counter++;
                        }
                    }
                    else if (orderVar == "movetime")
                    {
                        while (counter < goOrder.length() && goOrder[counter] != ' ')
                        {
                            fixedTime = fixedTime * 10 + goOrder[counter] - '1' + 1;
                        }
                    }
                    else if (orderVar == "mate")
                    {
                        mateSearch = true;
                    }
                    else if (orderVar == "infinite")
                    {
                        infiniteSearch = true;
                    }
                    counter++;
                    orderVar = "";
                }
            }
            if (orderVar == "mate")
            {
                mateSearch = true;
            }
            else if (orderVar == "infinite")
            {
                infiniteSearch = true;
            }
            if (mateSearch)
            {
                // TODO
            }
            else if (fixedTime > 0)
            {
                // TODO
            }
            else if (fixedDepth > 0)
            {
                // TODO
            }
            else if (fixedNodes > 0)
            {
                // TODO
            }
            else if (infiniteSearch)
            {
                std::cout << "info depth 1 time 0 nodes 0 nps 0 pv\n" << std::flush;
                MainSearchStart();
            }
            else
            {
                if (!thisBoard->sideToMove)
                {
                    auto allowedTimeDuration = getAllowedTime(wTimeMill, wIncMill, remainedMoves);
                    Search::allowedTime = std::chrono::duration_cast<std::chrono::milliseconds>(allowedTimeDuration).count();
                }
                else
                {
                    auto allowedTimeDuration = getAllowedTime(bTimeMill, bIncMill, remainedMoves);
                    Search::allowedTime = std::chrono::duration_cast<std::chrono::milliseconds>(allowedTimeDuration).count();
                }
                Search::finiteSearch = true;
                MainSearchStart();
            }
        }
    }
}

void UCI::SetAutomaticOrders(int savedOrderToProcessNo)
{
    if (!IsRelease)
    {
        if (savedOrderToProcessNo == 0)
        {
            order = "setoption name MultiPV value 1";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else if (savedOrderToProcessNo == 1)
        {
            if (StartPosForTest)
            {
                order = "position startpos";
            }
            else
            {
                order = "position rnbqk2r/ppp1pp1p/7B/3P4/2PP2n1/8/PP3PPP/RN1QKBNR b - - 1 0";
            }
        }
        else if (savedOrderToProcessNo == 2)
        {
            order = "go infinite";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else if (savedOrderToProcessNo == 3)
        {
            order = "ucinewgame";
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        else if (savedOrderToProcessNo == 4)
        {
            order = "position startpos";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else if (savedOrderToProcessNo == 5)
        {
            order = "go infinite";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else if (savedOrderToProcessNo == 6)
        {
            order = "stop";
            std::this_thread::sleep_for(std::chrono::milliseconds(8000));
        }
        else if (savedOrderToProcessNo == 7)
        {
            order = "isready";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else if (savedOrderToProcessNo == 8)
        {
            order = "position startpos moves d2d4 g8f6 c2c4 e7e5 d4e5";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else if (savedOrderToProcessNo == 9)
        {
            order = "go infinite";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

std::chrono::nanoseconds UCI::getAllowedTime(int TimeMill, int IncMill, int remainedMoves)
{
    long long nanoSecs;
    if (remainedMoves != 0)
    {
        if (remainedMoves != 1)
        {
            nanoSecs = static_cast<long long>(TimeMill * (1 + 0.01 * remainedMoves) / remainedMoves + IncMill);
        }
        else
        {
            nanoSecs = static_cast<int>(TimeMill * 0.7);
        }
    }
    else
    {
        nanoSecs = static_cast<int>(TimeMill * 5.0 / 165 + IncMill);
    }
    return std::chrono::nanoseconds(nanoSecs * 10000);
}

LastFourMoves *UCI::MakeMoves(std::string moves, Board &thisBoard)
{
    LastFourMoves *lastMoves = new LastFourMoves();
    Move *move = new Move();
    lastMoves->Move1 = MoveLogic::MoveCopy(move);
    lastMoves->Move2 = MoveLogic::MoveCopy(move);
    lastMoves->Move3 = MoveLogic::MoveCopy(move);
    lastMoves->Move4 = MoveLogic::MoveCopy(move);
    delete move;
    move = nullptr;
    std::string moveString = moves.substr(6);
    int counter = 0;
    int eachMoveCounter = 0;
    std::string tempMove = "";
    while (counter < moveString.length())
    {
        tempMove += moveString[counter];
        eachMoveCounter++;
        if (counter + 2 >= moveString.length())
        {
            tempMove += moveString[counter + 1];
            counter++;
        }
        else if (eachMoveCounter > 2 && moveString[counter + 2] < 'i' && moveString[counter + 2] >= 'a' && moveString[counter + 3] >= '1' && moveString[counter + 3] < '9')
        {
            eachMoveCounter = 0;
            Move *doneMove = ChessStringManipulation::ConvertTextToMove(tempMove, thisBoard);
            GameLogic::DoMove(thisBoard, *doneMove, *doneMove, -4, -4);
            lastMoves->Move1 = MoveLogic::MoveCopy(lastMoves->Move2);
            lastMoves->Move2 = MoveLogic::MoveCopy(lastMoves->Move3);
            lastMoves->Move3 = MoveLogic::MoveCopy(lastMoves->Move4);
            lastMoves->Move4 = MoveLogic::MoveCopy(doneMove);
            tempMove = "";
            counter++;
            delete doneMove;
            doneMove = nullptr;
        }
        counter++;
    }
    Move *doneMove2 = ChessStringManipulation::ConvertTextToMove(tempMove, thisBoard);
    GameLogic::DoMove(thisBoard, *doneMove2, *doneMove2, -4, -4);
    lastMoves->Move1 = MoveLogic::MoveCopy(lastMoves->Move2);
    lastMoves->Move2 = MoveLogic::MoveCopy(lastMoves->Move3);
    lastMoves->Move3 = MoveLogic::MoveCopy(lastMoves->Move4);
    lastMoves->Move4 = MoveLogic::MoveCopy(doneMove2);
    delete doneMove2;
    doneMove2 = nullptr;
    return lastMoves;
}