#include "BoardInitializer.h"
#include "BoardLogic.h"
#include "BoardMaker.h"
#include "GameLogic.h"
#include "KingSetup.h"
#include "MoveLogic.h"
#include "Option.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"

#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
struct PerftExpectation
{
    int depth;
    std::uint64_t nodes;
};

struct PerftPosition
{
    const char* name;
    const char* fen;
    std::vector<PerftExpectation> expectations;
};

const std::vector<PerftPosition> Positions = {
    {
        "start",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        {{1, 20}, {2, 400}, {3, 8902}}
    },
    {
        "kiwipete",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        {{1, 48}, {2, 2039}}
    },
    {
        "position3",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        {{1, 14}, {2, 191}, {3, 2812}}
    },
    {
        "position4",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        {{1, 6}, {2, 264}}
    },
    {
        "position5",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        {{1, 44}, {2, 1486}}
    },
    {
        "position6",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        {{1, 46}, {2, 2079}}
    }
};

class GeneratedMoves
{
public:
    explicit GeneratedMoves(Board& board, int depth, int depthGone)
        : moveList(MoveLogic::MoveGenerator(board, depth, depthGone))
    {
    }

    ~GeneratedMoves()
    {
        for (int counter = 0; counter < moveList.count; counter++)
        {
            delete moveList.moves[counter];
            moveList.moves[counter] = nullptr;
        }
    }

    GeneratedMoves(const GeneratedMoves&) = delete;
    GeneratedMoves& operator=(const GeneratedMoves&) = delete;

    MoveList moveList;
};

void InitializeEngine()
{
    Option::Initialize();
    AttackPlaces::Initialize();
    BoardInitializer::Initialize();
    PieceMoves::Initialize();
    MoveLogic::Initialize();
    KingSetup::Initialize();
    PassedPawnSetup::Initialize();
}

const PerftPosition& FindPosition(const std::string& name)
{
    for (const PerftPosition& position : Positions)
    {
        if (name == position.name)
        {
            return position;
        }
    }

    throw std::runtime_error("Unknown test position: " + name);
}

std::unique_ptr<Board> MakeBoard(const PerftPosition& position)
{
    return std::unique_ptr<Board>(BoardMaker::MakeInitialBoard(position.fen));
}

bool IsLegalAfterMove(Board& board, int movingSide)
{
    return !BoardLogic::UnderAttack(
        board,
        board.pieces[movingSide * 8 + 6].front(),
        board.sideToMove);
}

std::string MoveToString(const Move& move)
{
    std::string text;
    text += static_cast<char>('a' + move.beginPlace % 8);
    text += static_cast<char>('1' + move.beginPlace / 8);
    text += static_cast<char>('a' + move.endPlace % 8);
    text += static_cast<char>('1' + move.endPlace / 8);

    switch (move.promotionPiece)
    {
    case 2:
    case 10:
        text += 'n';
        break;
    case 3:
    case 11:
        text += 'b';
        break;
    case 4:
    case 12:
        text += 'r';
        break;
    case 5:
    case 13:
        text += 'q';
        break;
    }

    return text;
}

void RequireSemanticEquality(Board& actual, Board& expected)
{
    Board::AreBoardsEqual(actual, expected);
    if (actual.moveNumber != expected.moveNumber)
    {
        std::ostringstream message;
        message << "Move numbers are not equal: actual=" << actual.moveNumber
                << ", expected=" << expected.moveNumber;
        throw std::runtime_error(message.str());
    }
}

std::uint64_t Perft(Board& board, int depth, Move& previousMove)
{
    if (depth == 0)
    {
        return 1;
    }

    GeneratedMoves generatedMoves(board, depth, 0);
    std::uint64_t nodes = 0;
    int movingSide = board.sideToMove ? 1 : 0;

    for (int counter = 0; counter < generatedMoves.moveList.count; counter++)
    {
        Move& move = *generatedMoves.moveList.moves[counter];
        MissingInfoAboutPrevStateFromMove missingInfo(board);
        GameLogic::DoMove(board, move, previousMove, depth, 0);
        bool legal = IsLegalAfterMove(board, movingSide);

        if (legal)
        {
            nodes += Perft(board, depth - 1, move);
        }

        GameLogic::UndoMove(board, move, missingInfo);
    }

    return nodes;
}

