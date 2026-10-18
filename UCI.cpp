#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "UCI.h"
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <algorithm>
#include "BoardInitializer.h"
#include "LastFourMoves.h"
#include "ChessStringManipulation.h"
#include "MoveLogic.h"
#include "GameLogic.h"
#include "BoardMaker.h"
#include "HashMemoryBudget.h"
#include "RepetitionHistory.h"
#include "Tablebase.h"

std::thread UCI::searchThread;
Move *UCI::move1 = nullptr;
Move *UCI::move2 = nullptr;
Move *UCI::move3 = nullptr;
Move *UCI::move4 = nullptr;
Board *UCI::thisBoard = nullptr;
std::string UCI::order = "";

void UCI::ReplaceCurrentBoard(Board *replacementBoard)
{
    if (thisBoard != nullptr)
    {
        delete thisBoard;
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

void UCI::MainSearchStart()
{
    if (searchThread.joinable())
    {
        Search::stopRequested = true;
        Search::active = false;
        searchThread.join();
    }
    HashMemoryBudget::EnsureDefaultConfigured(std::cout);
    HashMemoryBudget::MarkSearchStarted();
    Search::startTime = std::chrono::high_resolution_clock::now();
    Search::stopRequested = false;
    Search::active = true;
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

    bool threadContinue = true;
    while (threadContinue)
    {
        if (!std::getline(in, order))
        {
            if (searchThread.joinable())
            {
                Search::stopRequested = true;
                Search::active = false;
                searchThread.join();
            }
            break;
        }

        if (order.empty())
        {
            continue;
        }

        if (order == "uci")
        {
            out << "id name Howl 1\n";
            out << "id author Masoud Modabber\n";
            out << "option name MultiPV type spin default 1 min 1 max 99\n";
            out << "option name Hash type spin min 8 max 1024 default 40\n";
            out << "option name SyzygyPath type string default <empty>\n";
            out << "option name SyzygyProbeLimit type spin default 5 min 0 max 7\n";
            out << "uciok\n" << std::flush;
        }
        else if (order == "isready")
        {
            if (searchThread.joinable())
            {
                searchThread.join();
            }
            HashMemoryBudget::EnsureDefaultConfigured(out);
            out << "readyok\n" << std::flush;
        }
        else if (order == "ucinewgame")
        {
            if (searchThread.joinable())
            {
                Search::stopRequested = true;
                Search::active = false;
                searchThread.join();
            }
        }
        else if (order == "quit")
        {
            if (searchThread.joinable())
            {
                Search::stopRequested = true;
                Search::active = false;
                searchThread.join();
            }
            threadContinue = false;
        }
        else if (order == "stop")
        {
            if (searchThread.joinable())
            {
                Search::stopRequested = true;
                Search::active = false;
                searchThread.join();
            }
        }
        if (!threadContinue)
            continue;

        if (order.length() >= 8 && order.substr(0, 8) == "position")
        {
            if (searchThread.joinable())
            {
                Search::stopRequested = true;
                Search::active = false;
                searchThread.join();
            }
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
            thisBoard->nnueHistory->snapshotCount = 0;
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
            if (order.rfind("setoption name SyzygyPath", 0) == 0)
            {
                const std::string marker = " value ";
                const std::size_t valueAt = order.find(marker);
                Tablebase::Path = valueAt == std::string::npos
                    ? std::string() : order.substr(valueAt + marker.size());
                if (!Tablebase::Initialize(Tablebase::Path) &&
                    !Tablebase::Path.empty())
                    out << "info string no Syzygy tablebases found at path\n";
                continue;
            }
            if (order.rfind("setoption name SyzygyProbeLimit", 0) == 0)
            {
                const std::string marker = " value ";
                const std::size_t valueAt = order.find(marker);
                if (valueAt != std::string::npos)
                {
                    try
                    {
                        Tablebase::ProbeLimit = std::clamp(
                            std::stoi(order.substr(valueAt + marker.size())), 0, 7);
                    }
                    catch (const std::exception &)
                    {
                        out << "info string invalid SyzygyProbeLimit value\n";
                    }
                }
                continue;
            }
            if (order.rfind("setoption name MultiPV", 0) == 0)
            {
                const std::string marker = " value ";
                const std::size_t valueAt = order.find(marker);
                if (valueAt != std::string::npos)
                {
                    try
                    {
                        Search::MultiPV = std::clamp(
                            std::stoi(order.substr(valueAt + marker.size())), 1, 99);
                    }
                    catch (const std::exception &)
                    {
                        out << "info string invalid MultiPV value\n";
                    }
                }
                continue;
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
            thisBoard.nnueHistory->snapshotCount = 0;
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
    thisBoard.nnueHistory->snapshotCount = 0;
    delete lastMoves->Move1;
    lastMoves->Move1 = lastMoves->Move2;
    lastMoves->Move2 = lastMoves->Move3;
    lastMoves->Move3 = lastMoves->Move4;
    lastMoves->Move4 = MoveLogic::MoveCopy(doneMove2);
    delete doneMove2;
    doneMove2 = nullptr;
    return lastMoves;
}
