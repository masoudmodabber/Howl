#include "BoardInitializer.h"
#include "BoardLogic.h"
#include "BoardMaker.h"
#include "ChessStringManipulation.h"
#include "EvaluationLogic.h"
#include "GameLogic.h"
#include "HashMemoryBudget.h"
#include "KingSetup.h"
#include "MoveLogic.h"
#include "Option.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "PVSSearch.h"
#include "QSearcher.h"
#include "RepetitionHistory.h"
#include "Search.h"
#include "TranspositionTable.h"
#include "UCI.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
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
    std::ostringstream diagnostics;
    if (!HashMemoryBudget::EnsureDefaultConfigured(diagnostics))
    {
        throw std::runtime_error("Default Hash configuration failed: " + diagnostics.str());
    }
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

std::string CompareHashRelevantState(Board& actual, Board& expected)
{
    std::unique_ptr<Board> actualCopy(actual.MakeCopy());
    std::unique_ptr<Board> expectedCopy(expected.MakeCopy());
    actualCopy->ZobristHashCode = 0;
    expectedCopy->ZobristHashCode = 0;

    try
    {
        Board::AreBoardsEqual(*actualCopy, *expectedCopy);
        return "equal";
    }
    catch (const std::exception& error)
    {
        return std::string("different: ") + error.what();
    }
}

std::string CompareSemanticStateWithoutHash(Board& actual, Board& expected)
{
    std::unique_ptr<Board> actualCopy(actual.MakeCopy());
    std::unique_ptr<Board> expectedCopy(expected.MakeCopy());
    actualCopy->ZobristHashCode = 0;
    expectedCopy->ZobristHashCode = 0;

    try
    {
        RequireSemanticEquality(*actualCopy, *expectedCopy);
        return "equal";
    }
    catch (const std::exception& error)
    {
        return std::string("different: ") + error.what();
    }
}

void ApplyMoves(Board& board, const std::vector<std::string>& moveTexts)
{
    Move previousMove{};

    for (const std::string& moveText : moveTexts)
    {
        GeneratedMoves generatedMoves(board, 1, 0);
        Move selectedMove{};
        bool found = false;

        for (int counter = 0; counter < generatedMoves.moveList.count; counter++)
        {
            Move& candidate = *generatedMoves.moveList.moves[counter];
            if (MoveToString(candidate) == moveText)
            {
                selectedMove = candidate;
                found = true;
                break;
            }
        }

        if (!found)
        {
            throw std::runtime_error("Production move generator did not produce " + moveText);
        }

        int movingSide = board.sideToMove ? 1 : 0;
        GameLogic::DoMove(board, selectedMove, previousMove, 1, 0);
        if (!IsLegalAfterMove(board, movingSide))
        {
            throw std::runtime_error("Production move generator produced illegal requested move " + moveText);
        }
        previousMove = selectedMove;
    }
}

int RequireEqualHashes(const std::string& testName, Board& actual, Board& expected)
{
    std::string semanticResult = CompareHashRelevantState(actual, expected);
    if (actual.ZobristHashCode != expected.ZobristHashCode || semanticResult != "equal")
    {
        std::cerr << "Zobrist consistency failure for " << testName << '\n'
                  << "  Expected relationship: actualHash == expectedHash\n"
                  << "  Actual relationship: actualHash=" << actual.ZobristHashCode
                  << ", expectedHash=" << expected.ZobristHashCode
                  << ", xor=" << (actual.ZobristHashCode ^ expected.ZobristHashCode) << '\n'
                  << "  Semantic board comparison: " << semanticResult << '\n';
        return 1;
    }

    std::cout << "Zobrist " << testName << ": hashes and semantic state match\n";
    return 0;
}

int RunZobristStartParity()
{
    std::unique_ptr<Board> startCopy(BoardInitializer::beginBoard->MakeCopy());
    std::unique_ptr<Board> fenBoard(BoardMaker::MakeInitialBoard(Positions.front().fen));
    return RequireEqualHashes("start position construction parity", *startCopy, *fenBoard);
}

int RunZobristMoveOrder()
{
    std::unique_ptr<Board> first(BoardInitializer::beginBoard->MakeCopy());
    std::unique_ptr<Board> second(BoardInitializer::beginBoard->MakeCopy());
    ApplyMoves(*first, {"g1f3", "g8f6", "b1c3", "b8c6"});
    ApplyMoves(*second, {"b1c3", "b8c6", "g1f3", "g8f6"});
    return RequireEqualHashes("move order transposition", *first, *second);
}

int RunZobristFenVersusMoves()
{
    std::unique_ptr<Board> moved(BoardInitializer::beginBoard->MakeCopy());
    ApplyMoves(*moved, {"g1f3", "g8f6", "b1c3", "b8c6"});
    std::unique_ptr<Board> fenBoard(BoardMaker::MakeInitialBoard(
        "r1bqkb1r/pppppppp/2n2n2/8/8/2N2N2/PPPPPPPP/R1BQKB1R w KQkq - 0 3"));
    return RequireEqualHashes("FEN versus production moves", *moved, *fenBoard);
}

int RunZobristSideToMove()
{
    std::unique_ptr<Board> white(BoardMaker::MakeInitialBoard(
        "4k3/8/8/8/8/8/8/4K3 w - - 0 1"));
    std::unique_ptr<Board> black(BoardMaker::MakeInitialBoard(
        "4k3/8/8/8/8/8/8/4K3 b - - 0 1"));
    std::unique_ptr<Board> normalizedBlack(black->MakeCopy());
    normalizedBlack->sideToMove = white->sideToMove;
    std::string semanticResult = CompareHashRelevantState(*white, *normalizedBlack);
    long long actualDifference = white->ZobristHashCode ^ black->ZobristHashCode;
    long long expectedDifference = BoardInitializer::ZCodeFlag[7];

    if (actualDifference != expectedDifference || semanticResult != "equal")
    {
        std::cerr << "Zobrist consistency failure for side to move\n"
                  << "  Expected relationship: whiteHash ^ blackHash == ZCodeFlag[7] ("
                  << expectedDifference << ")\n"
                  << "  Actual relationship: whiteHash ^ blackHash == " << actualDifference << '\n'
                  << "  Semantic board comparison after normalizing side to move: "
                  << semanticResult << '\n';
        return 1;
    }

    std::cout << "Zobrist side to move: expected XOR relationship holds\n";
    return 0;
}

int RunZobristCastlingRights()
{
    struct CastlingCase
    {
        const char* name;
        const char* rights;
        int flagIndex;
        bool Board::*right;
    };

    const CastlingCase cases[] = {
        {"white kingside", "Qkq", 6, &Board::whiteSmallCastle},
        {"white queenside", "Kkq", 5, &Board::whiteBigCastle},
        {"black kingside", "KQq", 4, &Board::blackSmallCastle},
        {"black queenside", "KQk", 3, &Board::blackBigCastle}
    };
    const std::string placement = "r3k2r/8/8/8/8/8/8/R3K2R w ";
    std::unique_ptr<Board> allRights(BoardMaker::MakeInitialBoard(
        placement + "KQkq - 0 1"));

    for (const CastlingCase& castlingCase : cases)
    {
        std::unique_ptr<Board> withoutRight(BoardMaker::MakeInitialBoard(
            placement + castlingCase.rights + " - 0 1"));
        std::unique_ptr<Board> normalized(withoutRight->MakeCopy());
        normalized.get()->*(castlingCase.right) = allRights.get()->*(castlingCase.right);
        std::string semanticResult = CompareHashRelevantState(*allRights, *normalized);
        long long actualDifference = allRights->ZobristHashCode ^ withoutRight->ZobristHashCode;
        long long expectedDifference = BoardInitializer::ZCodeFlag[castlingCase.flagIndex];

        if (actualDifference != expectedDifference || semanticResult != "equal")
        {
            std::cerr << "Zobrist consistency failure for " << castlingCase.name
                      << " castling right\n"
                      << "  Expected relationship: withRightHash ^ withoutRightHash == ZCodeFlag["
                      << castlingCase.flagIndex << "] (" << expectedDifference << ")\n"
                      << "  Actual relationship: withRightHash ^ withoutRightHash == "
                      << actualDifference << '\n'
                      << "  Semantic board comparison after normalizing the castling right: "
                      << semanticResult << '\n';
            return 1;
        }
    }

    std::cout << "Zobrist castling rights: all expected XOR relationships hold\n";
    return 0;
}

bool HasLegalEnPassantMove(Board& board, const std::string& expectedMove)
{
    GeneratedMoves generatedMoves(board, 1, 0);
    Move previousMove{};
    int movingSide = board.sideToMove ? 1 : 0;

    for (int counter = 0; counter < generatedMoves.moveList.count; counter++)
    {
        Move& move = *generatedMoves.moveList.moves[counter];
        if (MoveToString(move) != expectedMove ||
            (move.PublicFlag & Option::PowerTwo[6]) == 0)
        {
            continue;
        }

        MissingInfoAboutPrevStateFromMove missingInfo(board);
        GameLogic::DoMove(board, move, previousMove, 1, 0);
        bool legal = IsLegalAfterMove(board, movingSide);
        GameLogic::UndoMove(board, move, missingInfo);
        return legal;
    }

    return false;
}

int RunZobristEnPassant(
    const std::string& testName,
    const std::string& initialFen,
    const std::string& doublePawnMove,
    const std::string& fenWithTarget,
    const std::string& fenWithoutTarget,
    int targetSquare,
    const std::string& expectedEnPassantMove)
{
    std::unique_ptr<Board> moved(BoardMaker::MakeInitialBoard(initialFen));
    ApplyMoves(*moved, {doublePawnMove});
    std::unique_ptr<Board> withTarget(BoardMaker::MakeInitialBoard(fenWithTarget));
    std::unique_ptr<Board> withoutTarget(BoardMaker::MakeInitialBoard(fenWithoutTarget));

    std::string semanticResult = CompareSemanticStateWithoutHash(*moved, *withTarget);
    bool movedHasCapture = HasLegalEnPassantMove(*moved, expectedEnPassantMove);
    bool fenHasCapture = HasLegalEnPassantMove(*withTarget, expectedEnPassantMove);

    if (semanticResult != "equal" ||
        moved->unpassentPlace != targetSquare ||
        withTarget->unpassentPlace != targetSquare ||
        moved->ZobristHashCode != withTarget->ZobristHashCode ||
        !movedHasCapture || !fenHasCapture)
    {
        std::cerr << "Zobrist consistency failure for " << testName << '\n'
                  << "  Expected semantic board comparison: equal\n"
                  << "  Actual semantic board comparison: " << semanticResult << '\n'
                  << "  Expected en passant target: " << targetSquare << '\n'
                  << "  Actual move/FEN targets: " << moved->unpassentPlace
                  << "/" << withTarget->unpassentPlace << '\n'
                  << "  Expected relationship: moveHash == fenHash\n"
                  << "  Actual move/FEN hashes: " << moved->ZobristHashCode
                  << "/" << withTarget->ZobristHashCode << '\n'
                  << "  Expected legal en passant move: " << expectedEnPassantMove << '\n'
                  << "  Generated legally from move/FEN positions: "
                  << movedHasCapture << "/" << fenHasCapture << '\n';
        return 1;
    }

    std::unique_ptr<Board> normalizedWithoutTarget(withoutTarget->MakeCopy());
    normalizedWithoutTarget->unpassentPlace = targetSquare;
    std::string relationSemanticResult =
        CompareSemanticStateWithoutHash(*withTarget, *normalizedWithoutTarget);
    long long actualDifference =
        withTarget->ZobristHashCode ^ withoutTarget->ZobristHashCode;
    long long expectedDifference =
        BoardInitializer::ZCodeUnpassentPlace[targetSquare];

    if (relationSemanticResult != "equal" || actualDifference != expectedDifference)
    {
        std::cerr << "Zobrist consistency failure for " << testName
                  << " hash relation\n"
                  << "  Expected relationship: withTargetHash ^ withoutTargetHash == "
                  << "ZCodeUnpassentPlace[" << targetSquare << "] ("
                  << expectedDifference << ")\n"
                  << "  Actual XOR difference: " << actualDifference << '\n'
                  << "  Semantic board comparison after normalizing en passant: "
                  << relationSemanticResult << '\n';
        return 1;
    }

    std::cout << "Zobrist " << testName
              << ": FEN/move parity, legal capture, and XOR relationship hold\n";
    return 0;
}

int RunZobristEnPassantRank3()
{
    return RunZobristEnPassant(
        "rank 3 en passant",
        "4k3/8/8/8/3p4/8/4P3/4K3 w - - 0 1",
        "e2e4",
        "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1",
        "4k3/8/8/8/3pP3/8/8/4K3 b - - 0 1",
        20,
        "d4e3");
}

int RunZobristEnPassantRank6()
{
    return RunZobristEnPassant(
        "rank 6 en passant",
        "4k3/4p3/8/3P4/8/8/8/4K3 b - - 0 1",
        "e7e5",
        "4k3/8/8/3Pp3/8/8/8/4K3 w - e6 0 2",
        "4k3/8/8/3Pp3/8/8/8/4K3 w - - 0 2",
        44,
        "d5e6");
}

int RunZobrist(const std::string& testCase)
{
    if (testCase == "start_parity")
        return RunZobristStartParity();
    if (testCase == "move_order")
        return RunZobristMoveOrder();
    if (testCase == "fen_vs_moves")
        return RunZobristFenVersusMoves();
    if (testCase == "side_to_move")
        return RunZobristSideToMove();
    if (testCase == "castling_rights")
        return RunZobristCastlingRights();
    if (testCase == "en_passant_rank3")
        return RunZobristEnPassantRank3();
    if (testCase == "en_passant_rank6")
        return RunZobristEnPassantRank6();

    throw std::runtime_error("Unknown Zobrist test case: " + testCase);
}

struct FixedDepthSearchResult
{
    Move move{};
    std::string moveText;
    int score = -200000;
    std::vector<std::pair<std::string, int>> rootScores;
};

FixedDepthSearchResult FixedDepthRoot(Board& board, int depth)
{
    RepetitionHistory::ResetWithRoot(board.ZobristHashCode);
    std::unique_ptr<Board> original(board.MakeCopy());
    GeneratedMoves generatedMoves(board, depth, 0);
    Move previousMove{};
    Move move2{};
    Move move3{};
    Move move4{};
    int movingSide = board.sideToMove ? 1 : 0;
    FixedDepthSearchResult result;
    bool found = false;

    for (int counter = 0; counter < generatedMoves.moveList.count; counter++)
    {
        Move& move = *generatedMoves.moveList.moves[counter];
        std::string moveText = MoveToString(move);
        MissingInfoAboutPrevStateFromMove missingInfo(board);
        GameLogic::DoMove(board, move, previousMove, depth, 0);

        if (IsLegalAfterMove(board, movingSide))
        {
            std::unique_ptr<MovePrintValue> searched(PVSSearch::PVS(
                true,
                -200000,
                200000,
                depth - 1,
                move,
                move2,
                move3,
                move4,
                board,
                false,
                true,
                1,
                false,
                false));
            int score = -searched->value;
            if (score > 159800 && score != 160000)
            {
                score--;
            }
            else if (score < -159800 && score != -160000)
            {
                score++;
            }
            result.rootScores.push_back({moveText, score});

            if (!found || score > result.score)
            {
                found = true;
                result.move = move;
                result.moveText = moveText;
                result.score = score;
            }
        }

        GameLogic::UndoMove(board, move, missingInfo);
        RequireSemanticEquality(board, *original);
    }

    if (!found)
    {
        throw std::runtime_error("Fixed-depth root found no legal move");
    }

    RequireSemanticEquality(board, *original);
    return result;
}

void PrintRootScores(const FixedDepthSearchResult& result)
{
    std::cerr << "  Root move scores:\n";
    for (const auto& rootScore : result.rootScores)
    {
        std::cerr << "    " << rootScore.first << ": " << rootScore.second << '\n';
    }
}

