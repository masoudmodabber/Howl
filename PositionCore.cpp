#include "PositionCore.h"

#include "Board.h"
#include "Move.h"
#include "MissingInfoAboutPrevStateFromMove.h"
#include "Option.h"

#include <cstdlib>
#include <iostream>

namespace
{
constexpr std::uint8_t White = 0;
constexpr std::uint8_t Black = 1;

int PieceTypeIndex(int piece)
{
    if (piece == 0)
        return -1;
    const int type = piece > 8 ? piece - 8 : piece;
    return type >= 1 && type <= 6 ? type - 1 : -1;
}

std::uint8_t PieceColour(int piece)
{
    return piece > 8 ? Black : White;
}

std::uint8_t CastlingRights(const Board& board)
{
    return static_cast<std::uint8_t>(
        (board.whiteSmallCastle ? 1 : 0) |
        (board.whiteBigCastle ? 2 : 0) |
        (board.blackSmallCastle ? 4 : 0) |
        (board.blackBigCastle ? 8 : 0));
}

void SynchronizeSquare(PositionCore& core, const Board& board, int square)
{
    if (square < 0 || square >= 64)
        return;

    const Bitboard squareBit = Option::PowerTwo[square];
    const int oldPiece = core.pieceAt[square];
    const int oldType = PieceTypeIndex(oldPiece);
    if (oldType >= 0)
    {
        const std::uint8_t oldColour = PieceColour(oldPiece);
        core.colourOccupancy[oldColour] &= ~squareBit;
        core.pieceOccupancy[oldType] &= ~squareBit;
        if (oldType == 5)
            core.kingSquare[oldColour] = 255;
    }

    const int newPiece = board.mainBoard[square];
    core.pieceAt[square] = static_cast<std::uint8_t>(newPiece);
    const int newType = PieceTypeIndex(newPiece);
    if (newType >= 0)
    {
        const std::uint8_t colour = PieceColour(newPiece);
        core.colourOccupancy[colour] |= squareBit;
        core.pieceOccupancy[newType] |= squareBit;
        if (newType == 5)
            core.kingSquare[colour] = static_cast<std::uint8_t>(square);
    }
}

void SynchronizeScalars(PositionCore& core, const Board& board)
{
    core.zobristHash = static_cast<std::uint64_t>(board.ZobristHashCode);
    core.fullmoveNumber = static_cast<std::uint32_t>(board.moveNumber);
    core.halfmoveClock = static_cast<std::uint16_t>(board.fiftyMoveRule);
    core.sideToMove = board.sideToMove ? 1 : 0;
    core.castlingRights = CastlingRights(board);
    core.enPassantSquare = static_cast<std::int8_t>(board.unpassentPlace);
}

void SetPiece(PositionCore& core, int square, int piece)
{
    const Bitboard bit = Option::PowerTwo[square];
    const int oldPiece = core.pieceAt[square];
    const int oldType = PieceTypeIndex(oldPiece);
    if (oldType >= 0)
    {
        const std::uint8_t colour = PieceColour(oldPiece);
        core.colourOccupancy[colour] &= ~bit;
        core.pieceOccupancy[oldType] &= ~bit;
        if (oldType == 5)
            core.kingSquare[colour] = 255;
    }
    core.pieceAt[square] = static_cast<std::uint8_t>(piece);
    const int newType = PieceTypeIndex(piece);
    if (newType >= 0)
    {
        const std::uint8_t colour = PieceColour(piece);
        core.colourOccupancy[colour] |= bit;
        core.pieceOccupancy[newType] |= bit;
        if (newType == 5)
            core.kingSquare[colour] = static_cast<std::uint8_t>(square);
    }
}

int CapturedSquare(const Move& move, bool movingWhite)
{
    return (move.PublicFlag & Option::PowerTwo[6]) != 0
        ? move.endPlace + (movingWhite ? -8 : 8)
        : move.endPlace;
}

void UpdateMoveSquares(PositionCore& core, const Move& move, bool movingWhite,
                       bool undo)
{
    if (move.promotionPiece < 0)
        return;

    const int capturedSquare = CapturedSquare(move, movingWhite);
    if (!undo)
    {
        const int movingPiece = core.pieceAt[move.beginPlace];
        SetPiece(core, move.beginPlace, 0);
        if ((move.PublicFlag & Option::PowerTwo[6]) != 0)
            SetPiece(core, capturedSquare, 0);
        const int promotedPiece = move.promotionPiece > 0
            ? move.promotionPiece : movingPiece;
        SetPiece(core, move.endPlace, promotedPiece);
    }
    else
    {
        const int currentPiece = core.pieceAt[move.endPlace];
        const int movingPiece = move.promotionPiece > 0
            ? (movingWhite ? 1 : 9) : currentPiece;
        SetPiece(core, move.endPlace, 0);
        SetPiece(core, move.beginPlace, movingPiece);
        if ((move.PublicFlag & Option::PowerTwo[6]) != 0)
            SetPiece(core, capturedSquare, movingWhite ? 9 : 1);
        else if (move.endPiece > 0)
            SetPiece(core, move.endPlace, move.endPiece);
    }

    int rookFrom = -1;
    int rookTo = -1;
    if ((move.CastleFlag & (Option::PowerTwo[3] | Option::PowerTwo[1])) != 0)
    {
        rookFrom = move.beginPlace + 3;
        rookTo = move.beginPlace + 1;
    }
    else if ((move.CastleFlag & (Option::PowerTwo[2] | Option::PowerTwo[0])) != 0)
    {
        rookFrom = move.beginPlace - 4;
        rookTo = move.beginPlace - 1;
    }
    if (rookFrom >= 0)
    {
        const int rook = movingWhite ? 4 : 12;
        SetPiece(core, undo ? rookTo : rookFrom, 0);
        SetPiece(core, undo ? rookFrom : rookTo, rook);
    }
}


[[noreturn]] void Mismatch(const Board& board, const Move* move,
                           const char* operation, const char* field,
                           int square = -1)
{
    std::cerr << "PositionCore mismatch after " << operation
              << ": field=" << field;
    if (square >= 0)
        std::cerr << " square=" << square;
    if (move != nullptr)
        std::cerr << " move=" << int(move->beginPlace) << '-' << int(move->endPlace)
                  << " promotion=" << int(move->promotionPiece)
                  << " publicFlags=" << int(static_cast<unsigned char>(move->PublicFlag))
                  << " castleFlags=" << int(static_cast<unsigned char>(move->CastleFlag));
    std::cerr << " hash=" << board.ZobristHashCode << '\n';
    std::abort();
}
}