void PrintRootDivide(const PerftPosition& position, int depth)
{
    std::unique_ptr<Board> board = MakeBoard(position);
    GeneratedMoves generatedMoves(*board, depth, 0);
    Move previousMove{};
    int movingSide = board->sideToMove ? 1 : 0;
    std::uint64_t total = 0;

    std::cout << "Root divide for " << position.name << " at depth " << depth << ":\n";
    for (int counter = 0; counter < generatedMoves.moveList.count; counter++)
    {
        Move& move = *generatedMoves.moveList.moves[counter];
        std::string moveText = MoveToString(move);
        MissingInfoAboutPrevStateFromMove missingInfo(*board);
        GameLogic::DoMove(*board, move, previousMove, depth, 0);
        bool legal = IsLegalAfterMove(*board, movingSide);

        if (legal)
        {
            std::uint64_t nodes = Perft(*board, depth - 1, move);
            total += nodes;
            std::cout << "  " << moveText << ": " << nodes << '\n';
        }

        GameLogic::UndoMove(*board, move, missingInfo);
    }
    std::cout << "  Total: " << total << '\n';
}

int RunPerft(const PerftPosition& position)
{
    std::string firstRestorationFailure;

    for (const PerftExpectation& expectation : position.expectations)
    {
        std::unique_ptr<Board> board = MakeBoard(position);
        std::unique_ptr<Board> original(board->MakeCopy());
        Move previousMove{};
        std::uint64_t actual = Perft(*board, expectation.depth, previousMove);

        if (actual != expectation.nodes)
        {
            std::cerr << "PERFT mismatch for " << position.name
                      << " at shallowest failing depth " << expectation.depth
                      << ": expected " << expectation.nodes
                      << ", actual " << actual << '\n';
            PrintRootDivide(position, expectation.depth);
            return 1;
        }

        try
        {
            RequireSemanticEquality(*board, *original);
        }
        catch (const std::exception& error)
        {
            if (firstRestorationFailure.empty())
            {
                std::ostringstream message;
                message << "PERFT restored the expected node count for " << position.name
                        << " at depth " << expectation.depth
                        << " but did not restore the root state: " << error.what();
                firstRestorationFailure = message.str();
            }
        }

        std::cout << "PERFT " << position.name << " depth " << expectation.depth
                  << ": " << actual << " nodes\n";
    }

    if (!firstRestorationFailure.empty())
    {
        std::cerr << firstRestorationFailure << '\n';
        return 1;
    }

    return 0;
}

int RunRestoration(const PerftPosition& position)
{
    std::unique_ptr<Board> board = MakeBoard(position);
    GeneratedMoves generatedMoves(*board, 1, 0);
    Move previousMove{};
    int movingSide = board->sideToMove ? 1 : 0;
    int legalMoveCount = 0;

    for (int counter = 0; counter < generatedMoves.moveList.count; counter++)
    {
        Move& move = *generatedMoves.moveList.moves[counter];
        std::string moveText = MoveToString(move);
        std::unique_ptr<Board> original(board->MakeCopy());
        MissingInfoAboutPrevStateFromMove missingInfo(*board);
        GameLogic::DoMove(*board, move, previousMove, 1, 0);
        bool legal = IsLegalAfterMove(*board, movingSide);
        GameLogic::UndoMove(*board, move, missingInfo);

        if (!legal)
        {
            continue;
        }

        legalMoveCount++;
        try
        {
            RequireSemanticEquality(*board, *original);
        }
        catch (const std::exception& error)
        {
            std::cerr << "State restoration mismatch for " << position.name
                      << " after " << moveText << ": " << error.what() << '\n';
            return 1;
        }
    }

    std::cout << "State restoration " << position.name << ": "
              << legalMoveCount << " legal root moves restored\n";
    return 0;
}
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: howl_correctness_tests <perft|restoration> <position>\n";
        return 2;
    }

    try
    {
        InitializeEngine();
        const PerftPosition& position = FindPosition(argv[2]);
        std::string testType = argv[1];

        if (testType == "perft")
        {
            return RunPerft(position);
        }
        if (testType == "restoration")
        {
            return RunRestoration(position);
        }

        throw std::runtime_error("Unknown test type: " + testType);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Test aborted: " << error.what() << '\n';
        return 1;
    }
}