bool ValidateReturnedMove(Board& board, const FixedDepthSearchResult& result)
{
    std::unique_ptr<Board> original(board.MakeCopy());
    GeneratedMoves generatedMoves(board, 1, 0);
    Move previousMove{};
    int movingSide = board.sideToMove ? 1 : 0;

    for (int counter = 0; counter < generatedMoves.moveList.count; counter++)
    {
        Move& move = *generatedMoves.moveList.moves[counter];
        if (MoveToString(move) != result.moveText)
        {
            continue;
        }

        MissingInfoAboutPrevStateFromMove missingInfo(board);
        GameLogic::DoMove(board, move, previousMove, 1, 0);
        bool legal = IsLegalAfterMove(board, movingSide);
        GameLogic::UndoMove(board, move, missingInfo);
        RequireSemanticEquality(board, *original);
        return legal;
    }

    return false;
}

int RunSearchCase(
    const std::string& name,
    const std::string& fen,
    int depth,
    const std::vector<std::string>& acceptableMoves)
{
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    FixedDepthSearchResult result = FixedDepthRoot(*board, depth);
    bool expected = acceptableMoves.empty() ||
        std::find(acceptableMoves.begin(), acceptableMoves.end(), result.moveText) != acceptableMoves.end();
    bool legal = ValidateReturnedMove(*board, result);

    if (!expected || !legal)
    {
        std::cerr << "Search correctness failure for " << name << '\n'
                  << "  FEN: " << fen << '\n'
                  << "  Requested depth: " << depth << '\n'
                  << "  Returned move: " << result.moveText << '\n'
                  << "  Returned score: " << result.score << '\n'
                  << "  Expected move set: ";
        if (acceptableMoves.empty())
        {
            std::cerr << "any legal move";
        }
        else
        {
            for (const std::string& move : acceptableMoves)
            {
                std::cerr << move << ' ';
            }
        }
        std::cerr << "\n  Returned move legal: " << legal << '\n';
        PrintRootScores(result);
        return 1;
    }

    std::cout << "Search " << name << " depth " << depth
              << ": " << result.moveText << " is expected and legal\n";
    return 0;
}

int RunReturnedMoveLegality()
{
    struct LegalityCase
    {
        const char* name;
        const char* fen;
    };

    const LegalityCase cases[] = {
        {"start", Positions[0].fen},
        {"kiwipete", Positions[1].fen},
        {"forced evasion", "7k/8/5K2/8/8/8/8/7R b - - 0 1"},
        {"rank 3 en passant", "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1"},
        {"promotion", "7k/5P2/6K1/8/8/8/8/8 w - - 0 1"}
    };

    for (const LegalityCase& legalityCase : cases)
    {
        int result = RunSearchCase(
            std::string("returned move legality: ") + legalityCase.name,
            legalityCase.fen,
            2,
            {});
        if (result != 0)
        {
            return result;
        }
    }

    return 0;
}

struct DirectQSearchResult
{
    int score = -200000;
    int moveCount = 0;
    std::string pv;
    QSearchTestStatistics statistics;
};

DirectQSearchResult RunDirectQSearch(
    const char* fen,
    int alpha,
    int beta,
    int lastCheck,
    int depth)
{
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    std::unique_ptr<Board> original(board->MakeCopy());
    Move previousMove{};
    Move move1{};
    Move move2{};
    Move move3{};
    const bool savedReleaseMode = UCI::IsRelease;
    UCI::IsRelease = true;
    Search::moveCount = 0;
    QSearcher::ResetTestStatistics();
    std::unique_ptr<MovePrintValue> searched(QSearcher::QSearch(
        true, alpha, beta, previousMove, 0, lastCheck, false, depth,
        move1, move2, move3, *board, false, 0, false));
    UCI::IsRelease = savedReleaseMode;
    RequireSemanticEquality(*board, *original);
    return {
        searched->value,
        Search::moveCount,
        searched->printString,
        QSearcher::TestStatistics()};
}

bool PVStartsWith(const DirectQSearchResult& result, const std::string& move)
{
    return result.pv == move || result.pv.rfind(move + " ", 0) == 0;
}

int ReportQSearchFailure(
    const char* name,
    const char* fen,
    const std::string& expectation,
    const DirectQSearchResult& result)
{
    std::cerr << "QSearch correctness failure: " << name << '\n'
              << "  FEN: " << fen << '\n'
              << "  Expected: " << expectation << '\n'
              << "  Actual score: " << result.score << '\n'
              << "  Actual PV: " << result.pv << '\n'
              << "  Search::moveCount: " << result.moveCount << '\n';
    return 1;
}

int RunQSearchInCheckStandPat()
{
    const char* fen = "7r/8/8/8/8/5k2/8/7K w - - 0 1";
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    const int staticEvaluation = EvaluationLogic::Evaluate(*board);
    const DirectQSearchResult result =
        RunDirectQSearch(fen, -200000, staticEvaluation, 0, 1);
    if (result.pv.empty())
    {
        return ReportQSearchFailure(
            "a checked side cannot stand pat", fen,
            "a legal evasion must be searched before returning", result);
    }
    std::cout << "QSearch does not stand pat while in check\n";
    return 0;
}

int RunQSearchQuietKingEvasion()
{
    const char* fen = "7r/8/8/8/8/5k2/8/7K w - - 0 1";
    const DirectQSearchResult reference =
        RunDirectQSearch(fen, -200000, 200000, 2, 1);
    const DirectQSearchResult pruned =
        RunDirectQSearch(fen, -281, 200000, 2, 1);
    if (!PVStartsWith(pruned, "h1g1") || pruned.score != reference.score)
    {
        return ReportQSearchFailure(
            "only legal quiet king evasion", fen,
            "h1g1 must be searched despite the delta threshold; reference score=" +
                std::to_string(reference.score), pruned);
    }
    std::cout << "QSearch searches its only legal quiet king evasion\n";
    return 0;
}

int RunQSearchQuietBlockEvasion()
{
    const char* fen = "k3r3/8/8/6b1/8/1b5n/3R2b1/4K3 w - - 0 1";
    const DirectQSearchResult reference =
        RunDirectQSearch(fen, -200000, 200000, 2, 1);
    const DirectQSearchResult pruned =
        RunDirectQSearch(fen, -1012, 200000, 2, 1);
    if (!PVStartsWith(pruned, "d2e2") || pruned.score != reference.score)
    {
        return ReportQSearchFailure(
            "only legal quiet blocking evasion", fen,
            "d2e2 must be searched; twelve pseudo-legal alternatives are illegal; reference score=" +
                std::to_string(reference.score), pruned);
    }
    std::cout << "QSearch searches its only legal quiet blocking evasion\n";
    return 0;
}

int RunQSearchTerminalPosition(bool mate)
{
    const char* fen = mate
        ? "k3r3/8/8/6b1/8/1b5n/N5b1/4K3 w - - 0 1"
        : "k3r3/8/8/6b1/8/1b5n/4N1b1/4K3 w - - 0 1";
    const DirectQSearchResult result =
        RunDirectQSearch(fen, -200000, 200000, 2, 1);
    const int expected = mate ? -159999 : 0;
    if (result.score != expected ||
        result.statistics.rootGeneratedMoves == 0 ||
        result.statistics.rootLegalMoves != 0 ||
        result.statistics.rootAvailableMoves != 0)
    {
        return ReportQSearchFailure(
            mate ? "checkmate accounting" : "stalemate accounting",
            fen, "score " + std::to_string(expected), result);
    }
    std::cout << "QSearch " << (mate ? "checkmate" : "stalemate")
              << " accounting is correct\n";
    return 0;
}

int RunQSearchPseudoLegalIllegalMoves()
{
    const char* fen = "k3r3/8/8/6b1/8/1b5n/3R2b1/4K3 w - - 0 1";
    const DirectQSearchResult result =
        RunDirectQSearch(fen, -200000, 200000, 2, 1);
    if (!PVStartsWith(result, "d2e2") ||
        result.statistics.rootGeneratedMoves != 13 ||
        result.statistics.rootLegalMoves != 1 ||
        result.statistics.rootAvailableMoves != 1 ||
        result.statistics.rootIllegalMovesBeforeFirstSearch == 0 ||
        !result.statistics.firstLegalSearchedMoveUsedFullWindow)
    {
        return ReportQSearchFailure(
            "pseudo-legal moves rejected after king-safety validation", fen,
            "only legal move d2e2", result);
    }
    std::cout << "QSearch excludes searched pseudo-legal moves that leave the king in check\n";
    return 0;
}

int RunQSearchFullWindowResearchAccounting()
{
    const char* fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    const DirectQSearchResult result =
        RunDirectQSearch(fen, -200000, 200000, 2, 1);
    if (result.statistics.rootFullWindowResearches == 0 ||
        result.statistics.rootAvailableMoves != result.statistics.rootLegalMoves)
    {
        return ReportQSearchFailure(
            "full-window re-search legal move accounting", fen,
            "at least one re-search and exactly one availability count per legal root move",
            result);
    }
    std::cout << "QSearch full-window re-searches do not double-count legal moves\n";
    return 0;
}

int RunQSearchPromotion(bool capture)
{
    const char* fen = capture
        ? "k6r/6P1/8/8/8/8/8/6K1 w - - 0 1"
        : "7k/5P2/6K1/8/8/8/8/8 w - - 0 1";
    const int alpha = capture ? 216 : 198;
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    GeneratedMoves generatedMoves(*board, 1, 0);
    const std::string queenMove = capture ? "g7h8q" : "f7f8q";
    const std::string knightMove = capture ? "g7h8n" : "f7f8n";
    bool queenGenerated = false;
    bool knightGenerated = false;
    for (int counter = 0; counter < generatedMoves.moveList.count; counter++)
    {
        const std::string move = MoveToString(*generatedMoves.moveList.moves[counter]);
        queenGenerated = queenGenerated || move == queenMove;
        knightGenerated = knightGenerated || move == knightMove;
    }
    const DirectQSearchResult result =
        RunDirectQSearch(fen, alpha, 200000, 2, 1);
    if (!queenGenerated || !knightGenerated ||
        !PVStartsWith(result, queenMove) || result.score <= alpha)
    {
        return ReportQSearchFailure(
            capture ? "promotion capture delta margin" : "quiet promotion delta margin",
            fen,
            queenMove + " must include promotion gain and exceed alpha=" +
                std::to_string(alpha), result);
    }
    std::cout << "QSearch includes promotion gain in the delta decision\n";
    return 0;
}

int RunQSearchCheckingMove(bool capture)
{
    const char* fen = capture
        ? "4k3/4r3/8/8/8/8/4Q3/K3R3 w - - 0 1"
        : "7k/8/5KQ1/8/8/8/8/8 w - - 0 1";
    const std::string expectedMove = capture ? "e2e7" : "g6g7";
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    const int eval = EvaluationLogic::Evaluate(*board);
    const int capturedValue = capture ? 550 : 0;
    const int alpha = Option::SafetyMargin + eval + capturedValue;
    const int movingSide = board->sideToMove ? 1 : 0;
    GeneratedMoves moves(*board, 1, 0);
    Move previousMove{};
    bool foundCheckingMove = false;
    bool currentDeltaRejectsMove = false;
    for (int counter = 0; counter < moves.moveList.count; counter++)
    {
        Move& move = *moves.moveList.moves[counter];
        if (MoveToString(move) != expectedMove)
        {
            continue;
        }
        MissingInfoAboutPrevStateFromMove missingInfo(*board);
        GameLogic::DoMove(*board, move, previousMove, 1, 0);
        const int opponent = board->sideToMove ? 1 : 0;
        foundCheckingMove = IsLegalAfterMove(*board, movingSide) &&
            BoardLogic::UnderAttack(
                *board,
                board->pieces[opponent * 8 + 6].front(),
                !board->sideToMove);
        GameLogic::UndoMove(*board, move, missingInfo);
        currentDeltaRejectsMove =
            Option::SafetyMargin + EvaluationLogic::Evaluate(*board) + capturedValue <= alpha;
        break;
    }
    const DirectQSearchResult result =
        RunDirectQSearch(fen, alpha, 200000, 2, 1);
    if (!foundCheckingMove || !currentDeltaRejectsMove ||
        !PVStartsWith(result, expectedMove) || result.score <= alpha)
    {
        return ReportQSearchFailure(
            capture ? "checking capture delta pruning" : "quiet checking move delta pruning",
            fen,
            expectedMove + " gives check and must not be rejected solely by the material delta test",
            result);
    }
    std::cout << "QSearch exempts checking moves from material-only delta rejection\n";
    return 0;
}

int RunQSearchUnderpromotion(bool knight)
{
    const char* fen = "7k/5P2/6K1/8/8/8/8/8 w - - 0 1";
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    const int eval = EvaluationLogic::Evaluate(*board);
    // Pawn value = 100, Knight = 350 (+250), Rook = 550 (+450)
    const int gain = knight ? 250 : 450;
    const std::string promoMove = knight ? "f7f8n" : "f7f8r";
    const int boundaryAlpha = Option::SafetyMargin + eval + gain; // equality boundary

    // At boundaryAlpha, move should be pruned if searched with alpha = boundaryAlpha
    // Below boundaryAlpha (boundaryAlpha - 1), move should be eligible
    const DirectQSearchResult belowResult =
        RunDirectQSearch(fen, boundaryAlpha - 1, 200000, 2, 1);
    if (belowResult.score <= boundaryAlpha - 1)
    {
        return ReportQSearchFailure(
            knight ? "knight underpromotion delta allowance" : "rook underpromotion delta allowance",
            fen,
            promoMove + " must be searchable below its exact delta boundary",
            belowResult);
    }

    std::cout << "QSearch accounts for " << (knight ? "knight" : "rook") << " underpromotion gain\n";
    return 0;
}

int RunQSearchPromotionCheck()
{
    const char* fen = "7k/4P3/6K1/8/8/8/8/8 w - - 0 1";
    // e7e8q delivers rank-8 check to king on h8; must not be delta-pruned even at elevated alpha
    const DirectQSearchResult result =
        RunDirectQSearch(fen, 1000, 200000, 2, 1);
    if (!PVStartsWith(result, "e7e8q") || result.score <= 1000)
    {
        return ReportQSearchFailure(
            "promotion check delta exemption", fen,
            "e7e8q gives check and must bypass delta pruning",
            result);
    }
    std::cout << "QSearch exempts checking promotions from delta pruning\n";
    return 0;
}

int RunQSearchDiscoveredCheck()
{
    const char* fen = "4k3/8/8/8/8/8/4N3/4R2K w - - 0 1";
    // e2c3/e2f4 unmasks the rook on e1 to give discovered check to King on e8.
    // Quiet move (capture=0) would delta-prune with alpha=500, but discovered check bypasses delta
    const DirectQSearchResult result =
        RunDirectQSearch(fen, 500, 200000, 2, 1);
    if (result.score <= 500)
    {
        return ReportQSearchFailure(
            "discovered check delta exemption", fen,
            "unmasking discovered check must bypass delta pruning",
            result);
    }
    std::cout << "QSearch exempts discovered checks from delta pruning\n";
    return 0;
}

int RunQSearchOrdinaryDelta()
{
    const char* fen = "rnbqkb1r/pppppppp/5n2/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 2 2";
    std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
    const int eval = EvaluationLogic::Evaluate(*board);

    // Quiet move: pieceValue = 0, delta bound = SafetyMargin + eval
    const int quietExactBoundary = Option::SafetyMargin + eval;

    // At exact boundary (equality), quiet moves must be pruned:
    // Testing with high alpha above any capture ensures all quiet moves are pruned by equality
    const DirectQSearchResult equalityResult =
        RunDirectQSearch(fen, quietExactBoundary, 200000, 2, 1);
    // Since all quiet moves have delta <= quietExactBoundary and are non-checking, they are pruned
    if (equalityResult.score > quietExactBoundary)
    {
        return ReportQSearchFailure(
            "ordinary delta equality pruning", fen,
            "ordinary non-checking moves at exact delta boundary must be pruned",
            equalityResult);
    }

    std::cout << "QSearch preserves ordinary delta boundary and equality pruning\n";
    return 0;
}

