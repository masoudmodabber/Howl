#include "PositionCore.h"

#include "Board.h"
#include "Move.h"
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

void SynchronizeMoveSquares(Board& board, const Move& move, bool movingWhite,
                            bool undo)
{
    PositionCore& core = board.positionCore;
    if (move.promotionPiece < 0)
    {
        SynchronizeScalars(core, board);
        return;
    }

    if (undo)
    {
        SynchronizeSquare(core, board, move.endPlace);
        SynchronizeSquare(core, board, move.beginPlace);
    }
    else
    {
        SynchronizeSquare(core, board, move.beginPlace);
        SynchronizeSquare(core, board, move.endPlace);
    }

    if ((move.PublicFlag & Option::PowerTwo[6]) != 0)
    {
        const int capturedPawnSquare = move.endPlace + (movingWhite ? -8 : 8);
        SynchronizeSquare(core, board, capturedPawnSquare);
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
        SynchronizeSquare(core, board, rookFrom);
        SynchronizeSquare(core, board, rookTo);
    }

    SynchronizeScalars(core, board);
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

void PositionCoreLogic::UpdateAfterMove(Board& board, const Move& move,
                                        bool movingWhite)
{
    SynchronizeMoveSquares(board, move, movingWhite, false);
}

void PositionCoreLogic::UpdateAfterUndo(Board& board, const Move& move,
                                        bool movingWhite)
{
    SynchronizeMoveSquares(board, move, movingWhite, true);
}

void PositionCoreLogic::Verify(const Board& board, const Move* move,
                               const char* operation)
{
    Bitboard expectedColours[2] = {};
    Bitboard expectedPieces[6] = {};
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
        }
    }
    if (board.positionCore.colourOccupancy[White] != expectedColours[White])
        Mismatch(board, move, operation, "white occupancy vs board squares");
    if (board.positionCore.colourOccupancy[White] != static_cast<Bitboard>(board.whitePieces))
        Mismatch(board, move, operation, "white occupancy vs legacy occupancy");
    if (board.positionCore.colourOccupancy[Black] != expectedColours[Black])
        Mismatch(board, move, operation, "black occupancy vs board squares");
    if (board.positionCore.colourOccupancy[Black] != static_cast<Bitboard>(board.blackPieces))
        Mismatch(board, move, operation, "black occupancy vs legacy occupancy");
    for (int type = 0; type < 6; ++type)
        if (board.positionCore.pieceOccupancy[type] != expectedPieces[type])
            Mismatch(board, move, operation, "piece occupancy", type);
    const std::uint8_t expectedWhiteKing = board.pieces[6].count > 0
        ? static_cast<std::uint8_t>(board.pieces[6].front()) : 255;
    const std::uint8_t expectedBlackKing = board.pieces[14].count > 0
        ? static_cast<std::uint8_t>(board.pieces[14].front()) : 255;
    if (board.positionCore.kingSquare[White] != expectedWhiteKing)
        Mismatch(board, move, operation, "white king square");
    if (board.positionCore.kingSquare[Black] != expectedBlackKing)
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
