#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "UCI.h"
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include "BoardInitializer.h"
#include "LastFourMoves.h"
#include "ChessStringManipulation.h"
#include "MoveLogic.h"
#include "GameLogic.h"
#include "BoardMaker.h"
#include "HashMemoryBudget.h"
#include "RepetitionHistory.h"

#include "DiagnosticLogger.h"

// Correctness tests retain the make/undo board-copy assertions. Normal engine
// execution starts in production mode and avoids those diagnostic allocations.
#if HOWL_CORRECTNESS_TESTING
bool UCI::IsRelease = false;
#else
bool UCI::IsRelease = true;
#endif
bool UCI::TestOrderAvailable = false;
bool UCI::StartPosForTest = false;

std::thread UCI::searchThread;
Move *UCI::move1 = nullptr;
Move *UCI::move2 = nullptr;
Move *UCI::move3 = nullptr;
Move *UCI::move4 = nullptr;
Board *UCI::thisBoard = nullptr;
std::string UCI::order = ""; // std::string(7, 'c');

#if HOWL_CORRECTNESS_TESTING
std::size_t UCI::releasedBoards = 0;
std::size_t UCI::releasedHistoryMoves = 0;
#endif

void UCI::ReplaceCurrentBoard(Board *replacementBoard)
{
    if (thisBoard != nullptr)
    {
        delete thisBoard;
#if HOWL_CORRECTNESS_TESTING
        releasedBoards++;
#endif
    }
    thisBoard = replacementBoard;
}

void UCI::ReleaseMoveHistory()
{
    Move **currentHistory[] = {&move1, &move2, &move3, &move4};
    for (Move **historyMove : currentHistory)
    {
        if (*historyMove != nullptr)
        {
            delete *historyMove;
#if HOWL_CORRECTNESS_TESTING
            releasedHistoryMoves++;
#endif
        }
        *historyMove = nullptr;
    }
}

void UCI::ReplaceMoveHistory(LastFourMoves *replacementHistory)
{
    ReleaseMoveHistory();
    if (replacementHistory != nullptr)
    {
        move1 = replacementHistory->Move1;
        move2 = replacementHistory->Move2;
        move3 = replacementHistory->Move3;
        move4 = replacementHistory->Move4;
        replacementHistory->Move1 = nullptr;
        replacementHistory->Move2 = nullptr;
        replacementHistory->Move3 = nullptr;
        replacementHistory->Move4 = nullptr;
        delete replacementHistory;
    }
}

void UCI::ReleaseCurrentPosition()
{
    ReplaceCurrentBoard(nullptr);
    ReleaseMoveHistory();
}

#if HOWL_CORRECTNESS_TESTING
void UCI::ResetPositionOwnershipStatistics()
{
    releasedBoards = 0;
    releasedHistoryMoves = 0;
}

std::size_t UCI::ReleasedBoardCount()
{
    return releasedBoards;
}

std::size_t UCI::ReleasedHistoryMoveCount()
{
    return releasedHistoryMoves;
}
#endif

void UCI::MainSearchStart()
{
    uint64_t prevId = DiagnosticLogger::currentSearchId.load();
    if (searchThread.joinable())
    {
        DiagnosticLogger::Log("JOIN_BEGIN", "MainSearchStart joining previous searchThread", prevId);
        Search::stopRequested = true;
        Search::active = false;
        searchThread.join();
        DiagnosticLogger::Log("JOIN_END", "MainSearchStart joined previous searchThread", prevId);
    }
    HashMemoryBudget::EnsureDefaultConfigured(std::cout);
    HashMemoryBudget::MarkSearchStarted();
    Search::startTime = std::chrono::high_resolution_clock::now();
    Search::stopRequested = false;
    Search::active = true;
    uint64_t searchId = ++DiagnosticLogger::currentSearchId;
    std::string fenBeforeGo = DiagnosticLogger::BoardToFen(thisBoard);
    DiagnosticLogger::Log("SEARCH_START", "Spawning search thread with FEN: " + fenBeforeGo, searchId);
    searchThread = std::thread(Search::MainSearch, std::ref(*move1), std::ref(*move2), std::ref(*move3), std::ref(*move4), std::ref(*thisBoard));
}