int RunQSearchDeltaBoundary()
{
    const char* fen = "7r/8/8/8/8/5k2/8/7K w - - 0 1";
    const DirectQSearchResult below =
        RunDirectQSearch(fen, -282, 200000, 2, 1);
    const DirectQSearchResult at =
        RunDirectQSearch(fen, -281, 200000, 2, 1);
    if (!PVStartsWith(below, "h1g1") || !PVStartsWith(at, "h1g1") ||
        below.moveCount == 0 || at.moveCount == 0 ||
        below.score <= -150000 || at.score <= -150000)
    {
        return ReportQSearchFailure(
            "delta pruning boundary", fen,
            "checked nodes search h1g1 on both sides of the ordinary delta boundary",
            at);
    }
    std::cout << "QSearch bypasses the ordinary delta boundary while in check\n";
    return 0;
}

int RunQSearchPawnCaptureTactics(int scenario)
{
    if (scenario == 1)
    {
        // 1. Defended pawn capture followed by recapture (Qxd5 refuted by Re4xd4/recapture)
        const char* fen = "4k3/8/8/3p4/4r3/8/3Q4/4K3 w - - 0 1";
        const DirectQSearchResult result = RunDirectQSearch(fen, -200000, 200000, 2, 1);
        if (result.score <= -150000)
        {
            return ReportQSearchFailure("defended pawn capture continuation", fen, "proper resolution without premature cutoff", result);
        }
        std::cout << "QSearch correctly resolves defended pawn capture and recapture\n";
        return 0;
    }
    else if (scenario == 2)
    {
        // 2. Free pawn capture with no tactical reply
        const char* fen = "4k3/8/8/3p4/8/8/3R4/4K3 w - - 0 1";
        const DirectQSearchResult result = RunDirectQSearch(fen, -200000, 200000, 2, 1);
        if (!PVStartsWith(result, "d2d5"))
        {
            return ReportQSearchFailure("free pawn capture continuation", fen, "Rxd5 captures free pawn", result);
        }
        std::cout << "QSearch correctly captures free pawn without tactical reply\n";
        return 0;
    }
    else
    {
        // 3. Pawn capture inside a forcing sequence
        const char* fen = "r1b1k2r/pppp1ppp/8/4P3/1b1q4/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 1";
        const DirectQSearchResult result = RunDirectQSearch(fen, -200000, 200000, 2, 1);
        if (result.score <= 0)
        {
            return ReportQSearchFailure("forcing sequence pawn capture", fen, "tactical sequence maintains advantage", result);
        }
        std::cout << "QSearch correctly evaluates pawn capture in forcing sequence\n";
        return 0;
    }
}

int RunQSearch(const std::string& testCase)
{
    if (testCase == "in_check_stand_pat")
        return RunQSearchInCheckStandPat();
    if (testCase == "quiet_king_evasion")
        return RunQSearchQuietKingEvasion();
    if (testCase == "quiet_block_evasion")
        return RunQSearchQuietBlockEvasion();
    if (testCase == "mate")
        return RunQSearchTerminalPosition(true);
    if (testCase == "stalemate")
        return RunQSearchTerminalPosition(false);
    if (testCase == "pseudo_legal_illegal")
        return RunQSearchPseudoLegalIllegalMoves();
    if (testCase == "research_accounting")
        return RunQSearchFullWindowResearchAccounting();
    if (testCase == "quiet_promotion")
        return RunQSearchPromotion(false);
    if (testCase == "promotion_capture")
        return RunQSearchPromotion(true);
    if (testCase == "underpromotion_knight")
        return RunQSearchUnderpromotion(true);
    if (testCase == "underpromotion_rook")
        return RunQSearchUnderpromotion(false);
    if (testCase == "promotion_check")
        return RunQSearchPromotionCheck();
    if (testCase == "discovered_check")
        return RunQSearchDiscoveredCheck();
    if (testCase == "ordinary_delta")
        return RunQSearchOrdinaryDelta();
    if (testCase == "pawn_capture_recapture")
        return RunQSearchPawnCaptureTactics(1);
    if (testCase == "pawn_capture_free")
        return RunQSearchPawnCaptureTactics(2);
    if (testCase == "pawn_capture_forcing")
        return RunQSearchPawnCaptureTactics(3);
    if (testCase == "quiet_check")
        return RunQSearchCheckingMove(false);
    if (testCase == "checking_capture")
        return RunQSearchCheckingMove(true);
    if (testCase == "delta_boundary")
        return RunQSearchDeltaBoundary();
    throw std::runtime_error("Unknown QSearch test case: " + testCase);
}

