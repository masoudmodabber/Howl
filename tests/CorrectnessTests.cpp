#include "BoardInitializer.h"
#include "BoardLogic.h"
#include "BoardMaker.h"
#include "EvaluationLogic.h"
#include "GameLogic.h"
#include "HashMemoryBudget.h"
#include "KingSetup.h"
#include "MoveLogic.h"
#include "Option.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "PVSSearch.h"
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
            capacityCase.evalBytes + FixedExchangeBytes;
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
            MiB + FixedExchangeBytes)
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
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: howl_correctness_tests <perft|restoration|zobrist|search|cache|lifecycle|memory> <case>\n";
        return 2;
    }

    try
    {
        InitializeEngine();
        std::string testType = argv[1];

        if (testType == "zobrist")
        {
            return RunZobrist(argv[2]);
        }
        if (testType == "search")
        {
            return RunSearch(argv[2]);
        }
        if (testType == "cache")
        {
            return RunCache(argv[2]);
        }
        if (testType == "lifecycle" && std::string(argv[2]) == "fen_replacement")
        {
            return RunFenReplacementLifecycle();
        }
        if (testType == "memory" && std::string(argv[2]) == "hash_budget")
        {
            return RunHashMemoryBudgetCoverage();
        }

        const PerftPosition& position = FindPosition(argv[2]);

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