bool UCI::ApplyHashOptionCommand(const std::string& command,
                                 std::ostream& diagnostics)
{
    const std::string prefix = "setoption name Hash";
    const std::string valuePrefix = prefix + " value ";
    if (command.compare(0, valuePrefix.size(), valuePrefix) == 0)
    {
        return HashMemoryBudget::ConfigureValue(
            command.substr(valuePrefix.size()), diagnostics);
    }
    if (command == prefix || command == prefix + " value")
    {
        return HashMemoryBudget::ConfigureValue("", diagnostics);
    }
    diagnostics << "info string malformed Hash option command\n";
    return false;
}

bool UCI::IsTest()
{
    return !IsRelease;
}

void UCI::MainAsync()
{
    Run(std::cin, std::cout);
}

void UCI::Run(std::istream& in, std::ostream& out)
{
    std::streambuf* oldCoutBuf = nullptr;
    if (&out != &std::cout)
    {
        oldCoutBuf = std::cout.rdbuf(out.rdbuf());
    }

    int savedOrdersCount = 3;
    int savedOrderToProcessNo = 0;
    bool threadContinue = true;
    TestOrderAvailable = !IsRelease;
    while (threadContinue)
    {
        if (IsRelease || !TestOrderAvailable)
        {
            if (!std::getline(in, order))
            {
                if (searchThread.joinable())
                {
                    DiagnosticLogger::Log("JOIN_BEGIN", "EOF on stdin, joining searchThread", DiagnosticLogger::currentSearchId.load());
                    Search::stopRequested = true;
                    Search::active = false;
                    searchThread.join();
                    DiagnosticLogger::Log("JOIN_END", "EOF on stdin, joined searchThread", DiagnosticLogger::currentSearchId.load());
                }
                break;
            }
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

        if (order.empty())
        {
            continue;
        }

        DiagnosticLogger::Log("UCI_CMD", order, DiagnosticLogger::currentSearchId.load());

        if (order == "uci")
        {
            out << "id name Howl 1\n";
            out << "id author Masoud Modabber\n";
            out << "option name MultiPV type spin default 1 min 1 max 99\n";
            out << "option name Hash type spin min 8 max 1024 default 40\n";
            out << "uciok\n" << std::flush;
        }
        else if (order == "isready")
        {
            if (searchThread.joinable())
            {
                DiagnosticLogger::Log("JOIN_BEGIN", "isready joining searchThread", DiagnosticLogger::currentSearchId.load());
                searchThread.join();
                DiagnosticLogger::Log("JOIN_END", "isready joined searchThread", DiagnosticLogger::currentSearchId.load());
            }
            HashMemoryBudget::EnsureDefaultConfigured(out);
            out << "readyok\n" << std::flush;
        }
        else if (order == "ucinewgame")
        {
            if (searchThread.joinable())
            {
                DiagnosticLogger::Log("STOP_REQUEST", "ucinewgame stopping search", DiagnosticLogger::currentSearchId.load());
                Search::stopRequested = true;
                Search::active = false;
                DiagnosticLogger::Log("JOIN_BEGIN", "ucinewgame joining searchThread", DiagnosticLogger::currentSearchId.load());
                searchThread.join();
                DiagnosticLogger::Log("JOIN_END", "ucinewgame joined searchThread", DiagnosticLogger::currentSearchId.load());
            }
        }
        else if (order == "quit")
        {
            if (searchThread.joinable())
            {
                DiagnosticLogger::Log("STOP_REQUEST", "quit stopping search", DiagnosticLogger::currentSearchId.load());
                Search::stopRequested = true;
                Search::active = false;
                DiagnosticLogger::Log("JOIN_BEGIN", "quit joining searchThread", DiagnosticLogger::currentSearchId.load());
                searchThread.join();
                DiagnosticLogger::Log("JOIN_END", "quit joined searchThread", DiagnosticLogger::currentSearchId.load());
            }
            threadContinue = false;
        }
        else if (order == "stop")
        {
            if (searchThread.joinable())
            {
                DiagnosticLogger::Log("STOP_REQUEST", "stop command received", DiagnosticLogger::currentSearchId.load());
                Search::stopRequested = true;
                Search::active = false;
                DiagnosticLogger::Log("JOIN_BEGIN", "stop joining searchThread", DiagnosticLogger::currentSearchId.load());
                searchThread.join();
                DiagnosticLogger::Log("JOIN_END", "stop joined searchThread", DiagnosticLogger::currentSearchId.load());
            }
        }
        if (!threadContinue)
            continue;

        if (order.length() >= 8 && order.substr(0, 8) == "position")
        {
            if (searchThread.joinable())
            {
                DiagnosticLogger::Log("STOP_REQUEST", "position stopping search", DiagnosticLogger::currentSearchId.load());
                Search::stopRequested = true;
                Search::active = false;
                DiagnosticLogger::Log("JOIN_BEGIN", "position joining searchThread", DiagnosticLogger::currentSearchId.load());
                searchThread.join();
                DiagnosticLogger::Log("JOIN_END", "position joined searchThread", DiagnosticLogger::currentSearchId.load());
            }
            DiagnosticLogger::Log("POSITION_BEGIN", order, DiagnosticLogger::currentSearchId.load());
            if (order.length() > 8 && order.substr(0, 9) == "position ")
            {
                if (order.substr(9, 8) == "startpos")
                {
                    ReplaceCurrentBoard(BoardInitializer::beginBoard->MakeCopy());
                    thisBoard->moveNumber = 1;
                    RepetitionHistory::ResetWithRoot(thisBoard->ZobristHashCode);
                    if (order.length() > 17)
                    {
                        LastFourMoves *lTemp = MakeMoves(order.substr(18), *thisBoard);
                        ReplaceMoveHistory(lTemp);
                    }
                    else
                    {
                        Move *move = new Move();
                        LastFourMoves *replacementHistory = new LastFourMoves();
                        replacementHistory->Move1 = MoveLogic::MoveCopy(move);
                        replacementHistory->Move2 = MoveLogic::MoveCopy(move);
                        replacementHistory->Move3 = MoveLogic::MoveCopy(move);
                        replacementHistory->Move4 = MoveLogic::MoveCopy(move);
                        delete move;
                        ReplaceMoveHistory(replacementHistory);
                    }
                }
                else
                {
                    Board *replacementBoard;
                    std::string fenPart = order.substr(9);
                    if (order.substr(9, 3) == "fen")
                    {
                        fenPart = order.substr(13);
                    }
                    size_t movesPos = fenPart.find(" moves ");
                    if (movesPos != std::string::npos)
                    {
                        std::string fenOnly = fenPart.substr(0, movesPos);
                        std::string movesOnly = fenPart.substr(movesPos + 1);
                        replacementBoard = BoardMaker::MakeInitialBoard(fenOnly);
                        ReplaceCurrentBoard(replacementBoard);
                        RepetitionHistory::ResetWithRoot(thisBoard->ZobristHashCode);
                        LastFourMoves *lTemp = MakeMoves(movesOnly, *thisBoard);
                        ReplaceMoveHistory(lTemp);
                    }
                    else
                    {
                        replacementBoard = BoardMaker::MakeInitialBoard(fenPart);
                        ReplaceCurrentBoard(replacementBoard);
                        RepetitionHistory::ResetWithRoot(thisBoard->ZobristHashCode);
                    }
                }
            }
            DiagnosticLogger::Log("POSITION_END", "Final FEN: " + DiagnosticLogger::BoardToFen(thisBoard), DiagnosticLogger::currentSearchId.load());
        }
        if (order == "ponderhit")
        {
            Search::startTime = std::chrono::high_resolution_clock::now();
            Search::beginTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            Search::finiteSearch = true;
        }
        if (order.length() > 9 && order.substr(0, 10) == "setoption ")
        {
            if (order.compare(0, 19, "setoption name Hash") == 0)
            {
                ApplyHashOptionCommand(order, out);
                continue;
            }
            std::string tempString = order.substr(15);
            int counter = 0;
            std::string option = "";
            while (counter < tempString.size() && tempString[counter] != ' ')
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
        }
        if (order == "go" || (order.length() > 2 && order.substr(0, 3) == "go "))
        {
            int wTimeMill = -1;
            int bTimeMill = -1;
            int wIncMill = 0;
            int bIncMill = 0;
            int remainedMoves = 0;
            int fixedDepth = -1;
            int64_t fixedNodes = -1;
            int fixedTime = -1;
            bool infiniteSearch = false;
            bool mateSearch = false;
            bool ponder = false;

            std::istringstream iss(order);
            std::string goCmd;
            iss >> goCmd;
            std::string token;
            while (iss >> token)
            {
                if (token == "searchmoves" || token == "searchMove")
                {
                    // TODO: searchmoves filtering
                }
                else if (token == "ponder")
                {
                    ponder = true;
                    infiniteSearch = true;
                }
                else if (token == "wtime")
                {
                    iss >> wTimeMill;
                }
                else if (token == "btime")
                {
                    iss >> bTimeMill;
                }
                else if (token == "winc")
                {
                    iss >> wIncMill;
                }
                else if (token == "binc")
                {
                    iss >> bIncMill;
                }
                else if (token == "movestogo")
                {
                    iss >> remainedMoves;
                }
                else if (token == "depth")
                {
                    iss >> fixedDepth;
                }
                else if (token == "nodes")
                {
                    iss >> fixedNodes;
                }
                else if (token == "movetime")
                {
                    iss >> fixedTime;
                }
                else if (token == "mate")
                {
                    mateSearch = true;
                    int mateDepth = 0;
                    iss >> mateDepth;
                }
                else if (token == "infinite")
                {
                    infiniteSearch = true;
                }
            }

            if (thisBoard == nullptr)
            {
                ReplaceCurrentBoard(BoardInitializer::beginBoard->MakeCopy());
                thisBoard->moveNumber = 1;
                RepetitionHistory::ResetWithRoot(thisBoard->ZobristHashCode);
            }
            if (move1 == nullptr)
            {
                Move *move = new Move();
                LastFourMoves *replacementHistory = new LastFourMoves();
                replacementHistory->Move1 = MoveLogic::MoveCopy(move);
                replacementHistory->Move2 = MoveLogic::MoveCopy(move);
                replacementHistory->Move3 = MoveLogic::MoveCopy(move);
                replacementHistory->Move4 = MoveLogic::MoveCopy(move);
                delete move;
                ReplaceMoveHistory(replacementHistory);
            }

            Search::maxDepth = fixedDepth;
            Search::maxNodes = fixedNodes;
            Search::isMoveTime = false;
            Search::allowedTime = 0.0;
            Search::finiteSearch = false;

            if (mateSearch)
            {
                // TODO
            }
            else if (fixedTime > 0)
            {
                Search::allowedTime = static_cast<double>(fixedTime);
                Search::isMoveTime = true;
                Search::finiteSearch = true;
                MainSearchStart();
            }
            else if (fixedDepth > 0 && fixedNodes <= 0 && wTimeMill < 0 && bTimeMill < 0)
            {
                Search::finiteSearch = true;
                MainSearchStart();
            }
            else if (fixedNodes > 0 && fixedDepth <= 0 && wTimeMill < 0 && bTimeMill < 0)
            {
                Search::finiteSearch = true;
                MainSearchStart();
            }
            else if (infiniteSearch || (wTimeMill < 0 && bTimeMill < 0 && fixedDepth <= 0 && fixedNodes <= 0))
            {
                out << "info depth 1 time 0 nodes 0 nps 0 pv\n" << std::flush;
                MainSearchStart();
            }
            else
            {
                int timeMill = (!thisBoard->sideToMove) ? (wTimeMill >= 0 ? wTimeMill : 0) : (bTimeMill >= 0 ? bTimeMill : 0);
                int incMill = (!thisBoard->sideToMove) ? wIncMill : bIncMill;
                auto allowedTimeDuration = getAllowedTime(timeMill, incMill, remainedMoves);
                Search::allowedTime = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(allowedTimeDuration).count());
                Search::finiteSearch = true;
                MainSearchStart();
            }
        }
    }

    if (searchThread.joinable())
    {
        Search::stopRequested = true;
        Search::active = false;
        searchThread.join();
    }
    if (oldCoutBuf != nullptr)
    {
        std::cout.rdbuf(oldCoutBuf);
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
    long long allocatedMs;
    if (remainedMoves != 0)
    {
        if (remainedMoves != 1)
        {
            allocatedMs = static_cast<long long>(TimeMill * (1 + 0.01 * remainedMoves) / remainedMoves + IncMill);
        }
        else
        {
            allocatedMs = static_cast<long long>(TimeMill * 0.7);
        }
    }
    else
    {
        allocatedMs = static_cast<long long>(TimeMill * 5.0 / 165 + IncMill);
    }
    if (allocatedMs < 1)
    {
        allocatedMs = 1;
    }
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(allocatedMs));
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
            delete lastMoves->Move1;
            lastMoves->Move1 = lastMoves->Move2;
            lastMoves->Move2 = lastMoves->Move3;
            lastMoves->Move3 = lastMoves->Move4;
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
    delete lastMoves->Move1;
    lastMoves->Move1 = lastMoves->Move2;
    lastMoves->Move2 = lastMoves->Move3;
    lastMoves->Move3 = lastMoves->Move4;
    lastMoves->Move4 = MoveLogic::MoveCopy(doneMove2);
    delete doneMove2;
    doneMove2 = nullptr;
    return lastMoves;
}