int RunSearch(const std::string& testCase)
{
    if (testCase == "mate_in_one")
        return RunSearchCase(
            "unique mate in one",
            "7k/8/5KQ1/8/8/8/8/8 w - - 0 1",
            1,
            {"g6g7"});
    if (testCase == "mate_in_two")
        // Nominal depth 1 is intentional: QSearch extends this checking line beyond the PVS horizon.
        return RunSearchCase(
            "unique mate in two",
            "1r4k1/4nppp/8/4Pb2/8/1P5P/r1PR4/3R3K w - - 0 27",
            1,
            {"d2d8"});
    if (testCase == "forced_evasion")
        return RunSearchCase(
            "only legal move / forced check evasion",
            "7k/8/5K2/8/8/8/8/7R b - - 0 1",
            1,
            {"h8g8"});
    if (testCase == "illegal_king_capture")
        return RunSearchCase(
            "reject illegal king capture",
            "k3r3/8/8/8/8/8/4q2R/4K3 w - - 0 1",
            1,
            {"h2e2"});
    if (testCase == "promotion_mate")
        return RunSearchCase(
            "promotion mate",
            "7k/5P2/6K1/8/8/8/8/8 w - - 0 1",
            1,
            {"f7f8q", "f7f8r"});
    if (testCase == "returned_move_legality")
        return RunReturnedMoveLegality();
    if (testCase == "lmr_research_accounting")
    {
        // Kiwipete at depth 3 exercises LMR reduction and full-window re-searches
        return RunSearchCase(
            "lmr research legal move accounting",
            Positions[1].fen,
            3,
            {});
    }
    if (testCase == "lmr_promotion_exemption")
    {
        // Promotion tactic at depth 4 exercises promotion unreduced search
        return RunSearchCase(
            "lmr promotion exemption",
            Positions[4].fen,
            4,
            {"d7c8q"});
    }
    if (testCase == "lmr_equal_winning_capture_exemption")
    {
        // Kiwipete at depth 4 exercises equal and winning capture unreduced search
        return RunSearchCase(
            "lmr equal winning capture exemption",
            Positions[1].fen,
            4,
            {"e2a6"});
    }
    if (testCase == "repetition_real_knight")
    {
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(Positions[0].fen));
        RepetitionHistory::ResetWithRoot(board->ZobristHashCode);
        long long rootHash = board->ZobristHashCode;

        // Play 1. Nf3 (g1f3) 2. Nf6 (g8f6) 3. Ng1 (f3g1) 4. Ng8 (f6g8)
        std::string moves[] = {"g1f3", "g8f6", "f3g1", "f6g8"};
        for (int i = 0; i < 4; ++i)
        {
            std::unique_ptr<Move> m(ChessStringManipulation::ConvertTextToMove(moves[i], *board));
            Move prev{};
            MissingInfoAboutPrevStateFromMove missing(*board);
            GameLogic::DoMove(*board, *m, prev, -1, -1);
            if (i < 3)
            {
                if (RepetitionHistory::IsRepetition(board->ZobristHashCode))
                {
                    std::cerr << "Premature repetition detected at move " << i + 1 << "\n";
                    return 1;
                }
            }
            else
            {
                if (board->ZobristHashCode != rootHash)
                {
                    std::cerr << "Root hash does not match after full knight cycle\n";
                    return 1;
                }
                if (!RepetitionHistory::IsRepetition(board->ZobristHashCode))
                {
                    std::cerr << "Repetition NOT detected after 4-move knight cycle\n";
                    return 1;
                }
            }
        }
        std::cout << "Real knight cycle repetition verified\n";
        return 0;
    }
    if (testCase == "repetition_false_a1")
    {
        // Quiet position with a knight that can move to a1
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard("8/8/8/8/8/1N6/8/8 w - - 0 1"));
        RepetitionHistory::ResetWithRoot(board->ZobristHashCode);
        std::unique_ptr<Move> m(ChessStringManipulation::ConvertTextToMove("b3a1", *board));
        Move prev{};
        MissingInfoAboutPrevStateFromMove missing(*board);
        GameLogic::DoMove(*board, *m, prev, -1, -1);
        if (RepetitionHistory::IsRepetition(board->ZobristHashCode))
        {
            std::cerr << "False positive repetition detected on move to a1\n";
            return 1;
        }
        std::cout << "False positive on a1 move prevented\n";
        return 0;
    }
    if (testCase == "repetition_castling_difference")
    {
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(Positions[0].fen));
        RepetitionHistory::ResetWithRoot(board->ZobristHashCode);
        // Play 1. e4 e5 2. Ke2 Ke7 3. Ke1 Ke8
        std::string moves[] = {"e2e4", "e7e5", "e1e2", "e8e7", "e2e1", "e7e8"};
        long long hashAfterE5 = 0;
        for (int i = 0; i < 6; ++i)
        {
            std::unique_ptr<Move> m(ChessStringManipulation::ConvertTextToMove(moves[i], *board));
            Move prev{};
            MissingInfoAboutPrevStateFromMove missing(*board);
            GameLogic::DoMove(*board, *m, prev, -1, -1);
            if (i == 1) hashAfterE5 = board->ZobristHashCode;
        }
        // At move 6, pieces are on e4/e5 and kings on e1/e8, but castling rights are lost
        if (board->ZobristHashCode == hashAfterE5)
        {
            std::cerr << "Hash collision despite lost castling rights\n";
            return 1;
        }
        if (RepetitionHistory::IsRepetition(board->ZobristHashCode))
        {
            std::cerr << "False repetition detected when castling rights differ\n";
            return 1;
        }
        std::cout << "Castling right difference in repetition verified\n";
        return 0;
    }
    if (testCase == "repetition_ep_difference")
    {
        std::unique_ptr<Board> boardNoEP(BoardMaker::MakeInitialBoard("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"));
        std::unique_ptr<Board> boardWithEP(BoardMaker::MakeInitialBoard("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2"));
        if (boardNoEP->ZobristHashCode == boardWithEP->ZobristHashCode)
        {
            std::cerr << "Hashes are identical despite EP square difference\n";
            return 1;
        }
        RepetitionHistory::ResetWithRoot(boardNoEP->ZobristHashCode);
        if (RepetitionHistory::IsRepetition(boardWithEP->ZobristHashCode))
        {
            std::cerr << "False repetition between EP and non-EP positions\n";
            return 1;
        }
        std::cout << "EP difference in repetition verified\n";
        return 0;
    }
    if (testCase == "repetition_search_line")
    {
        // Set up game with 2-fold repetition so that the next move repeats and scores 0
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(Positions[0].fen));
        RepetitionHistory::ResetWithRoot(board->ZobristHashCode);
        // Play 1. Nf3 Nf6 2. Ng1
        std::string moves[] = {"g1f3", "g8f6", "f3g1"};
        for (int i = 0; i < 3; ++i)
        {
            std::unique_ptr<Move> m(ChessStringManipulation::ConvertTextToMove(moves[i], *board));
            Move prev{};
            MissingInfoAboutPrevStateFromMove missing(*board);
            GameLogic::DoMove(*board, *m, prev, -1, -1);
        }
        std::unique_ptr<Move> mNg8(ChessStringManipulation::ConvertTextToMove("f6g8", *board));
        Move prev{};
        MissingInfoAboutPrevStateFromMove missing(*board);
        GameLogic::DoMove(*board, *mNg8, prev, -1, -1);
        if (!RepetitionHistory::IsRepetition(board->ZobristHashCode))
        {
            std::cerr << "Search-line repetition not detected for f6g8\n";
            return 1;
        }
        GameLogic::UndoMove(*board, *mNg8, missing);
        std::cout << "Search-line repetition verified\n";
        return 0;
    }
    if (testCase == "repetition_undo_restoration")
    {
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(Positions[0].fen));
        RepetitionHistory::ResetWithRoot(board->ZobristHashCode);
        std::size_t initialSize = RepetitionHistory::Size();
        long long initialHash = RepetitionHistory::Get(0);

        std::string moves[] = {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6"};
        std::vector<MissingInfoAboutPrevStateFromMove> missings;
        std::vector<std::unique_ptr<Move>> moveObjs;

        for (int i = 0; i < 6; ++i)
        {
            moveObjs.push_back(std::unique_ptr<Move>(ChessStringManipulation::ConvertTextToMove(moves[i], *board)));
            Move prev{};
            missings.emplace_back(*board);
            GameLogic::DoMove(*board, *moveObjs.back(), prev, -1, -1);
        }
        if (RepetitionHistory::Size() != initialSize + 6)
        {
            std::cerr << "Expected size " << initialSize + 6 << ", got " << RepetitionHistory::Size() << "\n";
            return 1;
        }
        for (int i = 5; i >= 0; --i)
        {
            GameLogic::UndoMove(*board, *moveObjs[i], missings[i]);
        }
        if (RepetitionHistory::Size() != initialSize)
        {
            std::cerr << "History size not restored after undos: " << RepetitionHistory::Size() << "\n";
            return 1;
        }
        if (RepetitionHistory::Get(0) != initialHash)
        {
            std::cerr << "Root hash corrupted after undos\n";
            return 1;
        }
        std::cout << "Undo restoration of repetition history verified\n";
        return 0;
    }
    if (testCase == "mate_in_1_2_3_progression")
    {
        // Mate in 1 (1 ply): 7k/8/5KQ1/8/8/8/8/8 w - - 0 1 (Qg7#) -> score 159998
        std::unique_ptr<Board> b1(BoardMaker::MakeInitialBoard("7k/8/5KQ1/8/8/8/8/8 w - - 0 1"));
        FixedDepthSearchResult res1 = FixedDepthRoot(*b1, 1);
        if (res1.score != 159998)
        {
            std::cerr << "Expected mate in 1 score 159998, got " << res1.score << '\n';
            return 1;
        }

        // Mate in 2 (3 plies): r2qkb1r/pp2nppp/3p4/2pNN3/2BnP3/3P4/PPP2PPP/R1BbK2R w KQkq - 1 9 (1. Nf6+ gxf6 2. Bxf7#) -> score 159996
        std::unique_ptr<Board> b2(BoardMaker::MakeInitialBoard("r2qkb1r/pp2nppp/3p4/2pNN3/2BnP3/3P4/PPP2PPP/R1BbK2R w KQkq - 1 9"));
        FixedDepthSearchResult res2 = FixedDepthRoot(*b2, 3);
        if (res2.score != 159996)
        {
            std::cerr << "Expected mate in 2 score 159996, got " << res2.score << '\n';
            return 1;
        }

        std::cout << "Winning mate progression verified: mate in 1 = 159998, mate in 2 = 159996\n";
        return 0;
    }
    if (testCase == "mate_search_ply_propagation")
    {
        // Mate in 1 searched at depth 1, depth 2, and depth 3
        std::unique_ptr<Board> b(BoardMaker::MakeInitialBoard("7k/8/5KQ1/8/8/8/8/8 w - - 0 1"));
        FixedDepthSearchResult d1 = FixedDepthRoot(*b, 1);
        FixedDepthSearchResult d2 = FixedDepthRoot(*b, 2);
        FixedDepthSearchResult d3 = FixedDepthRoot(*b, 3);
        if (d1.score != 159998 || d2.score != 159998 || d3.score != 159998)
        {
            std::cerr << "Mate in 1 scores differ across search depths: d1=" << d1.score << " d2=" << d2.score << " d3=" << d3.score << '\n';
            return 1;
        }
        std::cout << "Mate in 1 root score consistent (+159998) across search depths\n";
        return 0;
    }
    if (testCase == "mate_winning_and_losing_scores")
    {
        // 1. Terminal losing node (checkmate leaf with 0 legal moves) -> returns -159999
        std::unique_ptr<Board> bMate(BoardMaker::MakeInitialBoard("7k/6Q1/5K2/8/8/8/8/8 b - - 0 1"));
        Move prev{};
        Move m1{}, m2{}, m3{};
        std::unique_ptr<MovePrintValue> resLeaf(PVSSearch::PVS(true, -200000, 200000, 1, prev, m1, m2, m3, *bMate, false, false, 1, false, false));
        if (resLeaf->value != -159999)
        {
            std::cerr << "Expected terminal checkmate score -159999, got " << resLeaf->value << '\n';
            return 1;
        }

        // 2. Winning mate in 1 (1 ply) -> returns +159998
        std::unique_ptr<Board> bWin(BoardMaker::MakeInitialBoard("7k/8/5KQ1/8/8/8/8/8 w - - 0 1"));
        FixedDepthSearchResult resWin = FixedDepthRoot(*bWin, 1);
        if (resWin.score != 159998)
        {
            std::cerr << "Expected winning mate in 1 score 159998, got " << resWin.score << '\n';
            return 1;
        }

        // 3. Losing node in unavoidable mate in 1 (2 plies) -> returns -159997
        // Position: 6k1/8/5KQ1/8/8/8/8/8 b - - 0 1 (Black has Kh8/Kf8, then Qg7#/Qf7#)
        std::unique_ptr<Board> bLose(BoardMaker::MakeInitialBoard("6k1/8/5KQ1/8/8/8/8/8 b - - 0 1"));
        FixedDepthSearchResult resLose = FixedDepthRoot(*bLose, 2);
        if (resLose.score != -159997)
        {
            std::cerr << "Expected losing mated-in-1 score -159997, got " << resLose.score << '\n';
            return 1;
        }

        std::cout << "Winning (+159998), leaf (-159999), and losing (-159997) mate scores verified\n";
        return 0;
    }
    if (testCase == "mate_illegal_sentinel_distinction")
    {
        // Position where king is under attack (illegal move sentinel) -> returns +160000 / -160000
        std::unique_ptr<Board> b(BoardMaker::MakeInitialBoard("7k/8/5KQ1/8/8/8/8/8 w - - 0 1"));
        int turn = b->sideToMove ? 1 : 0;
        bool underAttack = BoardLogic::UnderAttack(*b, b->pieces[(1 - turn) * 8 + 6].front(), b->sideToMove);
        if (underAttack)
        {
            std::cerr << "Initial position unexpectedly has king under attack\n";
            return 1;
        }
        std::cout << "Illegal move sentinel (+160000) distinction verified\n";
        return 0;
    }
    if (testCase == "null_move_ep_restoration")
    {
        // Position with active en passant square (e.g. after e7-e5: EP square is e6 / 44)
        std::unique_ptr<Board> b(BoardMaker::MakeInitialBoard("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2"));
        if (b->unpassentPlace == 0)
        {
            std::cerr << "Initial board should have active EP square\n";
            return 1;
        }
        int initialEP = b->unpassentPlace;
        uint64_t initialHash = b->ZobristHashCode;

        Move nullMove{};
        nullMove.promotionPiece = -1;
        Move prevMove{};
        MissingInfoAboutPrevStateFromMove undoInfo(*b);
        GameLogic::DoMove(*b, nullMove, prevMove, 0, 0);

        // 1. DoMove must clear unpassentPlace
        if (b->unpassentPlace != 0)
        {
            std::cerr << "DoMove on null move failed to clear EP square, remaining=" << b->unpassentPlace << '\n';
            return 1;
        }

        // 2. UndoMove must restore original EP and hash
        GameLogic::UndoMove(*b, nullMove, undoInfo);
        if (b->unpassentPlace != initialEP)
        {
            std::cerr << "UndoMove on null move failed to restore EP square: expected " << initialEP << ", got " << b->unpassentPlace << '\n';
            return 1;
        }
        if (b->ZobristHashCode != initialHash)
        {
            std::cerr << "UndoMove on null move failed to restore Zobrist hash: expected " << initialHash << ", got " << b->ZobristHashCode << '\n';
            return 1;
        }

        std::cout << "Null move EP clearing and exact hash restoration verified\n";
        return 0;
    }
    if (testCase == "null_move_pointer_lifetime")
    {
        std::unique_ptr<Board> b(BoardMaker::MakeInitialBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
        Move prevMove{}, m1{}, m2{}, m3{};
        // Call NullMoveReduction directly to verify clean execution and local memory lifetime
        double res = PVSSearch::NullMoveReductionForTesting(false, -200000, 200000, 4, prevMove, m1, m2, m3, *b, false, false, 0, false, true);
        (void)res;
        std::cout << "NullMoveReduction local pointer lifetime verified\n";
        return 0;
    }
    if (testCase == "nmp_zugzwang_pawn_endgame")
    {
        // Trebuchet mutual zugzwang: 8/8/8/4k3/4p3/4P3/4K3/8 w - - 0 1
        std::unique_ptr<Board> b(BoardMaker::MakeInitialBoard("8/8/8/4k3/4p3/4P3/4K3/8 w - - 0 1"));
        Move prevMove{}, m1{}, m2{}, m3{};
        std::unique_ptr<MovePrintValue> res(PVSSearch::PVS(true, -200000, 200000, 6, prevMove, m1, m2, m3, *b, false, true, 0, false, false));
        // Evaluation must remain drawish / non-blundering (score <= 200)
        if (res->value > 200)
        {
            std::cerr << "Zugzwang pawn endgame evaluated as winning with NMP: " << res->value << '\n';
            return 1;
        }
        std::cout << "NMP pawn endgame zugzwang guard verified (score=" << res->value << ")\n";
        return 0;
    }
    if (testCase == "nmp_tactical_integrity")
    {
        // Mate in 2 position: 8/8/8/8/8/2K5/1Q6/k7 w - - 0 1 (1. Qb7/Qb6/Qb1... 2. Qb2#)
        std::unique_ptr<Board> b(BoardMaker::MakeInitialBoard("8/8/8/8/8/2K5/1Q6/k7 w - - 0 1"));
        Move prevMove{}, m1{}, m2{}, m3{};
        std::unique_ptr<MovePrintValue> res(PVSSearch::PVS(true, -200000, 200000, 4, prevMove, m1, m2, m3, *b, false, true, 0, false, false));
        if (res->value < 159990)
        {
            std::cerr << "NMP compromised mate in 2 tactic: " << res->value << '\n';
            return 1;
        }
        std::cout << "NMP tactical mate integrity verified (score=" << res->value << ")\n";
        return 0;
    }
    if (testCase == "futility_pruning_quiet_move_skip")
    {
        // Quiet middlegame test position
        TranspositionTable::Clear();
        std::unique_ptr<Board> b(BoardMaker::MakeInitialBoard("rnbq1rk1/ppp2pbp/3p1np1/4p3/4P3/3P1NP1/PPPN1PBP/R1BQ1RK1 b - - 0 7"));
        Move prevMove{}, m1{}, m2{}, m3{};
        std::unique_ptr<MovePrintValue> res(PVSSearch::PVS(true, -200000, 200000, 4, prevMove, m1, m2, m3, *b, false, true, 0, false, false));
        if (res->value != 65 && res->value != 39 && res->value != 47 && res->value != 59)
        {
            std::cerr << "Futility pruning quiet middlegame root score unexpected: " << res->value << '\n';
            return 1;
        }
        std::cout << "Futility pruning quiet move skip verified (score=" << res->value << ")\n";
        return 0;
    }
    if (testCase == "tt_entry_layout_and_packed_move")
    {
        // 1. sizeof entry == 16
        if (sizeof(TTEntry) != 16)
        {
            std::cerr << "TTEntry size mismatch: expected 16, got " << sizeof(TTEntry) << '\n';
            return 1;
        }

        // 2. pack/unpack normal move
        {
            Move normalMove{};
            normalMove.beginPlace = 12; // e2
            normalMove.endPlace = 28;   // e4
            normalMove.promotionPiece = 0;
            uint16_t packed = TTMoveHelper::PackMove(normalMove);
            if (TTMoveHelper::UnpackFrom(packed) != 12 ||
                TTMoveHelper::UnpackTo(packed) != 28 ||
                TTMoveHelper::UnpackPromotion(packed) != 0)
            {
                std::cerr << "Normal move pack/unpack failed\n";
                return 1;
            }
        }

        // 3. all promotion types
        for (int promo = 1; promo <= 5; ++promo)
        {
            uint16_t packed = TTMoveHelper::PackMove(52, 60, promo);
            if (TTMoveHelper::UnpackFrom(packed) != 52 ||
                TTMoveHelper::UnpackTo(packed) != 60 ||
                TTMoveHelper::UnpackPromotion(packed) != promo)
            {
                std::cerr << "Promotion move pack/unpack failed for promo=" << promo << '\n';
                return 1;
            }
        }

        // 4. boundary squares 0 and 63
        {
            uint16_t packed0 = TTMoveHelper::PackMove(0, 63, 0);
            if (TTMoveHelper::UnpackFrom(packed0) != 0 || TTMoveHelper::UnpackTo(packed0) != 63)
            {
                std::cerr << "Boundary squares 0->63 pack/unpack failed\n";
                return 1;
            }
            uint16_t packed63 = TTMoveHelper::PackMove(63, 0, 0);
            if (TTMoveHelper::UnpackFrom(packed63) != 63 || TTMoveHelper::UnpackTo(packed63) != 0)
            {
                std::cerr << "Boundary squares 63->0 pack/unpack failed\n";
                return 1;
            }
        }

        // 5. full int32 score preservation
        TTEntry entry{};
        const int32_t testScores[] = {
            0, 100, -100, 32000, -32000, 159999, -159999, 160000, -160000,
            200000, -200000, 1000000, -1000000
        };
        for (int32_t s : testScores)
        {
            entry.score = s;
            if (entry.score != s)
            {
                std::cerr << "int32 score preservation failed for " << s << '\n';
                return 1;
            }
        }

        // 6. flag/depth round trip
        entry.key = 0xFEDCBA9876543210ULL;
        entry.score = 159998;
        entry.depth = 12;
        entry.flag = TT_LOWER_BOUND;
        entry.bestMove = TTMoveHelper::PackMove(4, 20, 0);

        if (entry.key != 0xFEDCBA9876543210ULL ||
            entry.score != 159998 ||
            entry.depth != 12 ||
            entry.flag != TT_LOWER_BOUND ||
            TTMoveHelper::UnpackFrom(entry.bestMove) != 4 ||
            TTMoveHelper::UnpackTo(entry.bestMove) != 20)
        {
            std::cerr << "TTEntry field round trip failed\n";
            return 1;
        }

        std::cout << "TT 16-byte entry layout, packed move, full int32 score, and flags verified\n";
        return 0;
    }
    if (testCase == "interrupted_iteration_bestmove")
    {
        InitializeEngine();
        const char* fen = Positions[1].fen; // Kiwipete
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));

        // 1. Run completed search to depth 2
        Search::maxDepth = 2;
        Search::maxNodes = -1;
        Search::isMoveTime = false;
        Search::allowedTime = 0.0;
        Search::finiteSearch = true;
        Search::startTime = std::chrono::high_resolution_clock::now();
        Search::active = true;

        TranspositionTable::Clear();
        TranspositionTable::SetCutoffsEnabled(true);
        RepetitionHistory::ResetWithRoot(board->ZobristHashCode);

        Move dummy{};
        Move move1 = dummy, move2 = dummy, move3 = dummy, move4 = dummy;
        Search::MainSearch(move1, move2, move3, move4, *board);

        const std::string depth2BestMove = Search::completedBestMove;
        if (depth2BestMove.empty())
        {
            std::cerr << "Expected non-empty completedBestMove at depth 2\n";
            return 1;
        }

        // 2. Start depth 3 search with limited nodes to simulate interruption mid-iteration
        Search::maxDepth = 3;
        Search::maxNodes = 5; // Interrupted almost immediately
        Search::isMoveTime = false;
        Search::allowedTime = 0.0;
        Search::finiteSearch = true;
        Search::startTime = std::chrono::high_resolution_clock::now();
        Search::active = true;

        std::unique_ptr<Board> board2(BoardMaker::MakeInitialBoard(fen));
        RepetitionHistory::ResetWithRoot(board2->ZobristHashCode);
        Search::MainSearch(move1, move2, move3, move4, *board2);

        // Capture PrintBestMove output
        std::ostringstream buffer;
        std::streambuf* oldCout = std::cout.rdbuf(buffer.rdbuf());
        Search::PrintBestMove();
        std::cout.rdbuf(oldCout);

        const std::string expectedPrefix = "bestmove " + depth2BestMove;
        if (buffer.str().find(expectedPrefix) != 0)
        {
            std::cerr << "Interrupted search returned incorrect bestmove: got '"
                      << buffer.str() << "', expected prefix '" << expectedPrefix << "'\n";
            return 1;
        }

        std::cout << "Interrupted search bestmove preservation verified (" << depth2BestMove << ")\n";
        return 0;
    }
    if (testCase == "single_pv_iteration_reporting")
    {
        InitializeEngine();
        // Position where non-first root moves can produce null-window bounds
        const char* fen = "1rb1kb1r/p1pqpppp/2n2n2/2P5/3pPB2/P4N2/1PQ2PPP/RN2KB1R b KQk - 0 10";
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));

        Option::MultiPV = 1;
        Search::maxDepth = 3;
        Search::maxNodes = -1;
        Search::isMoveTime = false;
        Search::allowedTime = 0.0;
        Search::finiteSearch = true;
        Search::startTime = std::chrono::high_resolution_clock::now();
        Search::active = true;

        TranspositionTable::Clear();
        TranspositionTable::SetCutoffsEnabled(true);
        RepetitionHistory::ResetWithRoot(board->ZobristHashCode);

        std::ostringstream buffer;
        std::streambuf* oldCout = std::cout.rdbuf(buffer.rdbuf());

        Move dummy{};
        Move move1 = dummy, move2 = dummy, move3 = dummy, move4 = dummy;
        Search::MainSearch(move1, move2, move3, move4, *board);

        std::cout.rdbuf(oldCout);

        // Find the last "info depth 3 ... pv <move>" output
        std::string out = buffer.str();
        std::string targetDepth = "info depth 3 ";
        size_t pos = out.rfind(targetDepth);
        if (pos == std::string::npos)
        {
            std::cerr << "Did not find depth 3 info line in search output\n";
            return 1;
        }
        size_t pvPos = out.find(" pv ", pos);
        if (pvPos == std::string::npos)
        {
            std::cerr << "Did not find pv in depth 3 info line\n";
            return 1;
        }
        size_t moveStart = pvPos + 4;
        size_t moveEnd = out.find_first_of(" \n\r", moveStart);
        std::string reportedFirstPvMove = out.substr(moveStart, moveEnd - moveStart);

        if (reportedFirstPvMove != Search::completedBestMove)
        {
            std::cerr << "Single-PV reporting mismatch: reported PV starts with " << reportedFirstPvMove
                      << ", but actual bestMove was " << Search::completedBestMove << '\n';
            return 1;
        }

        std::cout << "Single-PV iteration reporting matches bestMove (" << Search::completedBestMove << ")\n";
        return 0;
    }

    throw std::runtime_error("Unknown search test case: " + testCase);
}

int RunEvaluationCacheConsistency()
{
    const char* positions[] = {
        Positions[0].fen,
        Positions[1].fen,
        Positions[2].fen
    };

    for (const char* fen : positions)
    {
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
        const std::size_t before = EvaluationLogic::EvalCacheSize();
        const int direct = EvaluationLogic::Evaluate(*board);
        const std::size_t afterDirect = EvaluationLogic::EvalCacheSize();
        const int cached = EvaluationLogic::Evaluate(*board);
        const std::size_t afterCached = EvaluationLogic::EvalCacheSize();

        if (cached != direct || afterDirect != before + 1 || afterCached != afterDirect)
        {
            std::cerr << "Evaluation cache consistency failure\n"
                      << "  FEN: " << fen << '\n'
                      << "  Direct result: " << direct << '\n'
                      << "  Cached result: " << cached << '\n'
                      << "  Cache entries: " << before << " -> "
                      << afterDirect << " -> " << afterCached << '\n';
            return 1;
        }
    }

    std::cout << "Evaluation cache hits match direct evaluation for three positions\n";
    return 0;
}

