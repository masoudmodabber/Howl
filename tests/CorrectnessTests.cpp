#include "BoardInitializer.h"
#include "BoardLogic.h"
#include "BoardMaker.h"
#include "GameLogic.h"
#include "KingSetup.h"
#include "MoveLogic.h"
#include "Option.h"
#include "PassedPawnSetup.h"
#include "PieceMoves.h"
#include "PVSSearch.h"

#include <algorithm>
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
        std::cerr << "Usage: howl_correctness_tests <perft|restoration|zobrist|search> <case>\n";
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