void PositionCoreLogic::Initialize(Board& board)
{
    board.positionCore = PositionCore{};
    for (int square = 0; square < 64; ++square)
        SynchronizeSquare(board.positionCore, board, square);
    SynchronizeScalars(board.positionCore, board);
}

void PositionCoreLogic::PrepareMove(const Board& board, const Move& move,
                                    MissingInfoAboutPrevStateFromMove& missingInfo)
{
    missingInfo.movedPieceIndex = -1;
    missingInfo.capturedPieceIndex = -1;
}

void PositionCoreLogic::UpdateAfterMove(Board& board, const Move& move,
                                        bool movingWhite,
                                        const MissingInfoAboutPrevStateFromMove* missingInfo)
{
    UpdateMoveSquares(board.positionCore, move, movingWhite, false);
    SynchronizeScalars(board.positionCore, board);
}

void PositionCoreLogic::UpdateAfterUndo(Board& board, const Move& move,
                                        bool movingWhite,
                                        const MissingInfoAboutPrevStateFromMove& missingInfo)
{
    UpdateMoveSquares(board.positionCore, move, movingWhite, true);
    SynchronizeScalars(board.positionCore, board);
}

void PositionCoreLogic::Verify(const Board& board, const Move* move,
                               const char* operation)
{
    Bitboard expectedColours[2] = {};
    Bitboard expectedPieces[6] = {};
    std::uint8_t expectedKings[2] = {255, 255};
    for (int square = 0; square < 64; ++square)
    {
        const int piece = board.mainBoard[square];
        if (board.positionCore.pieceAt[square] != piece)
            Mismatch(board, move, operation, "pieceAt", square);
        const int type = PieceTypeIndex(piece);
        if (type >= 0)
        {
            expectedColours[PieceColour(piece)] |= Option::PowerTwo[square];
            expectedPieces[type] |= Option::PowerTwo[square];
            if (type == 5)
                expectedKings[PieceColour(piece)] = static_cast<std::uint8_t>(square);
        }
    }
    if (board.positionCore.colourOccupancy[White] != expectedColours[White])
        Mismatch(board, move, operation, "white occupancy vs board squares");
    if (board.positionCore.colourOccupancy[Black] != expectedColours[Black])
        Mismatch(board, move, operation, "black occupancy vs board squares");
    for (int type = 0; type < 6; ++type)
        if (board.positionCore.pieceOccupancy[type] != expectedPieces[type])
            Mismatch(board, move, operation, "piece occupancy", type);
    if (board.positionCore.kingSquare[White] != expectedKings[White])
        Mismatch(board, move, operation, "white king square");
    if (board.positionCore.kingSquare[Black] != expectedKings[Black])
        Mismatch(board, move, operation, "black king square");
    if (board.positionCore.sideToMove != static_cast<std::uint8_t>(board.sideToMove))
        Mismatch(board, move, operation, "side to move");
    if (board.positionCore.castlingRights != CastlingRights(board))
        Mismatch(board, move, operation, "castling rights");
    if (board.positionCore.enPassantSquare != static_cast<std::int8_t>(board.unpassentPlace))
        Mismatch(board, move, operation, "en passant square");
    if (board.positionCore.zobristHash != static_cast<std::uint64_t>(board.ZobristHashCode))
        Mismatch(board, move, operation, "zobrist hash");
    if (board.positionCore.fullmoveNumber != static_cast<std::uint32_t>(board.moveNumber))
        Mismatch(board, move, operation, "fullmove number");
    if (board.positionCore.halfmoveClock != static_cast<std::uint16_t>(board.fiftyMoveRule))
        Mismatch(board, move, operation, "halfmove clock");
}