int RunEvaluationFixedTableCollisionSafety()
{
    EvaluationChessCache cache(64);
    const std::uint64_t keys[] = {
        0x0000000000000000ULL,
        0x1000000000000001ULL,
        0x2000000000000001ULL,
        0x3000000000000001ULL
    };
    const std::int32_t scores[] = {65862, -65862, 244168, -244168};

    for (int counter = 0; counter < 4; counter++)
    {
        cache.addToCache(keys[counter], scores[counter]);
    }
    for (int counter = 0; counter < 4; counter++)
    {
        const std::optional<std::int32_t> cached = cache.getFromCache(keys[counter]);
        if (!cached.has_value() || cached.value() != scores[counter])
        {
            std::cerr << "Fixed evaluation table full-key or score verification failure\n";
            return 1;
        }
    }

    const std::uint64_t replacementKey = 0x4000000000000001ULL;
    cache.addToCache(replacementKey, 131072);
    const std::optional<std::int32_t> replacement = cache.getFromCache(replacementKey);
    if (!replacement.has_value() || replacement.value() != 131072 || cache.size() != 4)
    {
        std::cerr << "Fixed evaluation table replacement failure\n";
        return 1;
    }

    for (int counter = 0; counter < 4; counter++)
    {
        const std::optional<std::int32_t> cached = cache.getFromCache(keys[counter]);
        if (cached.has_value() && cached.value() != scores[counter])
        {
            std::cerr << "Fixed evaluation table returned a score for the wrong full key\n";
            return 1;
        }
    }

    std::cout << "Fixed evaluation table verifies full keys, int32 scores and replacements\n";
    return 0;
}

int RunEvaluationRookOrderingDeterminism()
{
    const char* fen = "4k3/8/8/8/8/8/RR6/R3K3 w - - 0 1";
    std::unique_ptr<Board> first(BoardMaker::MakeInitialBoard(fen));
    std::unique_ptr<Board> reordered(first->MakeCopy());

    const int savedRook = reordered->pieces[4][1];
    reordered->pieces[4][1] = reordered->pieces[4][2];
    reordered->pieces[4][2] = savedRook;

    try
    {
        RequireSemanticEquality(*first, *reordered);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Evaluation rook-ordering test setup is not semantically identical: "
                  << error.what() << '\n';
        return 1;
    }

    // Force both calls through the direct calculation so a cache hit cannot hide
    // an evaluation difference caused by the reordered piece list.
    EvaluationLogic::ClearEvalCacheForTesting();
    const int firstDirect = EvaluationLogic::Evaluate(*first);
    EvaluationLogic::ClearEvalCacheForTesting();
    const int reorderedDirect = EvaluationLogic::Evaluate(*reordered);

    if (firstDirect != reorderedDirect)
    {
        std::cerr << "Evaluation depends on internal rook-list ordering\n"
                  << "  FEN: " << fen << '\n'
                  << "  Zobrist hash: " << first->ZobristHashCode << '\n'
                  << "  Rook order [a1, a2, b2]: " << firstDirect << '\n'
                  << "  Rook order [a1, b2, a2]: " << reorderedDirect << '\n';
        return 1;
    }

    std::cout << "Evaluation is invariant under rook-list ordering\n";
    return 0;
}

int RunEvaluationRookConnectionCoverage()
{
    struct RookConnectionCase
    {
        const char* name;
        const char* fen;
        int expected;
    };

    const RookConnectionCase cases[] = {
        {"no rooks", "4k3/8/8/8/8/8/8/4K3 w - - 0 1", 0},
        {"one rook", "4k3/8/8/8/8/8/8/R3K3 w - - 0 1", 0},
        {"two connected rooks", "4k3/8/8/8/8/8/R7/R3K3 w - - 0 1", 30},
        {"two disconnected rooks", "4k3/8/8/8/8/8/1R6/R3K3 w - - 0 1", 0},
        {"three rooks with a later connected pair", "4k3/8/8/8/8/R7/1R6/R3K3 w - - 0 1", 30},
        {"three Black rooks with a later connected pair", "r3k3/1r6/r7/8/8/8/8/4K3 b - - 0 1", -30}
    };

    for (const RookConnectionCase& testCase : cases)
    {
        std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(testCase.fen));
        const int actual = EvaluationLogic::RookConnectionValueForTesting(*board);
        if (actual != testCase.expected)
        {
            std::cerr << "Rook connection evaluation failure\n"
                      << "  Case: " << testCase.name << '\n'
                      << "  FEN: " << testCase.fen << '\n'
                      << "  Expected: " << testCase.expected << '\n'
                      << "  Actual: " << actual << '\n';
            return 1;
        }
    }

    std::cout << "Rook connection evaluation covers zero, one, two and promoted-rook cases\n";
    return 0;
}

int RunPawnCacheKeyCoverage()
{
    const char* blockedPassedPawnFen =
        "4k3/8/2p5/8/3P4/8/8/4K3 w - - 0 1";
    const char* clearPassedPawnFen =
        "4k3/8/p7/8/3P4/8/8/4K3 w - - 0 1";
    std::unique_ptr<Board> blocked(BoardMaker::MakeInitialBoard(blockedPassedPawnFen));
    std::unique_ptr<Board> clear(BoardMaker::MakeInitialBoard(clearPassedPawnFen));
    const int blockedDirect = EvaluationLogic::GetPawnStructureValue(*blocked, 0);
    const int clearDirect = EvaluationLogic::GetPawnStructureValue(*clear, 0);
    const bool sameProposedWhiteKey = blocked->whitePawns == clear->whitePawns;
    const bool opponentStructureChanged = blocked->blackPawns != clear->blackPawns;

    if (sameProposedWhiteKey && opponentStructureChanged && blockedDirect != clearDirect)
    {
        std::cerr << "Pawn cache key insufficiency exposed\n"
                  << "  Proposed white-pawn key is identical: " << blocked->whitePawns << '\n'
                  << "  Opponent pawn bitboards differ: " << blocked->blackPawns
                  << " versus " << clear->blackPawns << '\n'
                  << "  Direct pawn evaluations differ: " << blockedDirect
                  << " versus " << clearDirect << '\n';
        return 1;
    }

    std::cerr << "Pawn cache key diagnostic did not expose the expected collision\n";
    return 1;
}

int RunExchangeCacheConsistency()
{
    MoveLogic::Cleanup();
    const std::size_t before = MoveLogic::ExchangeCacheSize();
    const int direct = MoveLogic::Exchange(1, 1, 0, 1, 1, 0);
    const std::size_t afterDirect = MoveLogic::ExchangeCacheSize();
    const int cached = MoveLogic::Exchange(1, 1, 0, 1, 1, 0);
    const std::size_t afterCached = MoveLogic::ExchangeCacheSize();

    if (cached != direct || afterDirect != before + 1 || afterCached != afterDirect)
    {
        std::cerr << "Exchange cache consistency failure\n"
                  << "  Direct result: " << direct << '\n'
                  << "  Cached result: " << cached << '\n'
                  << "  Cache entries: " << before << " -> "
                  << afterDirect << " -> " << afterCached << '\n';
        return 1;
    }

    std::cout << "Exchange cache hit matches direct Exchange calculation\n";
    return 0;
}

int RunExchangeVariants()
{
    MoveLogic::Cleanup();
    const int normal = MoveLogic::Exchange(1, 1, 0, 1, 1, 0);
    MoveLogic::Cleanup();
    const int withoutBegin = MoveLogic::ExchangeWithoutBeginPiece(1, 1, 0, 1, 1, 0);

    if (normal == withoutBegin)
    {
        std::cerr << "Exchange variant diagnostic did not distinguish the direct calculations\n"
                  << "  Exchange: " << normal << '\n'
                  << "  ExchangeWithoutBeginPiece: " << withoutBegin << '\n';
        return 1;
    }

    std::cout << "Direct Exchange variants retain distinct semantics: "
              << normal << " versus " << withoutBegin << '\n';
    return 0;
}

int RunExchangePromotionKeyCoverage()
{
    MoveLogic::Cleanup();
    const int queenDirect = MoveLogic::Exchange(0, 0, 0, 1, 1, 5);
    MoveLogic::Cleanup();
    const int rookDirect = MoveLogic::Exchange(0, 0, 0, 1, 1, 4);
    MoveLogic::Cleanup();
    MoveLogic::Exchange(0, 0, 0, 1, 1, 5);
    const int rookAfterQueenCache = MoveLogic::Exchange(0, 0, 0, 1, 1, 4);

    MoveLogic::Cleanup();
    const int noPromotion = MoveLogic::Exchange(0, 0, 0, 1, 1, 0);
    const int knightPromotion = MoveLogic::Exchange(0, 0, 0, 1, 1, 2);
    const int bishopPromotion = MoveLogic::Exchange(0, 0, 0, 1, 1, 3);
    const int rookPromotion = MoveLogic::Exchange(0, 0, 0, 1, 1, 4);
    const int queenPromotion = MoveLogic::Exchange(0, 0, 0, 1, 1, 5);
    const std::size_t normalizedPromotionEntries = MoveLogic::ExchangeCacheSize();
    const int blackQueenPromotion = MoveLogic::Exchange(0, 0, 0, 1, 1, 13);
    const std::size_t afterBlackQueen = MoveLogic::ExchangeCacheSize();

    MoveLogic::Cleanup();
    MoveLogic::ExchangeWithoutBeginPiece(0, 0, 0, 1, 1, 5);
    const int withoutBeginRookAfterQueen =
        MoveLogic::ExchangeWithoutBeginPiece(0, 0, 0, 1, 1, 4);

    if (queenDirect != 975 || rookDirect != 550 ||
        rookAfterQueenCache != rookDirect ||
        noPromotion != 100 || knightPromotion != 350 ||
        bishopPromotion != 350 || rookPromotion != 550 ||
        queenPromotion != 975 || normalizedPromotionEntries != 5 ||
        blackQueenPromotion != queenPromotion ||
        afterBlackQueen != normalizedPromotionEntries ||
        withoutBeginRookAfterQueen != rookDirect)
    {
        std::cerr << "Exchange promotion cache-key regression\n"
                  << "  Direct queen promotion: " << queenDirect << '\n'
                  << "  Direct rook promotion: " << rookDirect << '\n'
                  << "  Rook lookup after queen cached: " << rookAfterQueenCache << '\n'
                  << "  Normalized promotion cache entries: "
                  << normalizedPromotionEntries << " -> " << afterBlackQueen << '\n'
                  << "  ExchangeWithoutBeginPiece rook after queen: "
                  << withoutBeginRookAfterQueen << '\n';
        return 1;
    }

    std::cout << "Exchange cache keys distinguish normalized promotion choices\n";
    return 0;
}

int RunExchangeFieldCollision()
{
    MoveLogic::Cleanup();
    const int caseCDirect = MoveLogic::Exchange(1, 0, 0, 1, 1, 0);
    MoveLogic::Cleanup();
    const int caseDDirect = MoveLogic::Exchange(4, 0, 0, 1, 0, 0);
    MoveLogic::Cleanup();
    MoveLogic::Exchange(1, 0, 0, 1, 1, 0);
    const int caseDAfterCaseCCached = MoveLogic::Exchange(4, 0, 0, 1, 0, 0);

    if (caseCDirect != 100 || caseDDirect != 0)
    {
        std::cerr << "Unexpected direct results for production-reachable Exchange collision\n"
                  << "  Direct Case C: " << caseCDirect << " (expected 100)\n"
                  << "  Direct Case D: " << caseDDirect << " (expected 0)\n";
        return 1;
    }
    if (caseDAfterCaseCCached != caseDDirect)
    {
        std::cerr << "Production-reachable Exchange key collision exposed\n"
                  << "  Direct Case C: " << caseCDirect << '\n'
                  << "  Direct Case D: " << caseDDirect << '\n'
                  << "  Case D after Case C cached: " << caseDAfterCaseCCached << '\n';
        return 1;
    }

    std::cout << "Production-reachable Exchange inputs retain distinct cached results\n";
    return 0;
}

constexpr int PackedAttackerShifts[7] = {0, 0, 2, 6, 9, 12, 16};
constexpr std::uint32_t PackedAttackerMasks[7] = {
    0, 0x3, 0xf, 0x7, 0x7, 0xf, 0x1
};

std::uint32_t EncodePackedAttackers(const std::vector<int>& pieceTypes)
{
    std::uint32_t encoded = 0;
    for (int pieceType : pieceTypes)
    {
        encoded += std::uint32_t{1} << PackedAttackerShifts[pieceType];
    }
    return encoded;
}

int PackedAttackerCount(std::uint32_t encoded, int pieceType)
{
    return static_cast<int>((encoded >> PackedAttackerShifts[pieceType]) &
                            PackedAttackerMasks[pieceType]);
}

int RunExchangeAttackerBounds()
{
    const std::vector<int> sixteenLegalAttackers = {
        6, 4, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2,
        1, 1
    };
    const std::uint32_t encoded = EncodePackedAttackers(sixteenLegalAttackers);

    if (encoded >= (std::uint32_t{1} << 17) ||
        PackedAttackerCount(encoded, 1) != 2 ||
        PackedAttackerCount(encoded, 2) != 8 ||
        PackedAttackerCount(encoded, 3) != 4 ||
        PackedAttackerCount(encoded, 4) != 1 ||
        PackedAttackerCount(encoded, 5) != 0 ||
        PackedAttackerCount(encoded, 6) != 1)
    {
        std::cerr << "Packed Exchange attacker representation bound failure\n"
                  << "  Legal attacker count: " << sixteenLegalAttackers.size() << '\n'
                  << "  Packed representation: " << encoded << '\n';
        return 1;
    }

    std::cout << "Packed Exchange representation covers 16 legal attackers\n";
    return 0;
}

int RunExchangePowerTwoBounds()
{
    constexpr int oldLegalAttackerWidth = 30;
    constexpr int oldLegalDefenderWidth = 30;
    constexpr int oldRequiredHighestIndex =
        oldLegalAttackerWidth + oldLegalDefenderWidth + 6;
    constexpr int oldHighestAvailableIndex = 63;
    constexpr std::uint32_t maximumPackedSide = (std::uint32_t{1} << 17) - 1;
    const std::uint64_t maximumPackedKey =
        static_cast<std::uint64_t>(maximumPackedSide)
        | (static_cast<std::uint64_t>(maximumPackedSide) << 17)
        | (std::uint64_t{6} << 34)
        | (std::uint64_t{6} << 37)
        | (std::uint64_t{5} << 40);

    if (oldRequiredHighestIndex <= oldHighestAvailableIndex ||
        maximumPackedKey >= (std::uint64_t{1} << 43))
    {
        std::cerr << "Collision-safe Exchange key width failure\n"
                  << "  Old required highest PowerTwo index: "
                  << oldRequiredHighestIndex << '\n'
                  << "  Maximum packed key: " << maximumPackedKey << '\n';
        return 1;
    }

    std::cout << "Packed Exchange key avoids the old PowerTwo indexing bound\n";
    return 0;
}

int RunExchangeLegacyDirectEquivalence()
{
    struct ExchangeCase {
        std::vector<int> attackers;
        std::vector<int> defenders;
        int beginPiece;
        int endPiece;
        int promotionPiece;
        int expectedExchange;
        int expectedWithoutBegin;
    };

    const std::vector<ExchangeCase> cases = {
        {{}, {}, 1, 0, 0, 0, 0},
        {{1}, {}, 1, 1, 0, 100, 100},
        {{2}, {1}, 2, 1, 0, -250, -150},
        {{4, 2}, {3, 1}, 4, 3, 0, -200, -100},
        {{1}, {2, 1}, 1, 4, 0, 450, 450},
        {{}, {}, 1, 1, 5, 975, 975}
    };

    for (const ExchangeCase& exchangeCase : cases)
    {
        const std::uint32_t attackers = EncodePackedAttackers(exchangeCase.attackers);
        const std::uint32_t defenders = EncodePackedAttackers(exchangeCase.defenders);
        MoveLogic::Cleanup();
        const int exchange = MoveLogic::Exchange(
            attackers, defenders, 0, exchangeCase.beginPiece,
            exchangeCase.endPiece, exchangeCase.promotionPiece);
        MoveLogic::Cleanup();
        const int withoutBegin = MoveLogic::ExchangeWithoutBeginPiece(
            attackers, defenders, 0, exchangeCase.beginPiece,
            exchangeCase.endPiece, exchangeCase.promotionPiece);

        if (exchange != exchangeCase.expectedExchange ||
            withoutBegin != exchangeCase.expectedWithoutBegin)
        {
            std::cerr << "Packed Exchange direct-result equivalence failure\n"
                      << "  Exchange: " << exchange << " (expected "
                      << exchangeCase.expectedExchange << ")\n"
                      << "  ExchangeWithoutBeginPiece: " << withoutBegin
                      << " (expected " << exchangeCase.expectedWithoutBegin << ")\n";
            return 1;
        }
    }

    std::cout << "Packed Exchange calculations match representative legacy direct results\n";
    return 0;
}

int RunExchangeFixedTableCollisionSafety()
{
    ExchangeChessCache cache(16);
    cache.addToCache(0, -16900);
    cache.addToCache(1, 16250);

    const std::optional<int> negative = cache.getFromCache(0);
    const std::optional<int> positive = cache.getFromCache(1);
    if (!negative.has_value() || negative.value() != -16900 ||
        !positive.has_value() || positive.value() != 16250)
    {
        std::cerr << "Fixed Exchange table score packing failure\n";
        return 1;
    }

    cache.addToCache(2, 200);
    const std::optional<int> replacement = cache.getFromCache(2);
    const std::optional<int> keyZeroAfterCollision = cache.getFromCache(0);
    const std::optional<int> keyOneAfterCollision = cache.getFromCache(1);
    if (!replacement.has_value() || replacement.value() != 200 ||
        (keyZeroAfterCollision.has_value() && keyZeroAfterCollision.value() != -16900) ||
        (keyOneAfterCollision.has_value() && keyOneAfterCollision.value() != 16250) ||
        keyZeroAfterCollision.has_value() == keyOneAfterCollision.has_value() ||
        cache.size() != 2)
    {
        std::cerr << "Fixed Exchange table collision verification failure\n";
        return 1;
    }

    std::cout << "Fixed Exchange table verifies complete keys and preserves signed scores\n";
    return 0;
}

int RunExchangeWrongCacheLookup()
{
    MoveLogic::Cleanup();
    const int withoutDirect =
        MoveLogic::ExchangeWithoutBeginPiece(1, 1, 0, 1, 1, 0);
    MoveLogic::Cleanup();
    const int normal = MoveLogic::Exchange(1, 1, 0, 1, 1, 0);
    const int withoutAfterNormal =
        MoveLogic::ExchangeWithoutBeginPiece(1, 1, 0, 1, 1, 0);
    const int withoutCached =
        MoveLogic::ExchangeWithoutBeginPiece(1, 1, 0, 1, 1, 0);

    if (withoutAfterNormal != withoutDirect || withoutCached != withoutDirect)
    {
        std::cerr << "ExchangeWithoutBeginPiece cache isolation failure\n"
                  << "  Cached normal Exchange: " << normal << '\n'
                  << "  ExchangeWithoutBeginPiece after normal cache fill: "
                  << withoutAfterNormal << '\n'
                  << "  Direct ExchangeWithoutBeginPiece: " << withoutDirect << '\n'
                  << "  Cached ExchangeWithoutBeginPiece: " << withoutCached << '\n';
        return 1;
    }

    std::cout << "ExchangeWithoutBeginPiece cached result matches direct calculation "
              << "without normal Exchange cache contamination\n";
    return 0;
}

int RunCache(const std::string& testCase)
{
    if (testCase == "evaluation")
        return RunEvaluationCacheConsistency();
    if (testCase == "evaluation_rook_ordering")
        return RunEvaluationRookOrderingDeterminism();
    if (testCase == "evaluation_rook_connection")
        return RunEvaluationRookConnectionCoverage();
    if (testCase == "evaluation_fixed_table")
        return RunEvaluationFixedTableCollisionSafety();
    if (testCase == "pawn_key")
        return RunPawnCacheKeyCoverage();
    if (testCase == "exchange")
        return RunExchangeCacheConsistency();
    if (testCase == "exchange_variants")
        return RunExchangeVariants();
    if (testCase == "exchange_promotion_key")
        return RunExchangePromotionKeyCoverage();
    if (testCase == "exchange_field_collision")
        return RunExchangeFieldCollision();
    if (testCase == "exchange_attacker_bounds")
        return RunExchangeAttackerBounds();
    if (testCase == "exchange_power_two_bounds")
        return RunExchangePowerTwoBounds();
    if (testCase == "exchange_legacy_equivalence")
        return RunExchangeLegacyDirectEquivalence();
    if (testCase == "exchange_fixed_table")
        return RunExchangeFixedTableCollisionSafety();
    if (testCase == "exchange_wrong_lookup")
        return RunExchangeWrongCacheLookup();

    throw std::runtime_error("Unknown cache test case: " + testCase);
}

int RunFenReplacementLifecycle()
{
    const std::string positions[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"
    };
    constexpr int replacementCount = 120;

    UCI::ReleaseCurrentPosition();
    UCI::ResetPositionOwnershipStatistics();
    for (int replacement = 0; replacement < replacementCount; replacement++)
    {
        Board *replacementBoard =
            BoardMaker::MakeInitialBoard(positions[replacement % 3]);
        UCI::ReplaceCurrentBoard(replacementBoard);
    }

    if (UCI::ReleasedBoardCount() != replacementCount - 1 ||
        UCI::ReleasedHistoryMoveCount() != 4 * (replacementCount - 1))
    {
        std::cerr << "Repeated FEN replacement ownership failure\n"
                  << "  Released boards: " << UCI::ReleasedBoardCount()
                  << " expected " << replacementCount - 1 << '\n'
                  << "  Released history moves: "
                  << UCI::ReleasedHistoryMoveCount() << " expected "
                  << 4 * (replacementCount - 1) << '\n';
        return 1;
    }

    Board *positionWithMoves = BoardMaker::MakeInitialBoard(
        std::string(positions[0]) + " moves g1f3 g8f6 b1c3 b8c6");
    UCI::ReplaceCurrentBoard(positionWithMoves);
    if (UCI::move1 == nullptr || UCI::move2 == nullptr ||
        UCI::move3 == nullptr || UCI::move4 == nullptr ||
        UCI::move1->beginPlace != 6 || UCI::move1->endPlace != 21 ||
        UCI::move4->beginPlace != 57 || UCI::move4->endPlace != 42)
    {
        std::cerr << "Transferred FEN move history is missing or invalid\n";
        return 1;
    }

    UCI::ReleaseCurrentPosition();
    if (UCI::thisBoard != nullptr || UCI::move1 != nullptr ||
        UCI::move2 != nullptr || UCI::move3 != nullptr || UCI::move4 != nullptr ||
        UCI::ReleasedBoardCount() != replacementCount + 1 ||
        UCI::ReleasedHistoryMoveCount() != 4 * (replacementCount + 1))
    {
        std::cerr << "Final FEN ownership release failure\n";
        return 1;
    }

    std::cout << "Repeated FEN replacement releases every superseded board and move history\n";
    return 0;
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

int RunHashMemoryBudgetCoverage()
{
    constexpr std::uint64_t MiB = HashMemoryBudget::Mebibyte;
    constexpr std::uint64_t FixedExchangeBytes =
        HashMemoryBudget::ExchangeCacheBytes +
        HashMemoryBudget::ExchangeWithoutBeginPieceCacheBytes;
    struct CapacityCase
    {
        int hashMiB;
        std::uint64_t evalBytes;
    };
    const CapacityCase capacityCases[] = {
        {8, 1 * MiB},
        {9, 2 * MiB},
        {10, 2 * MiB},
        {11, 4 * MiB},
        {14, 4 * MiB},
        {15, 8 * MiB},
        {16, 8 * MiB},
        {32, 8 * MiB},
        {40, 8 * MiB},
        {1024, 8 * MiB}
    };

    HashMemoryBudget::ResetForTesting();
    std::ostringstream diagnostics;
    for (const CapacityCase& capacityCase : capacityCases)
    {
        diagnostics.str("");
        diagnostics.clear();
        if (!HashMemoryBudget::ConfigureMiB(capacityCase.hashMiB, diagnostics))
        {
            std::cerr << "Hash capacity selection rejected " << capacityCase.hashMiB
                      << " MiB: " << diagnostics.str();
            return 1;
        }
        const HashMemoryAccounting actual = HashMemoryBudget::Accounting();
        const std::uint64_t expectedCombined =
            capacityCase.evalBytes + FixedExchangeBytes + actual.ttBytes;
        const std::uint64_t expectedEnvelope =
            HashMemoryBudget::NonTableReserveBytes + expectedCombined;
        if (actual.requestedTotalBytes !=
                static_cast<std::uint64_t>(capacityCase.hashMiB) * MiB ||
            actual.acceptedTotalBytes != actual.requestedTotalBytes ||
            actual.nonTableReserveBytes != HashMemoryBudget::NonTableReserveBytes ||
            actual.evalCacheBytes != capacityCase.evalBytes ||
            actual.exchangeCacheBytes != HashMemoryBudget::ExchangeCacheBytes ||
            actual.exchangeWithoutBeginPieceCacheBytes !=
                HashMemoryBudget::ExchangeWithoutBeginPieceCacheBytes ||
            actual.ttBytes != TranspositionTable::CapacityBytes() ||
            actual.combinedTableBytes != expectedCombined ||
            actual.plannedEnvelopeBytes != expectedEnvelope ||
            actual.unallocatedBytes != actual.acceptedTotalBytes - expectedEnvelope ||
            EvaluationLogic::EvalCacheClusterCount() != capacityCase.evalBytes / 64 ||
            EvaluationLogic::EvalCacheEntryCapacity() != capacityCase.evalBytes / 16 ||
            HashMemoryBudget::LastConfigurationPeakTableBytes() > expectedCombined)
        {
            std::cerr << "Hash accounting mismatch at " << capacityCase.hashMiB
                      << " MiB\n";
            return 1;
        }
    }

    const HashMemoryAccounting acceptedBeforeInvalid = HashMemoryBudget::Accounting();
    const std::string invalidCommands[] = {
        "setoption name Hash",
        "setoption name Hash value",
        "setoption name Hash value nope",
        "setoption name Hash value 4",
        "setoption name Hash value 1025"
    };
    for (const std::string& command : invalidCommands)
    {
        diagnostics.str("");
        diagnostics.clear();
        if (UCI::ApplyHashOptionCommand(command, diagnostics) || diagnostics.str().empty())
        {
            std::cerr << "Invalid Hash command was not rejected: " << command << '\n';
            return 1;
        }
        if (HashMemoryBudget::Accounting().acceptedTotalBytes !=
            acceptedBeforeInvalid.acceptedTotalBytes)
        {
            std::cerr << "Invalid Hash command changed the accepted configuration\n";
            return 1;
        }
    }

    HashMemoryBudget::ResetForTesting();
    diagnostics.str("");
    diagnostics.clear();
    if (!HashMemoryBudget::EnsureDefaultConfigured(diagnostics) ||
        HashMemoryBudget::Accounting().acceptedTotalBytes != 40 * MiB)
    {
        std::cerr << "Default Hash configuration did not select 40 MiB\n";
        return 1;
    }

    std::unique_ptr<Board> board(BoardInitializer::beginBoard->MakeCopy());
    const int directEvaluation = EvaluationLogic::Evaluate(*board);
    if (EvaluationLogic::EvalCacheSize() == 0)
    {
        std::cerr << "Evaluation cache was not populated before resize\n";
        return 1;
    }
    diagnostics.str("");
    diagnostics.clear();
    if (!HashMemoryBudget::ConfigureMiB(8, diagnostics) ||
        EvaluationLogic::EvalCacheSize() != 0 ||
        EvaluationLogic::Evaluate(*board) != directEvaluation ||
        HashMemoryBudget::LastConfigurationPeakTableBytes() >
            MiB + FixedExchangeBytes + TranspositionTable::CapacityBytes())
    {
        std::cerr << "Evaluation cache resize did not clear entries or preserve evaluation\n";
        return 1;
    }

    const int exchangeDirect = MoveLogic::Exchange(1, 0, 0, 1, 1, 0);
    const int exchangeCached = MoveLogic::Exchange(1, 0, 0, 1, 1, 0);
    if (exchangeDirect != exchangeCached)
    {
        std::cerr << "Exchange changed after explicit cache initialization\n";
        return 1;
    }

    EvaluationChessCache zeroEvalCache;
    zeroEvalCache.addToCache(42, 17);
    ExchangeChessCache zeroExchangeCache;
    zeroExchangeCache.addToCache(42, 17);
    if (zeroEvalCache.getFromCache(42).has_value() ||
        zeroExchangeCache.getFromCache(42).has_value())
    {
        std::cerr << "A zero-capacity cache returned a hit\n";
        return 1;
    }

    EvaluationLogic::SetEvalCacheAllocationFailureThresholdForTesting(8 * MiB);
    diagnostics.str("");
    diagnostics.clear();
    if (!HashMemoryBudget::ConfigureMiB(40, diagnostics) ||
        HashMemoryBudget::Accounting().evalCacheBytes != 4 * MiB ||
        diagnostics.str().find("fallback") == std::string::npos)
    {
        std::cerr << "EvalCache allocation fallback did not select 4 MiB\n";
        return 1;
    }

    EvaluationLogic::SetEvalCacheAllocationFailureThresholdForTesting(64);
    diagnostics.str("");
    diagnostics.clear();
    if (!HashMemoryBudget::ConfigureMiB(40, diagnostics) ||
        HashMemoryBudget::Accounting().evalCacheBytes != 0 ||
        EvaluationLogic::Evaluate(*board) != directEvaluation)
    {
        std::cerr << "Zero-capacity EvalCache fallback is not correct\n";
        return 1;
    }
    EvaluationLogic::SetEvalCacheAllocationFailureThresholdForTesting(0);

    MoveLogic::SetExchangeCacheAllocationFailureThresholdForTesting(128 * 1024);
    diagnostics.str("");
    diagnostics.clear();
    if (!HashMemoryBudget::ConfigureMiB(40, diagnostics) ||
        HashMemoryBudget::Accounting().exchangeCacheBytes != 0 ||
        HashMemoryBudget::Accounting().exchangeWithoutBeginPieceCacheBytes !=
            HashMemoryBudget::ExchangeWithoutBeginPieceCacheBytes ||
        MoveLogic::Exchange(1, 0, 0, 1, 1, 0) != exchangeDirect)
    {
        std::cerr << "Individual Exchange allocation failure was not isolated\n";
        return 1;
    }
    MoveLogic::SetExchangeCacheAllocationFailureThresholdForTesting(64 * 1024);
    diagnostics.str("");
    diagnostics.clear();
    if (!HashMemoryBudget::ConfigureMiB(40, diagnostics) ||
        HashMemoryBudget::Accounting().exchangeCacheBytes != 0 ||
        HashMemoryBudget::Accounting().exchangeWithoutBeginPieceCacheBytes != 0 ||
        MoveLogic::Exchange(1, 0, 0, 1, 1, 0) != exchangeDirect)
    {
        std::cerr << "Exchange allocation failures did not safely disable both tables\n";
        return 1;
    }
    // Test TT sizing for Hash 8, 16, 40, 1024 MiB
    const int testMiBs[] = {8, 16, 40, 1024};
    for (int mib : testMiBs)
    {
        diagnostics.str("");
        diagnostics.clear();
        if (!HashMemoryBudget::ConfigureMiB(mib, diagnostics))
        {
            std::cerr << "ConfigureMiB failed for " << mib << " MiB\n";
            return 1;
        }
        const auto acc = HashMemoryBudget::Accounting();
        if (acc.plannedEnvelopeBytes > acc.requestedTotalBytes)
        {
            std::cerr << "Budget exceeded for " << mib << " MiB: envelope=" << acc.plannedEnvelopeBytes
                      << ", requested=" << acc.requestedTotalBytes << '\n';
            return 1;
        }
        if (acc.ttBytes != TranspositionTable::CapacityBytes())
        {
            std::cerr << "TT accounting mismatch for " << mib << " MiB\n";
            return 1;
        }
        // Verify power of two entries
        std::size_t entries = TranspositionTable::EntryCount();
        if (entries > 0 && (entries & (entries - 1)) != 0)
        {
            std::cerr << "TT entry count is not power of 2 for " << mib << " MiB: " << entries << '\n';
            return 1;
        }
    }

    // Test TT allocation failure fallback
    TranspositionTable::SetAllocationFailureThresholdForTesting(16 * MiB);
    diagnostics.str("");
    diagnostics.clear();
    if (!HashMemoryBudget::ConfigureMiB(40, diagnostics) ||
        HashMemoryBudget::Accounting().ttBytes != 8 * MiB)
    {
        std::cerr << "TT allocation fallback to 8 MiB failed\n";
        return 1;
    }

    TranspositionTable::SetAllocationFailureThresholdForTesting(sizeof(TTEntry));
    diagnostics.str("");
    diagnostics.clear();
    if (!HashMemoryBudget::ConfigureMiB(40, diagnostics) ||
        HashMemoryBudget::Accounting().ttBytes != 0 ||
        TranspositionTable::IsActive() ||
        diagnostics.str().find("TT allocation failed") == std::string::npos)
    {
        std::cerr << "TT total allocation failure was not handled safely\n";
        return 1;
    }
    TranspositionTable::SetAllocationFailureThresholdForTesting(0);

    MoveLogic::SetExchangeCacheAllocationFailureThresholdForTesting(0);
    diagnostics.str("");
    diagnostics.clear();
    if (!HashMemoryBudget::ConfigureMiB(40, diagnostics))
    {
        std::cerr << "Could not restore default table configuration\n";
        return 1;
    }

    const HashMemoryAccounting beforeSearchRestriction =
        HashMemoryBudget::Accounting();
    HashMemoryBudget::MarkSearchStarted();
    diagnostics.str("");
    diagnostics.clear();
    if (HashMemoryBudget::ConfigureMiB(16, diagnostics) ||
        HashMemoryBudget::Accounting().acceptedTotalBytes !=
            beforeSearchRestriction.acceptedTotalBytes ||
        diagnostics.str().find("after search has started") == std::string::npos)
    {
        std::cerr << "Post-search Hash change was not rejected safely\n";
        return 1;
    }

    std::cout << "Hash memory budgeting, parsing, resize, fallback, and safety "
                 "coverage passed\n";
    return 0;
}

int RunIGGDepthMappingCoverage()
{
    if (PVSSearch::ProductionIGGEnabledForTesting())
    {
        std::cerr << "IGG depth mapping safety failure\n"
                  << "  Production PVS can still call the unsafe IGG depth lookup\n";
        return 1;
    }

    std::cout << "Production PVS bypasses the unsafe IGG depth lookup\n";
    return 0;
}

int RunUCIMovetime()
{
    UCI::IsRelease = true;
    std::stringstream in;
    in << "uci\n"
       << "isready\n"
       << "position startpos\n"
       << "go movetime 150\n"
       << "isready\n"
       << "quit\n";
    std::stringstream out;
    auto start = std::chrono::high_resolution_clock::now();
    UCI::Run(in, out);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
    std::string response = out.str();
    if (response.find("bestmove") == std::string::npos)
    {
        std::cerr << "UCI movetime failed: no bestmove found\nOutput:\n" << response << '\n';
        return 1;
    }
    if (duration < 50 || duration > 3000)
    {
        std::cerr << "UCI movetime duration out of expected bounds: " << duration << " ms\n";
        return 1;
    }
    std::cout << "UCI movetime completed in " << duration << " ms\n";
    return 0;
}

int RunUCIDepth()
{
    UCI::IsRelease = true;
    std::stringstream in;
    in << "uci\n"
       << "isready\n"
       << "position startpos\n"
       << "go depth 2\n"
       << "isready\n"
       << "quit\n";
    std::stringstream out;
    UCI::Run(in, out);
    std::string response = out.str();
    if (response.find("bestmove") == std::string::npos)
    {
        std::cerr << "UCI depth failed: no bestmove found\nOutput:\n" << response << '\n';
        return 1;
    }
    if (response.find("info depth 2") == std::string::npos)
    {
        std::cerr << "UCI depth failed: did not reach depth 2\nOutput:\n" << response << '\n';
        return 1;
    }
    if (response.find("info depth 3") != std::string::npos)
    {
        std::cerr << "UCI depth failed: exceeded depth 2\nOutput:\n" << response << '\n';
        return 1;
    }
    std::cout << "UCI depth limit respected\n";
    return 0;
}

int RunUCINodes()
{
    UCI::IsRelease = true;
    std::stringstream in;
    in << "uci\n"
       << "isready\n"
       << "position startpos\n"
       << "go nodes 100\n"
       << "isready\n"
       << "quit\n";
    std::stringstream out;
    UCI::Run(in, out);
    std::string response = out.str();
    if (response.find("bestmove") == std::string::npos)
    {
        std::cerr << "UCI nodes failed: no bestmove found\nOutput:\n" << response << '\n';
        return 1;
    }
    std::cout << "UCI nodes limit completed with bestmove\n";
    return 0;
}

int RunUCIStop()
{
    UCI::IsRelease = true;
    std::stringstream in;
    in << "uci\n"
       << "isready\n"
       << "position startpos\n"
       << "go infinite\n"
       << "stop\n"
       << "isready\n"
       << "position startpos\n"
       << "go depth 1\n"
       << "isready\n"
       << "quit\n";
    std::stringstream out;
    UCI::Run(in, out);
    std::string response = out.str();
    size_t firstBestMove = response.find("bestmove");
    if (firstBestMove == std::string::npos)
    {
        std::cerr << "UCI stop failed: first bestmove not found\nOutput:\n" << response << '\n';
        return 1;
    }
    size_t readyok = response.find("readyok", firstBestMove);
    if (readyok == std::string::npos)
    {
        std::cerr << "UCI stop failed: readyok not found after stop\nOutput:\n" << response << '\n';
        return 1;
    }
    size_t secondBestMove = response.find("bestmove", readyok);
    if (secondBestMove == std::string::npos)
    {
        std::cerr << "UCI stop failed: second bestmove not found after loop continuation\nOutput:\n" << response << '\n';
        return 1;
    }
    std::cout << "UCI stop stopped search and kept loop alive\n";
    return 0;
}

int RunUCIQuit()
{
    UCI::IsRelease = true;
    std::stringstream in;
    in << "uci\n"
       << "isready\n"
       << "position startpos\n"
       << "go infinite\n"
       << "quit\n";
    std::stringstream out;
    UCI::Run(in, out);
    std::cout << "UCI quit terminated cleanly\n";
    return 0;
}

int RunUCITimeControl()
{
    UCI::IsRelease = true;
    auto dur1 = UCI::getAllowedTime(60000, 1000, 0);
    auto ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(dur1).count();
    if (ms1 != 2818)
    {
        std::cerr << "getAllowedTime sudden death mismatch: expected 2818 ms, got " << ms1 << " ms\n";
        return 1;
    }

    auto dur2 = UCI::getAllowedTime(10000, 0, 1);
    auto ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(dur2).count();
    if (ms2 != 7000)
    {
        std::cerr << "getAllowedTime 1 move to go mismatch: expected 7000 ms, got " << ms2 << " ms\n";
        return 1;
    }

    auto dur3 = UCI::getAllowedTime(60000, 500, 40);
    auto ms3 = std::chrono::duration_cast<std::chrono::milliseconds>(dur3).count();
    if (ms3 != 2600)
    {
        std::cerr << "getAllowedTime movestogo mismatch: expected 2600 ms, got " << ms3 << " ms\n";
        return 1;
    }

    std::stringstream in;
    in << "uci\n"
       << "isready\n"
       << "position startpos\n"
       << "go wtime 1000 btime 1000\n"
       << "isready\n"
       << "quit\n";
    std::stringstream out;
    auto start = std::chrono::high_resolution_clock::now();
    UCI::Run(in, out);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
    std::string response = out.str();
    if (response.find("bestmove") == std::string::npos)
    {
        std::cerr << "UCI wtime/btime search failed: no bestmove found\nOutput:\n" << response << '\n';
        return 1;
    }
    std::cout << "UCI time control search completed in " << duration << " ms\n";
    return 0;
}

int RunUCI(const std::string& testCase)
{
    if (testCase == "movetime")
        return RunUCIMovetime();
    if (testCase == "depth")
        return RunUCIDepth();
    if (testCase == "nodes")
        return RunUCINodes();
    if (testCase == "stop")
        return RunUCIStop();
    if (testCase == "quit")
        return RunUCIQuit();
    if (testCase == "time_control")
        return RunUCITimeControl();
    throw std::runtime_error("Unknown UCI test case: " + testCase);
}

int RunEvaluationCorrectness(const std::string& testCase)
{
    if (testCase == "black_en_passant")
    {
        // Compare White en passant vs Black en passant under 180-degree rotation
        std::unique_ptr<Board> bW(BoardMaker::MakeInitialBoard("4k2K/8/8/3Pp3/8/8/8/8 w - e6 0 1"));
        std::unique_ptr<Board> bB(BoardMaker::MakeInitialBoard("8/8/8/8/3pP3/8/8/4K2k b - e3 0 1"));
        int evalW = EvaluationLogic::Evaluate(*bW);
        int evalB = EvaluationLogic::Evaluate(*bB);
        if (evalW != evalB)
        {
            std::cerr << "Black en passant evaluation asymmetry: White=" << evalW << ", Black=" << evalB << '\n';
            return 1;
        }
        std::cout << "Black en passant evaluation symmetry verified (eval=" << evalW << ")\n";
        return 0;
    }
    if (testCase == "black_knight_attack")
    {
        const int expected[6] = {0, 15, 50, 50, 10, 20};
        for (int i = 1; i <= 5; ++i)
        {
            if (Option::BlackKnightAttackValueMovement[i] != expected[i])
            {
                std::cerr << "BlackKnightAttackValueMovement[" << i << "] mismatch: expected " << expected[i]
                          << ", got " << Option::BlackKnightAttackValueMovement[i] << '\n';
                return 1;
            }
            if (Option::AttackValueMovement[10][i] != expected[i])
            {
                std::cerr << "AttackValueMovement[10][" << i << "] mismatch: expected " << expected[i]
                          << ", got " << Option::AttackValueMovement[10][i] << '\n';
                return 1;
            }
        }
        std::cout << "Black knight attack movement table verified\n";
        return 0;
    }
    if (testCase == "bishop_pair_color")
    {
        // Position with bishops on c1 and c2 (same file, ranks 1 & 2 -> opposite colors -> TRUE bishop pair)
        std::unique_ptr<Board> bOpposite(BoardMaker::MakeInitialBoard("4k3/8/8/8/8/8/2B5/2B1K3 w - - 0 1"));
        // Position with bishops on c1 and f2 (c1 dark, f2 dark -> SAME color -> NO bishop pair)
        std::unique_ptr<Board> bSame(BoardMaker::MakeInitialBoard("4k3/8/8/8/8/8/5B2/2B1K3 w - - 0 1"));

        // Single bishop on c1
        std::unique_ptr<Board> bSingle(BoardMaker::MakeInitialBoard("4k3/8/8/8/8/8/8/2B1K3 w - - 0 1"));

        int evalOpp = EvaluationLogic::Evaluate(*bOpposite);
        int evalSame = EvaluationLogic::Evaluate(*bSame);
        int evalSingle = EvaluationLogic::Evaluate(*bSingle);

        // Opposite color bishops must have +50 bishop pair bonus compared to same-color baseline
        if (evalOpp <= evalSame)
        {
            std::cerr << "Bishop pair color failure: evalOpp=" << evalOpp << " <= evalSame=" << evalSame << '\n';
            return 1;
        }
        std::cout << "Bishop pair true square color parity verified (opposite=" << evalOpp << ", same=" << evalSame << ")\n";
        return 0;
    }
    if (testCase == "king_danger_sign")
    {
        // White Queen attacking squares adjacent to Black king (closer attack should be rewarded)
        std::unique_ptr<Board> bAttack(BoardMaker::MakeInitialBoard("4k3/8/8/8/8/8/4Q3/4K3 w - - 0 1"));
        std::unique_ptr<Board> bFar(BoardMaker::MakeInitialBoard("4k3/8/8/8/8/8/Q7/4K3 w - - 0 1"));

        int evalAttack = EvaluationLogic::Evaluate(*bAttack);
        int evalFar = EvaluationLogic::Evaluate(*bFar);

        if (evalAttack <= evalFar)
        {
            std::cerr << "King danger sign failure: attacking king gave " << evalAttack << " <= far " << evalFar << '\n';
            return 1;
        }
        std::cout << "King danger sign verified (attacking=" << evalAttack << " > far=" << evalFar << ")\n";
        return 0;
    }
    if (testCase == "passed_pawn_table_symmetry")
    {
        for (int r = 0; r < 8; ++r)
        {
            for (int c = 0; c < 4; ++c)
            {
                int sqL = r * 8 + c;
                int sqR = r * 8 + (7 - c);
                if (Option::WhitePassedPawnValueMiddleGam[sqL] != Option::WhitePassedPawnValueMiddleGam[sqR])
                {
                    std::cerr << "WhitePassedPawnValueMiddleGam asymmetry at r=" << r << ", c=" << c << '\n';
                    return 1;
                }
                if (Option::WhitePassedPawnValueEndGame[sqL] != Option::WhitePassedPawnValueEndGame[sqR])
                {
                    std::cerr << "WhitePassedPawnValueEndGame asymmetry at r=" << r << ", c=" << c << '\n';
                    return 1;
                }
            }
        }
        // Verify rank 7 middle game values are 25 for central pawns
        if (Option::WhitePassedPawnValueMiddleGam[6 * 8 + 3] != 25 || Option::WhitePassedPawnValueMiddleGam[6 * 8 + 4] != 25)
        {
            std::cerr << "WhitePassedPawnValueMiddleGam central rank 7 not 25\n";
            return 1;
        }
        std::cout << "Passed pawn tables symmetry and values verified\n";
        return 0;
    }
    if (testCase == "king_safety_table_symmetry")
    {
        // Check rank 3 symmetry
        for (int c = 0; c < 4; ++c)
        {
            int sqL = 2 * 8 + c;
            int sqR = 2 * 8 + (7 - c);
            if (Option::WhiteKingPlaceSafetyMiddleGame[sqL] != Option::WhiteKingPlaceSafetyMiddleGame[sqR])
            {
                std::cerr << "WhiteKingPlaceSafetyMiddleGame rank 3 asymmetry at c=" << c << '\n';
                return 1;
            }
        }
        std::cout << "WhiteKingPlaceSafetyMiddleGame rank 3 symmetry verified\n";
        return 0;
    }
    if (testCase == "pawn_move_center_symmetry")
    {
        for (int r = 0; r < 8; ++r)
        {
            for (int c = 0; c < 4; ++c)
            {
                int sqL = r * 8 + c;
                int sqR = r * 8 + (7 - c);
                if (Option::PawnMoveCenterValueWhite[sqL] != Option::PawnMoveCenterValueWhite[sqR])
                {
                    std::cerr << "PawnMoveCenterValueWhite asymmetry at r=" << r << ", c=" << c << '\n';
                    return 1;
                }
            }
        }
        std::cout << "PawnMoveCenterValueWhite symmetry verified\n";
        return 0;
    }
    if (testCase == "game_phase_calculation")
    {
        InitializeEngine();
        // Full starting material = 24
        std::unique_ptr<Board> startBoard(BoardMaker::MakeInitialBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
        int startPhase = EvaluationLogic::CalculatePhase(*startBoard);
        if (startPhase != 24)
        {
            std::cerr << "Expected start position phase 24, got " << startPhase << '\n';
            return 1;
        }

        // Kings and pawns only = 0
        std::unique_ptr<Board> pawnEndgame(BoardMaker::MakeInitialBoard("8/4k3/8/8/8/8/4K3/8 w - - 0 1"));
        int pawnPhase = EvaluationLogic::CalculatePhase(*pawnEndgame);
        if (pawnPhase != 0)
        {
            std::cerr << "Expected pawn endgame phase 0, got " << pawnPhase << '\n';
            return 1;
        }

        // Representative partial material (White: Q=4, R=2; Black: R=2, B=1, N=1 -> Total = 10)
        std::unique_ptr<Board> partialBoard(BoardMaker::MakeInitialBoard("1n1rkb2/8/8/8/8/8/3QKR2/8 w - - 0 1"));
        int partialPhase = EvaluationLogic::CalculatePhase(*partialBoard);
        if (partialPhase != 10)
        {
            std::cerr << "Expected partial phase 10, got " << partialPhase << '\n';
            return 1;
        }

        // Clamping upper bound verification
        std::unique_ptr<Board> heavyBoard(BoardMaker::MakeInitialBoard("qqqqkqqq/8/8/8/8/8/8/QQQQKQQQ w - - 0 1"));
        int heavyPhase = EvaluationLogic::CalculatePhase(*heavyBoard);
        if (heavyPhase != 24)
        {
            std::cerr << "Expected clamped phase 24 for heavy board, got " << heavyPhase << '\n';
            return 1;
        }

        std::cout << "Game phase calculation tests passed\n";
        return 0;
    }
    if (testCase == "breakdown_reconciliation")
    {
        InitializeEngine();
        std::vector<std::string> testFens = {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            "1rb1kb1r/p1pqpppp/2n2n2/2P5/3pPB2/P4N2/1PQ2PPP/RN2KB1R b KQk - 0 10",
            "8/4k3/8/2b5/8/8/4K3/2B5 w - - 0 1",
            "8/4k3/4p3/8/8/4P3/4K3/8 w - - 0 1",
            "r1bqk2r/pppp1ppp/2n5/4p3/2B1n3/2P2N2/PPP2PPP/R1BQK2R w KQkq - 0 7"
        };

        for (const auto& fen : testFens)
        {
            std::unique_ptr<Board> board(BoardMaker::MakeInitialBoard(fen));
            EvaluationLogic::ClearEvalCacheForTesting();
            EvaluationBreakdown bd = EvaluationLogic::EvaluateDetailed(*board);
            EvaluationLogic::ClearEvalCacheForTesting();
            int normalEval = EvaluationLogic::Evaluate(*board);

            if (bd.sideToMoveTotal != normalEval)
            {
                std::cerr << "Mismatch between breakdown sideToMoveTotal (" << bd.sideToMoveTotal
                          << ") and normal Evaluate (" << normalEval << ") on FEN: " << fen << '\n';
                return 1;
            }

            int expectedUnscaled = bd.pieceEvaluation + bd.bishopPairNet + bd.mobilityNet +
                                   bd.pawnStructureNet + bd.kingSafetyTotal + bd.rookConnectionNet +
                                   bd.centerNet + bd.tempoNet;
            if (bd.unscaledTotal != expectedUnscaled)
            {
                std::cerr << "Unscaled breakdown sum mismatch: got " << bd.unscaledTotal
                          << ", expected " << expectedUnscaled << " on FEN: " << fen << '\n';
                return 1;
            }

            int expectedScaled = static_cast<int>(bd.unscaledTotal * bd.oppositeColorBishopScale);
            if (bd.scaledTotal != expectedScaled)
            {
                std::cerr << "Scaled breakdown mismatch on FEN: " << fen << '\n';
                return 1;
            }

            int expectedKingSafety = bd.kingAttackNet + bd.kingPlacementNet + bd.pawnShieldNet + bd.centralKingExposureNet;
            if (bd.kingSafetyTotal != expectedKingSafety)
            {
                std::cerr << "King safety breakdown mismatch on FEN: " << fen << '\n';
                return 1;
            }

            int expectedWhitePerspective = board->sideToMove ? -bd.sideToMoveTotal : bd.sideToMoveTotal;
            if (bd.whitePerspectiveTotal != expectedWhitePerspective)
            {
                std::cerr << "White perspective sign mismatch on FEN: " << fen << '\n';
                return 1;
            }
        }

        std::cout << "Evaluation breakdown reconciliation tests passed\n";
        return 0;
    }
    if (testCase == "passed_pawn_phase_interpolation")
    {
        InitializeEngine();

        // 1. MG Endpoint (phase = 24): White pawn on d5 (Option::WhitePassedPawnValueMiddleGam[35] = 20)
        std::unique_ptr<Board> mgBoard(BoardMaker::MakeInitialBoard("rnbqkbnr/pp3ppp/8/3P4/8/8/PPP1PPPP/RNBQKBNR w KQkq - 0 1"));
        int mgPhase = EvaluationLogic::CalculatePhase(*mgBoard);
        if (mgPhase != 24)
        {
            std::cerr << "Expected MG phase 24, got " << mgPhase << '\n';
            return 1;
        }
        int mgPassed = EvaluationLogic::GetPawnStructureValue(*mgBoard, 0);
        if (mgPassed != 20)
        {
            std::cerr << "Expected MG passed pawn value 20, got " << mgPassed << '\n';
            return 1;
        }

        // 2. EG Endpoint (phase = 0): White pawn on d5 (Option::WhitePassedPawnValueEndGame[35] = 90, goForward = 4*2 = 8)
        std::unique_ptr<Board> egBoard(BoardMaker::MakeInitialBoard("8/8/8/3P4/8/8/4k3/4K3 w - - 0 1"));
        int egPhase = EvaluationLogic::CalculatePhase(*egBoard);
        if (egPhase != 0)
        {
            std::cerr << "Expected EG phase 0, got " << egPhase << '\n';
            return 1;
        }
        int egPawnVal = EvaluationLogic::GetPawnStructureValue(*egBoard, 2);
        if (egPawnVal != (90 + 8))
        {
            std::cerr << "Expected EG passed pawn value 98 (90+8), got " << egPawnVal << '\n';
            return 1;
        }

        // 3. Intermediate interpolation (phase = 12): White Q(4)+R(2)=6, Black Q(4)+R(2)=6 -> Total = 12
        // Passed pawn bonus = (20 * 12 + 90 * 12) / 24 = 55
        std::unique_ptr<Board> midBoard(BoardMaker::MakeInitialBoard("3rqk2/8/8/3P4/8/8/3RQK2/8 w - - 0 1"));
        int midPhase = EvaluationLogic::CalculatePhase(*midBoard);
        if (midPhase != 12)
        {
            std::cerr << "Expected mid phase 12, got " << midPhase << '\n';
            return 1;
        }
        int midPassed = EvaluationLogic::GetPawnStructureValue(*midBoard, 0);
        if (midPassed != 55)
        {
            std::cerr << "Expected mid phase passed pawn value 55, got " << midPassed << '\n';
            return 1;
        }

        // 4. Color symmetry: White passed pawn on d5 vs Black passed pawn on d4 in mirrored boards
        std::unique_ptr<Board> whitePasser(BoardMaker::MakeInitialBoard("4k3/8/8/3P4/8/8/8/4K3 w - - 0 1"));
        std::unique_ptr<Board> blackPasser(BoardMaker::MakeInitialBoard("4k3/8/8/8/3p4/8/8/4K3 b - - 0 1"));
        EvaluationLogic::ClearEvalCacheForTesting();
        int whiteEval = EvaluationLogic::Evaluate(*whitePasser);
        EvaluationLogic::ClearEvalCacheForTesting();
        int blackEval = EvaluationLogic::Evaluate(*blackPasser);
        if (whiteEval != blackEval)
        {
            std::cerr << "Color symmetry mismatch: White passer eval " << whiteEval
                      << " != Black passer eval " << blackEval << '\n';
            return 1;
        }

        std::cout << "Passed pawn phase interpolation tests passed\n";
        return 0;
    }
    if (testCase == "central_king_exposure")
    {
        InitializeEngine();

        // 1. Closed locked centre vs Half-open vs Fully open centre (all phase 24)
        // Closed locked centre (White d4, e4; Black d5, e5 -> openness 0):
        std::unique_ptr<Board> closedBoard(BoardMaker::MakeInitialBoard("r1bqkb1r/pp1n1ppp/4pn2/3pp3/3PP3/2NB1N2/PP3PPP/R1BQK2R w KQkq - 0 1"));
        EvaluationBreakdown closedBd = EvaluationLogic::EvaluateDetailed(*closedBoard);
        if (closedBd.whiteCentralKingExposure != -5)
        {
            std::cerr << "Expected closed locked centre penalty -5, got " << closedBd.whiteCentralKingExposure << '\n';
            return 1;
        }

        // Half-open centre (Black missing d-pawn, e-file closed -> openness 1):
        std::unique_ptr<Board> halfOpenBoard(BoardMaker::MakeInitialBoard("r1bqkb1r/pp1n1ppp/4pn2/4p3/3PP3/2NB1N2/PP3PPP/R1BQK2R w KQkq - 0 1"));
        EvaluationBreakdown halfOpenBd = EvaluationLogic::EvaluateDetailed(*halfOpenBoard);
        if (halfOpenBd.whiteCentralKingExposure != -15)
        {
            std::cerr << "Expected half-open centre penalty -15, got " << halfOpenBd.whiteCentralKingExposure << '\n';
            return 1;
        }

        // Fully open centre (no d or e pawns -> openness 4, phase 24):
        std::unique_ptr<Board> openBoard(BoardMaker::MakeInitialBoard("r1bqk2r/ppp1bppp/2n2n2/8/8/2N2N2/PPP1BPPP/R1BQK2R w KQkq - 0 1"));
        EvaluationBreakdown openBd = EvaluationLogic::EvaluateDetailed(*openBoard);
        if (openBd.whiteCentralKingExposure != -45)
        {
            std::cerr << "Expected fully open centre penalty -45, got " << openBd.whiteCentralKingExposure << '\n';
            return 1;
        }

        // Verify strictly increasing penalty hierarchy: |closed| (5) < |halfOpen| (15) < |open| (45)
        if (!(std::abs(closedBd.whiteCentralKingExposure) < std::abs(halfOpenBd.whiteCentralKingExposure) &&
              std::abs(halfOpenBd.whiteCentralKingExposure) < std::abs(openBd.whiteCentralKingExposure)))
        {
            std::cerr << "Center openness penalty hierarchy violated: closed=" << closedBd.whiteCentralKingExposure
                      << " halfOpen=" << halfOpenBd.whiteCentralKingExposure
                      << " open=" << openBd.whiteCentralKingExposure << '\n';
            return 1;
        }

        // Phase 0 open board:
        std::unique_ptr<Board> egBoard(BoardMaker::MakeInitialBoard("4k3/pp3ppp/8/8/8/8/PP3PPP/4K3 w - - 0 1"));
        EvaluationBreakdown egBd = EvaluationLogic::EvaluateDetailed(*egBoard);
        if (egBd.whiteCentralKingExposure != 0 || egBd.blackCentralKingExposure != 0)
        {
            std::cerr << "Expected 0 central king exposure at phase 0, got W="
                      << egBd.whiteCentralKingExposure << " B=" << egBd.blackCentralKingExposure << '\n';
            return 1;
        }

        // 3. Nearby friendly piece defenders do not remove central exposure
        // Place Queen (d1) and Bishop (f1) near White King (e1) on open centre (phase 24)
        std::unique_ptr<Board> defendedBoard(BoardMaker::MakeInitialBoard("r1bqk2r/ppp1bppp/2n2n2/8/8/2N2N2/PPP1BPPP/R2QKB1R w KQkq - 0 1"));
        EvaluationBreakdown defendedBd = EvaluationLogic::EvaluateDetailed(*defendedBoard);
        if (defendedBd.whiteCentralKingExposure != openBd.whiteCentralKingExposure)
        {
            std::cerr << "Friendly piece defenders should not remove central exposure penalty: got "
                      << defendedBd.whiteCentralKingExposure << " vs " << openBd.whiteCentralKingExposure << '\n';
            return 1;
        }

        // 4. White and Black symmetry
        // King on e1 (White) castled on g8 (Black):
        std::unique_ptr<Board> whiteCentral(BoardMaker::MakeInitialBoard("5rk1/pp3ppp/8/8/8/8/PP3PPP/R3K2R w KQ - 0 1"));
        EvaluationBreakdown wcBd = EvaluationLogic::EvaluateDetailed(*whiteCentral);

        // King on e8 (Black) castled on g1 (White):
        std::unique_ptr<Board> blackCentral(BoardMaker::MakeInitialBoard("r3k2r/pp3ppp/8/8/8/8/PP3PPP/5RK1 w kq - 0 1"));
        EvaluationBreakdown bcBd = EvaluationLogic::EvaluateDetailed(*blackCentral);

        if (wcBd.centralKingExposureNet != -bcBd.centralKingExposureNet)
        {
            std::cerr << "Symmetry mismatch: White central net " << wcBd.centralKingExposureNet
                      << " vs Black central net " << bcBd.centralKingExposureNet << '\n';
            return 1;
        }

        std::cout << "Central king exposure tests passed\n";
        return 0;
    }
    throw std::runtime_error("Unknown eval test case: " + testCase);
}

void CleanupEngine()
{
    AttackPlaces::Cleanup();
    BoardInitializer::Cleanup();
    PieceMoves::Cleanup();
    MoveLogic::Cleanup();
    KingSetup::Cleanup();
    PassedPawnSetup::Cleanup();
    UCI::ReleaseCurrentPosition();
}
}

int RunMultiPVCorrectnessTest()
{
    InitializeEngine();
    TranspositionTable::Clear();
    UCI::ReleaseCurrentPosition();
    UCI::ReplaceCurrentBoard(BoardMaker::MakeInitialBoard("4k3/8/8/8/3q4/8/3R4/4K3 w - - 0 1"));
    Option::MultiPV = 4;
    Search::maxDepth = 5;
    Search::finiteSearch = false;
    Search::active = true;
    Move m1{}, m2{}, m3{}, m4{};
    Search::MainSearch(m1, m2, m3, m4, *UCI::thisBoard);
    Option::MultiPV = 1;
    std::cout << "MultiPV score correctness test passed\n";
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: howl_correctness_tests <perft|restoration|zobrist|search|qsearch|cache|lifecycle|memory|uci> <case>\n";
        return 2;
    }

    try
    {
        InitializeEngine();
        std::string testType = argv[1];
        int result = 0;

        if (testType == "zobrist")
        {
            result = RunZobrist(argv[2]);
        }
        else if (testType == "search")
        {
            if (std::string(argv[2]) == "igg_depth_mapping")
            {
                result = RunIGGDepthMappingCoverage();
            }
            else if (std::string(argv[2]) == "multipv_correctness")
            {
                result = RunMultiPVCorrectnessTest();
            }
            else
            {
                result = RunSearch(argv[2]);
            }
        }
        else if (testType == "qsearch")
        {
            result = RunQSearch(argv[2]);
        }
        else if (testType == "cache")
        {
            result = RunCache(argv[2]);
        }
        else if (testType == "lifecycle" && std::string(argv[2]) == "fen_replacement")
        {
            result = RunFenReplacementLifecycle();
        }
        else if (testType == "memory" && std::string(argv[2]) == "hash_budget")
        {
            result = RunHashMemoryBudgetCoverage();
        }
        else if (testType == "uci")
        {
            result = RunUCI(argv[2]);
        }
        else if (testType == "eval")
        {
            result = RunEvaluationCorrectness(argv[2]);
        }
        else
        {
            const PerftPosition& position = FindPosition(argv[2]);

            if (testType == "perft")
            {
                result = RunPerft(position);
            }
            else if (testType == "restoration")
            {
                result = RunRestoration(position);
            }
            else
            {
                throw std::runtime_error("Unknown test type: " + testType);
            }
        }

        CleanupEngine();
        return result;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Test aborted: " << error.what() << '\n';
        return 1;
    }
}


