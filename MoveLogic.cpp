#include "QSearcher.h"
#include "PVSSearch.h"
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "MoveLogic.h"
#include "Move.h"
#include "Board.h"
#include "Search.h"
#include "Option.h"
#include "PieceMoves.h"
#include "AttackPlaces.h"
#include "GameLogic.h"
#include "BoardLogic.h"
#include "MissingInfoAboutPrevStateFromMove.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>

namespace
{
thread_local bool movePoolEnabled = false;

union alignas(Move) MovePoolSlot
{
    std::byte storage[sizeof(Move)];
    MovePoolSlot* next;
};

class MovePool
{
public:
    MovePool()
    {
        for (std::size_t i = 0; i + 1 < slots.size(); ++i)
            slots[i].next = &slots[i + 1];
        slots.back().next = nullptr;
        freeList = &slots.front();
    }

    void* Allocate()
    {
        if (freeList == nullptr)
            return nullptr;
        MovePoolSlot* slot = freeList;
        freeList = slot->next;
        return slot->storage;
    }

    bool Owns(void* pointer) const
    {
        const auto address = reinterpret_cast<std::uintptr_t>(pointer);
        const auto begin = reinterpret_cast<std::uintptr_t>(slots.data());
        const auto end = begin + sizeof(slots);
        return address >= begin && address < end &&
            (address - begin) % sizeof(MovePoolSlot) == 0;
    }

    void Deallocate(void* pointer)
    {
        auto* slot = reinterpret_cast<MovePoolSlot*>(pointer);
        slot->next = freeList;
        freeList = slot;
    }

private:
    static constexpr std::size_t Capacity = 4096;
    std::array<MovePoolSlot, Capacity> slots{};
    MovePoolSlot* freeList = nullptr;
};

thread_local MovePool movePool;

}

void* Move::operator new(std::size_t size)
{
    if (!movePoolEnabled)
        return ::operator new(size);
    void* pointer = size == sizeof(Move) ? movePool.Allocate() : nullptr;
    if (pointer == nullptr)
        pointer = ::operator new(size);
    return pointer;
}

void Move::operator delete(void* pointer) noexcept
{
    if (movePool.Owns(pointer))
    {
        movePool.Deallocate(pointer);
        return;
    }
    ::operator delete(pointer);
}

void Move::SetPoolEnabled(bool enabled) { movePoolEnabled = enabled; }

namespace
{

constexpr std::uint32_t PackedAttackerUnits[7] = {
    0,
    std::uint32_t{1} << 0,
    std::uint32_t{1} << 2,
    std::uint32_t{1} << 6,
    std::uint32_t{1} << 9,
    std::uint32_t{1} << 12,
    std::uint32_t{1} << 16
};

constexpr std::uint32_t PackedAttackerMasks[7] = {
    0,
    std::uint32_t{0x3} << 0,
    std::uint32_t{0xf} << 2,
    std::uint32_t{0x7} << 6,
    std::uint32_t{0x7} << 9,
    std::uint32_t{0xf} << 12,
    std::uint32_t{0x1} << 16
};

void AddPackedAttacker(std::uint32_t& attackers, int pieceType)
{
    attackers += PackedAttackerUnits[pieceType];
}

int PopLeastValuableAttacker(std::uint32_t& attackers)
{
    for (int pieceType = 1; pieceType <= 6; ++pieceType)
    {
        if ((attackers & PackedAttackerMasks[pieceType]) != 0)
        {
            attackers -= PackedAttackerUnits[pieceType];
            return pieceType;
        }
    }
    return 0;
}

bool IsSoleAttacker(std::uint32_t attackers, int pieceType)
{
    return attackers == PackedAttackerUnits[pieceType];
}

int NormalizeExchangePiece(int piece)
{
    return piece > 8 ? piece - 8 : piece;
}

bool PieceAttacksKing(int pieceType, bool whitePiece, int pieceSquare,
                      int kingSquare, long long occupancy)
{
    const long long kingBit = Option::PowerTwo[kingSquare];
    switch (pieceType)
    {
    case 1:
        return ((whitePiece ? AttackPlaces::WhitePawnAttackPlaces[pieceSquare]
                            : AttackPlaces::BlackPawnAttackPlaces[pieceSquare]) & kingBit) != 0;
    case 2:
        return (AttackPlaces::KnightAttackPlaces[pieceSquare] & kingBit) != 0;
    case 3:
        return (AttackPlaces::BishopAttack[pieceSquare][kingSquare] & occupancy) == kingBit;
    case 4:
        return (AttackPlaces::RookAttack[pieceSquare][kingSquare] & occupancy) == kingBit;
    case 5:
        return (AttackPlaces::QueenAttack[pieceSquare][kingSquare] & occupancy) == kingBit;
    case 6:
        return (AttackPlaces::KingAttackPlaces[pieceSquare] & kingBit) != 0;
    default:
        return false;
    }
}

#if defined(HOWL_MOVE_WOULD_GIVE_CHECK_VERIFY) && HOWL_MOVE_WOULD_GIVE_CHECK_VERIFY
bool LegacyMoveWouldGiveCheck(Board& board, const Move& move)
{
    const bool movingWhite = !board.sideToMove;
    const int enemyKingIndex = movingWhite ? 14 : 6;
    if (board.pieces[enemyKingIndex].count == 0)
        return false;

    const int enemyKingSquare = board.pieces[enemyKingIndex].front();
    long long occupancy = board.whitePieces | board.blackPieces;
    occupancy &= ~Option::PowerTwo[move.beginPlace];
    occupancy |= Option::PowerTwo[move.endPlace];

    if ((move.PublicFlag & Option::PowerTwo[6]) != 0)
    {
        const int capturedPawnSquare = move.endPlace + (movingWhite ? -8 : 8);
        occupancy &= ~Option::PowerTwo[capturedPawnSquare];
    }

    int rookFrom = -1;
    int rookTo = -1;
    if ((move.CastleFlag & Option::PowerTwo[3]) != 0)
    {
        rookFrom = move.beginPlace + 3;
        rookTo = move.beginPlace + 1;
    }
    else if ((move.CastleFlag & Option::PowerTwo[2]) != 0)
    {
        rookFrom = move.beginPlace - 4;
        rookTo = move.beginPlace - 1;
    }
    else if ((move.CastleFlag & Option::PowerTwo[1]) != 0)
    {
        rookFrom = move.beginPlace + 3;
        rookTo = move.beginPlace + 1;
    }
    else if ((move.CastleFlag & Option::PowerTwo[0]) != 0)
    {
        rookFrom = move.beginPlace - 4;
        rookTo = move.beginPlace - 1;
    }
    if (rookFrom >= 0)
    {
        occupancy &= ~Option::PowerTwo[rookFrom];
        occupancy |= Option::PowerTwo[rookTo];
    }

    const int pieceOffset = movingWhite ? 0 : 8;
    for (int pieceType = 1; pieceType <= 6; ++pieceType)
    {
        for (int originalSquare : board.pieces[pieceType + pieceOffset])
        {
            int virtualSquare = originalSquare;
            int virtualPieceType = pieceType;
            if (originalSquare == move.beginPlace)
            {
                virtualSquare = move.endPlace;
                if (move.promotionPiece > 0)
                    virtualPieceType = NormalizeExchangePiece(move.promotionPiece);
            }
            else if (pieceType == 4 && originalSquare == rookFrom)
            {
                virtualSquare = rookTo;
            }

            if (PieceAttacksKing(virtualPieceType, movingWhite, virtualSquare,
                                 enemyKingSquare, occupancy))
                return true;
        }
    }
    return false;
}
#endif

bool PositionCoreMoveWouldGiveCheck(const Board& board, const Move& move)
{
    const PositionCore& core = board.positionCore;
    const bool movingWhite = core.sideToMove == 0;
    const int movingColour = movingWhite ? 0 : 1;
    const int enemyColour = movingWhite ? 1 : 0;
    const int enemyKingSquare = core.kingSquare[enemyColour];
    if (enemyKingSquare >= 64)
        return false;

    Bitboard occupancy = core.colourOccupancy[0] | core.colourOccupancy[1];
    const Bitboard beginBit = static_cast<Bitboard>(Option::PowerTwo[move.beginPlace]);
    const Bitboard endBit = static_cast<Bitboard>(Option::PowerTwo[move.endPlace]);
    occupancy &= ~beginBit;
    occupancy |= endBit;

    if ((move.PublicFlag & Option::PowerTwo[6]) != 0)
    {
        const int capturedPawnSquare = move.endPlace + (movingWhite ? -8 : 8);
        occupancy &= ~static_cast<Bitboard>(Option::PowerTwo[capturedPawnSquare]);
    }

    int rookFrom = -1;
    int rookTo = -1;
    if ((move.CastleFlag & Option::PowerTwo[3]) != 0 ||
        (move.CastleFlag & Option::PowerTwo[1]) != 0)
    {
        rookFrom = move.beginPlace + 3;
        rookTo = move.beginPlace + 1;
    }
    else if ((move.CastleFlag & Option::PowerTwo[2]) != 0 ||
             (move.CastleFlag & Option::PowerTwo[0]) != 0)
    {
        rookFrom = move.beginPlace - 4;
        rookTo = move.beginPlace - 1;
    }
    if (rookFrom >= 0)
    {
        occupancy &= ~static_cast<Bitboard>(Option::PowerTwo[rookFrom]);
        occupancy |= static_cast<Bitboard>(Option::PowerTwo[rookTo]);
    }

    Bitboard piecesAfterMove[6];
    for (int pieceType = 0; pieceType < 6; ++pieceType)
    {
        piecesAfterMove[pieceType] =
            core.pieceOccupancy[pieceType] & core.colourOccupancy[movingColour];
    }

    const int movingPiece = core.pieceAt[move.beginPlace];
    const int movingPieceType = NormalizeExchangePiece(movingPiece);
    if (movingPieceType >= 1 && movingPieceType <= 6)
    {
        piecesAfterMove[movingPieceType - 1] &= ~beginBit;
        const int destinationPieceType = move.promotionPiece > 0
            ? NormalizeExchangePiece(move.promotionPiece) : movingPieceType;
        piecesAfterMove[destinationPieceType - 1] |= endBit;
    }

    if (rookFrom >= 0)
    {
        piecesAfterMove[3] &=
            ~static_cast<Bitboard>(Option::PowerTwo[rookFrom]);
        piecesAfterMove[3] |=
            static_cast<Bitboard>(Option::PowerTwo[rookTo]);
    }

    for (int pieceType = 1; pieceType <= 6; ++pieceType)
    {
        Bitboard pieces = piecesAfterMove[pieceType - 1];
        while (pieces != 0)
        {
            const int square = __builtin_ctzll(pieces);
            pieces &= pieces - 1;
            if (PieceAttacksKing(pieceType, movingWhite, square,
                                 enemyKingSquare,
                                 static_cast<long long>(occupancy)))
                return true;
        }
    }
    return false;
}

#if defined(HOWL_MOVE_WOULD_GIVE_CHECK_VERIFY) && HOWL_MOVE_WOULD_GIVE_CHECK_VERIFY
[[noreturn]] void ReportMoveWouldGiveCheckMismatch(
    const Board& board, const Move& move, bool legacyResult,
    bool positionCoreResult)
{
    std::cerr << "MoveWouldGiveCheck mismatch: move="
              << static_cast<char>('a' + move.beginPlace % 8)
              << static_cast<char>('1' + move.beginPlace / 8)
              << static_cast<char>('a' + move.endPlace % 8)
              << static_cast<char>('1' + move.endPlace / 8)
              << " from=" << int(move.beginPlace)
              << " to=" << int(move.endPlace)
              << " movingPiece=" << int(board.positionCore.pieceAt[move.beginPlace])
              << " legacy=" << legacyResult
              << " positionCore=" << positionCoreResult << '\n';
    std::abort();
}
#endif

bool MoveWouldGiveCheck(Board& board, const Move& move)
{
    const bool positionCoreResult = PositionCoreMoveWouldGiveCheck(board, move);
#if defined(HOWL_MOVE_WOULD_GIVE_CHECK_VERIFY) && HOWL_MOVE_WOULD_GIVE_CHECK_VERIFY
    const bool legacyResult = LegacyMoveWouldGiveCheck(board, move);
    if (legacyResult != positionCoreResult)
        ReportMoveWouldGiveCheckMismatch(
            board, move, legacyResult, positionCoreResult);
#endif
    return positionCoreResult;
}

std::uint64_t MakeExchangeKey(std::uint32_t attacker, std::uint32_t defender,
                              int beginPiece, int endPiece, int promotionPiece)
{
    return static_cast<std::uint64_t>(attacker)
        | (static_cast<std::uint64_t>(defender) << 17)
        | (static_cast<std::uint64_t>(beginPiece) << 34)
        | (static_cast<std::uint64_t>(NormalizeExchangePiece(endPiece)) << 37)
        | (static_cast<std::uint64_t>(promotionPiece % 8) << 40);
}

}

bool MoveLogic::MoveGivesCheck(Board& thisBoard, const Move& move)
{
    return MoveWouldGiveCheck(thisBoard, move);
}

double MoveLogic::pieceValue[15];
int MoveLogic::pieceMoveStack[15];
ExchangeChessCache MoveLogic::ExchangeCache;
ExchangeChessCache MoveLogic::ExchangeCacheWithoutBeginPiece;

bool MoveLogic::initialized = false;

void MoveLogic::Initialize()
{
    if (!initialized)
    {
        pieceMoveStack[1] = 2;
        pieceMoveStack[2] = 3;
        pieceMoveStack[3] = 4;
        pieceMoveStack[4] = 5;
        pieceMoveStack[5] = 1;
        pieceMoveStack[6] = 6;
        pieceMoveStack[9] = 10;
        pieceMoveStack[10] = 11;
        pieceMoveStack[11] = 12;
        pieceMoveStack[12] = 13;
        pieceMoveStack[13] = 9;
        pieceMoveStack[14] = 14;
        pieceValue[0] = 0.0;
        pieceValue[1] = 1.0;
        pieceValue[2] = 3.5;
        pieceValue[3] = 3.5;
        pieceValue[4] = 5.5;
        pieceValue[5] = 9.75;
        pieceValue[6] = 25;
        pieceValue[9] = 1.0;
        pieceValue[10] = 3.5;
        pieceValue[11] = 3.5;
        pieceValue[12] = 5.5;
        pieceValue[13] = 9.75;
        pieceValue[14] = 25;
        initialized = true;
    }
}

MoveList MoveLogic::MoveGenerator(Board &thisBoard, int depth, int depthGone, bool onlyCapturesAndChecks, bool includeQuietChecks)
{
    AttackerState whiteAttacker = SetWhiteAttacker(thisBoard);
    AttackerState blackAttacker = SetBlackAttacker(thisBoard);
    return MoveGenerator(thisBoard, depth, depthGone, onlyCapturesAndChecks, true, whiteAttacker, blackAttacker, includeQuietChecks);
}

MoveList MoveLogic::MoveGenerator(Board &thisBoard, int depth, int depthGone, bool onlyCapturesAndChecks, bool scoreAndSort, const AttackerState& whiteAttacker, const AttackerState& blackAttacker, bool includeQuietChecks)
{
    MoveList moveList;
    MoveGeneratorInto(thisBoard, depth, depthGone, onlyCapturesAndChecks, scoreAndSort,
                      whiteAttacker, blackAttacker, moveList, includeQuietChecks);
    return moveList;
}

#if defined(HOWL_MOVE_GENERATOR_INTO_VERIFY) && HOWL_MOVE_GENERATOR_INTO_VERIFY
void MoveLogic::LegacyMoveGeneratorInto(Board &thisBoard, int depth, int depthGone, bool onlyCapturesAndChecks, bool scoreAndSort, const AttackerState& whiteAttacker, const AttackerState& blackAttacker, MoveList& moveList, bool includeQuietChecks)
{
    moveList.Clear();
    if (Search::moveCount == 14962)
    {
        int x = 1;
    }
    long long whitePieces = thisBoard.whitePieces;
    long long blackPieces = thisBoard.blackPieces;
    int *mainBoard = thisBoard.mainBoard;
    long long wholeBoard = whitePieces | blackPieces;

    auto castleSquaresSafe = [&](bool white, int start, int transit, int destination)
    {
        const AttackerState opponentAttacks = white
            ? SetBlackAttacker(thisBoard)
            : SetWhiteAttacker(thisBoard);
        return opponentAttacks.pieceCounts[start] == 0 &&
               opponentAttacks.pieceCounts[transit] == 0 &&
               opponentAttacks.pieceCounts[destination] == 0;
    };

    int enemyKingPos = -1;
    long long enemyKingBit = 0;
    long long friendlySliderRayMask = 0;
    if (onlyCapturesAndChecks && includeQuietChecks)
    {
        int enemyKingIndex = (!thisBoard.sideToMove ? 14 : 6);
        if (thisBoard.pieces[enemyKingIndex].count > 0)
        {
            enemyKingPos = thisBoard.pieces[enemyKingIndex].front();
            enemyKingBit = Option::PowerTwo[enemyKingPos];
            int offset = (!thisBoard.sideToMove ? 0 : 8);
            for (int p = 3; p <= 5; ++p)
            {
                for (int pos : thisBoard.pieces[p + offset])
                {
                    friendlySliderRayMask |= AttackPlaces::LineMask[pos][enemyKingPos];
                }
            }
        }
    }

    if (!thisBoard.sideToMove)
    {
        for (int pieceCounter = 1; pieceCounter < 7; pieceCounter++)
        {
            int piece = pieceMoveStack[pieceCounter];
            switch (piece)
            {
            case 1:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    if (PieceMoves::WhitePawnMoves[piecePosition][0] != nullptr)
                    {
                        int endPlace = piecePosition + 16;
                        if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || ((AttackPlaces::WhitePawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (whiteAttacker.pieceCounts[endPlace] != 0 || endPlace >= 40)))))
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][0]);
                            if ((PieceMoves::pawnTwoMove[piecePosition] & wholeBoard) == 0)
                            {
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                newMove->value = ExchangeWithoutBeginPiece(whiteAttacker.pieceCounts[newMove->endPlace], blackAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][1] != nullptr && (Option::PowerTwo[piecePosition + 8] & wholeBoard) == 0)
                    {
                        int endPlace = piecePosition + 8;
                        if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || ((AttackPlaces::WhitePawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (whiteAttacker.pieceCounts[endPlace] != 0 || endPlace >= 40)))))
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(whiteAttacker.pieceCounts[newMove->endPlace], blackAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][2] != nullptr && (Option::PowerTwo[piecePosition + 8] & wholeBoard) == 0)
                    {
                        for (int i = 2; i <= 5; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(whiteAttacker.pieceCounts[newMove->endPlace], blackAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 5 - (i - 2));
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][6] != nullptr && piecePosition + 7 == thisBoard.unpassentPlace)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][6]);
                        newMove->endPiece = 9;
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][7] != nullptr && piecePosition + 9 == thisBoard.unpassentPlace)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][7]);
                        newMove->endPiece = 9;
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][8] != nullptr && (Option::PowerTwo[piecePosition + 7] & blackPieces) != 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][8]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][9] != nullptr && (Option::PowerTwo[piecePosition + 7] & blackPieces) != 0)
                    {
                        for (int i = 9; i <= 12; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][13] != nullptr && (Option::PowerTwo[piecePosition + 9] & blackPieces) != 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][13]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][14] != nullptr && (Option::PowerTwo[piecePosition + 9] & blackPieces) != 0)
                    {
                        for (int i = 14; i <= 17; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                }
                break;
            case 2:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace = piecePosition + 17;
                    if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][0]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 10;
                    if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][2]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 15;
                    if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][4]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 6;
                    if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][6]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 10;
                    if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][8]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 17;
                    if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][10]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 15;
                    if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][12]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 6;
                    if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][14]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                }
                break;
            case 3:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 4:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 5:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][8][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][9][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][10][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][11][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][12][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][13][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][14][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][15][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 6:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace;
                    endPlace = piecePosition + 7;
                    if (PieceMoves::WhiteKingMoves[piecePosition][0] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][0]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 8;
                    if (PieceMoves::WhiteKingMoves[piecePosition][2] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][2]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 9;
                    if (PieceMoves::WhiteKingMoves[piecePosition][4] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][4]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 1;
                    if (PieceMoves::WhiteKingMoves[piecePosition][6] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][6]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 7;
                    if (PieceMoves::WhiteKingMoves[piecePosition][8] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][8]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 8;
                    if (PieceMoves::WhiteKingMoves[piecePosition][10] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][10]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 9;
                    if (PieceMoves::WhiteKingMoves[piecePosition][12] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][12]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 1;
                    if (PieceMoves::WhiteKingMoves[piecePosition][14] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][14]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    if (thisBoard.whiteSmallCastle && (!onlyCapturesAndChecks || (includeQuietChecks && (AttackPlaces::LineMask[5][enemyKingPos] != 0 || AttackPlaces::LineMask[6][enemyKingPos] != 0))) && castleSquaresSafe(true, 4, 5, 6) && mainBoard[5] == 0 && mainBoard[6] == 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][16]);
                        newMove->value = 50;
                    }
                    if (thisBoard.whiteBigCastle && (!onlyCapturesAndChecks || (includeQuietChecks && (AttackPlaces::LineMask[3][enemyKingPos] != 0 || AttackPlaces::LineMask[2][enemyKingPos] != 0))) && castleSquaresSafe(true, 4, 3, 2) && mainBoard[3] == 0 && mainBoard[2] == 0 && mainBoard[1] == 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][17]);
                        newMove->value = 50;
                    }
                }
                break;
            }
        }
    }
    else
    {
        for (int pieceCounter = 9; pieceCounter < 15; pieceCounter++)
        {
            int piece = pieceMoveStack[pieceCounter];
            switch (piece)
            {
            case 9:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int exchangeInPlace = -ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[piecePosition], whiteAttacker.pieceCounts[piecePosition], piecePosition, piece - 8, 0, 0);
                    if (PieceMoves::BlackPawnMoves[piecePosition][0] != nullptr)
                    {
                        int endPlace = piecePosition - 16;
                        if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || ((AttackPlaces::BlackPawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (blackAttacker.pieceCounts[endPlace] != 0 || endPlace <= 23)))))
                        {
                            if ((PieceMoves::pawnTwoMove[piecePosition] & wholeBoard) == 0)
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][0]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                newMove->value = ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[newMove->endPlace], whiteAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                            }
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][1] != nullptr && (Option::PowerTwo[piecePosition - 8] & wholeBoard) == 0)
                    {
                        int endPlace = piecePosition - 8;
                        if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || ((AttackPlaces::BlackPawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (blackAttacker.pieceCounts[endPlace] != 0 || endPlace <= 23)))))
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[newMove->endPlace], whiteAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][2] != nullptr && (Option::PowerTwo[piecePosition - 8] & wholeBoard) == 0)
                    {
                        for (int i = 2; i <= 5; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[newMove->endPlace], whiteAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 5 - (i - 2));
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][6] != nullptr && piecePosition - 7 == thisBoard.unpassentPlace)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][6]);
                        newMove->endPiece = 1;
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][7] != nullptr && piecePosition - 9 == thisBoard.unpassentPlace)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][7]);
                        newMove->endPiece = 1;
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][8] != nullptr && (Option::PowerTwo[piecePosition - 7] & whitePieces) != 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][8]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][9] != nullptr && (Option::PowerTwo[piecePosition - 7] & whitePieces) != 0)
                    {
                        for (int i = 9; i <= 12; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][13] != nullptr && (Option::PowerTwo[piecePosition - 9] & whitePieces) != 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][13]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][14] != nullptr && (Option::PowerTwo[piecePosition - 9] & whitePieces) != 0)
                    {
                        for (int i = 14; i <= 17; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                }
                break;
            case 10:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace = piecePosition + 17;
                    if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][0]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 10;
                    if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][2]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 15;
                    if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][4]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 6;
                    if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][6]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 10;
                    if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][8]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 17;
                    if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][10]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 15;
                    if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][12]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 6;
                    if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][14]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                }
                break;
            case 11:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 12:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 13:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][8][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][9][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][10][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][11][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][12][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][13][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][14][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][15][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 14:
                for (int piecePosition : thisBoard.pieces[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace;
                    endPlace = piecePosition + 7;
                    if (PieceMoves::BlackKingMoves[piecePosition][0] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][0]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 8;
                    if (PieceMoves::BlackKingMoves[piecePosition][2] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][2]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 9;
                    if (PieceMoves::BlackKingMoves[piecePosition][4] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][4]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 1;
                    if (PieceMoves::BlackKingMoves[piecePosition][6] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][6]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 7;
                    if (PieceMoves::BlackKingMoves[piecePosition][8] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][8]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 8;
                    if (PieceMoves::BlackKingMoves[piecePosition][10] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][10]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 9;
                    if (PieceMoves::BlackKingMoves[piecePosition][12] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][12]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 1;
                    if (PieceMoves::BlackKingMoves[piecePosition][14] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][14]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    if (thisBoard.blackSmallCastle && (!onlyCapturesAndChecks || (includeQuietChecks && (AttackPlaces::LineMask[61][enemyKingPos] != 0 || AttackPlaces::LineMask[62][enemyKingPos] != 0))) && castleSquaresSafe(false, 60, 61, 62) && mainBoard[61] == 0 && mainBoard[62] == 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][16]);
                        newMove->value = 25;
                    }
                    if (thisBoard.blackBigCastle && (!onlyCapturesAndChecks || (includeQuietChecks && (AttackPlaces::LineMask[59][enemyKingPos] != 0 || AttackPlaces::LineMask[58][enemyKingPos] != 0))) && castleSquaresSafe(false, 60, 59, 58) && mainBoard[59] == 0 && mainBoard[58] == 0 && mainBoard[57] == 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][17]);
                        newMove->value = 25;
                    }
                }
                break;
            }
        }
    }
    // --- END REPLACEMENT OF VECTOR USAGE ---

    if (!onlyCapturesAndChecks && scoreAndSort)
    {
        for (int i = 0; i < moveList.count; ++i)
            moveList[i].givesCheck = MoveWouldGiveCheck(thisBoard, moveList[i]);
    }

    if (scoreAndSort)
    {
        ScoreAndSortMoves(thisBoard, moveList, depth, depthGone, whiteAttacker, blackAttacker);
    }
}


#endif

void MoveLogic::PositionCoreMoveGeneratorInto(Board &thisBoard, int depth, int depthGone, bool onlyCapturesAndChecks, bool scoreAndSort, const AttackerState& whiteAttacker, const AttackerState& blackAttacker, MoveList& moveList, bool includeQuietChecks)
{
    moveList.Clear();
    if (Search::moveCount == 14962)
    {
        int x = 1;
    }

    const PositionCore& core = thisBoard.positionCore;
    MyList positionCorePieceLists[15];
    int positionCoreMainBoard[64] = {};
    for (int piece = 0; piece < 15; ++piece)
    {
        for (int index = 0; index < core.pieceOrderCount[piece]; ++index)
        {
            const int square = core.pieceOrder[piece][index];
            positionCorePieceLists[piece].push_back(square);
        }
    }
    for (int square = 0; square < 64; ++square)
        positionCoreMainBoard[square] = core.pieceAt[square];

    long long whitePieces = static_cast<long long>(core.colourOccupancy[0]);
    long long blackPieces = static_cast<long long>(core.colourOccupancy[1]);
    int *mainBoard = positionCoreMainBoard;
    long long wholeBoard = whitePieces | blackPieces;

    auto castleSquaresSafe = [&](bool white, int start, int transit, int destination)
    {
        const AttackerState opponentAttacks = white
            ? SetBlackAttacker(thisBoard)
            : SetWhiteAttacker(thisBoard);
        return opponentAttacks.pieceCounts[start] == 0 &&
               opponentAttacks.pieceCounts[transit] == 0 &&
               opponentAttacks.pieceCounts[destination] == 0;
    };

    int enemyKingPos = -1;
    long long enemyKingBit = 0;
    long long friendlySliderRayMask = 0;
    if (onlyCapturesAndChecks && includeQuietChecks)
    {
        int enemyKingIndex = (!core.sideToMove ? 14 : 6);
        if (positionCorePieceLists[enemyKingIndex].count > 0)
        {
            enemyKingPos = positionCorePieceLists[enemyKingIndex].front();
            enemyKingBit = Option::PowerTwo[enemyKingPos];
            int offset = (!core.sideToMove ? 0 : 8);
            for (int p = 3; p <= 5; ++p)
            {
                for (int pos : positionCorePieceLists[p + offset])
                {
                    friendlySliderRayMask |= AttackPlaces::LineMask[pos][enemyKingPos];
                }
            }
        }
    }

    if (!core.sideToMove)
    {
        for (int pieceCounter = 1; pieceCounter < 7; pieceCounter++)
        {
            int piece = pieceMoveStack[pieceCounter];
            switch (piece)
            {
            case 1:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    if (PieceMoves::WhitePawnMoves[piecePosition][0] != nullptr)
                    {
                        int endPlace = piecePosition + 16;
                        if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || ((AttackPlaces::WhitePawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (whiteAttacker.pieceCounts[endPlace] != 0 || endPlace >= 40)))))
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][0]);
                            if ((PieceMoves::pawnTwoMove[piecePosition] & wholeBoard) == 0)
                            {
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                newMove->value = ExchangeWithoutBeginPiece(whiteAttacker.pieceCounts[newMove->endPlace], blackAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][1] != nullptr && (Option::PowerTwo[piecePosition + 8] & wholeBoard) == 0)
                    {
                        int endPlace = piecePosition + 8;
                        if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || ((AttackPlaces::WhitePawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (whiteAttacker.pieceCounts[endPlace] != 0 || endPlace >= 40)))))
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(whiteAttacker.pieceCounts[newMove->endPlace], blackAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][2] != nullptr && (Option::PowerTwo[piecePosition + 8] & wholeBoard) == 0)
                    {
                        for (int i = 2; i <= 5; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(whiteAttacker.pieceCounts[newMove->endPlace], blackAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 5 - (i - 2));
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][6] != nullptr && piecePosition + 7 == core.enPassantSquare)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][6]);
                        newMove->endPiece = 9;
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][7] != nullptr && piecePosition + 9 == core.enPassantSquare)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][7]);
                        newMove->endPiece = 9;
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][8] != nullptr && (Option::PowerTwo[piecePosition + 7] & blackPieces) != 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][8]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][9] != nullptr && (Option::PowerTwo[piecePosition + 7] & blackPieces) != 0)
                    {
                        for (int i = 9; i <= 12; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][13] != nullptr && (Option::PowerTwo[piecePosition + 9] & blackPieces) != 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][13]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                    }
                    if (PieceMoves::WhitePawnMoves[piecePosition][14] != nullptr && (Option::PowerTwo[piecePosition + 9] & blackPieces) != 0)
                    {
                        for (int i = 14; i <= 17; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhitePawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                }
                break;
            case 2:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace = piecePosition + 17;
                    if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][0]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 10;
                    if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][2]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 15;
                    if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][4]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 6;
                    if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][6]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 10;
                    if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][8]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 17;
                    if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][10]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 15;
                    if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][12]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 6;
                    if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][14]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                }
                break;
            case 3:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 4:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 5:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][8][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][9][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][10][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][11][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][12][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][13][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][14][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && blackAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & blackPieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][15][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 6:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace;
                    endPlace = piecePosition + 7;
                    if (PieceMoves::WhiteKingMoves[piecePosition][0] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][0]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 8;
                    if (PieceMoves::WhiteKingMoves[piecePosition][2] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][2]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 9;
                    if (PieceMoves::WhiteKingMoves[piecePosition][4] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][4]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 1;
                    if (PieceMoves::WhiteKingMoves[piecePosition][6] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][6]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 7;
                    if (PieceMoves::WhiteKingMoves[piecePosition][8] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][8]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 8;
                    if (PieceMoves::WhiteKingMoves[piecePosition][10] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][10]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 9;
                    if (PieceMoves::WhiteKingMoves[piecePosition][12] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][12]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 1;
                    if (PieceMoves::WhiteKingMoves[piecePosition][14] != nullptr && blackAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][14]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & blackPieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    if ((core.castlingRights & 1) && (!onlyCapturesAndChecks || (includeQuietChecks && (AttackPlaces::LineMask[5][enemyKingPos] != 0 || AttackPlaces::LineMask[6][enemyKingPos] != 0))) && castleSquaresSafe(true, 4, 5, 6) && mainBoard[5] == 0 && mainBoard[6] == 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][16]);
                        newMove->value = 50;
                    }
                    if ((core.castlingRights & 2) && (!onlyCapturesAndChecks || (includeQuietChecks && (AttackPlaces::LineMask[3][enemyKingPos] != 0 || AttackPlaces::LineMask[2][enemyKingPos] != 0))) && castleSquaresSafe(true, 4, 3, 2) && mainBoard[3] == 0 && mainBoard[2] == 0 && mainBoard[1] == 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::WhiteKingMoves[piecePosition][17]);
                        newMove->value = 50;
                    }
                }
                break;
            }
        }
    }
    else
    {
        for (int pieceCounter = 9; pieceCounter < 15; pieceCounter++)
        {
            int piece = pieceMoveStack[pieceCounter];
            switch (piece)
            {
            case 9:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int exchangeInPlace = -ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[piecePosition], whiteAttacker.pieceCounts[piecePosition], piecePosition, piece - 8, 0, 0);
                    if (PieceMoves::BlackPawnMoves[piecePosition][0] != nullptr)
                    {
                        int endPlace = piecePosition - 16;
                        if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || ((AttackPlaces::BlackPawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (blackAttacker.pieceCounts[endPlace] != 0 || endPlace <= 23)))))
                        {
                            if ((PieceMoves::pawnTwoMove[piecePosition] & wholeBoard) == 0)
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][0]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                                newMove->value = ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[newMove->endPlace], whiteAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                            }
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][1] != nullptr && (Option::PowerTwo[piecePosition - 8] & wholeBoard) == 0)
                    {
                        int endPlace = piecePosition - 8;
                        if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || ((AttackPlaces::BlackPawnAttackPlaces[endPlace] & enemyKingBit) != 0 && (blackAttacker.pieceCounts[endPlace] != 0 || endPlace <= 23)))))
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[newMove->endPlace], whiteAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 0);
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][2] != nullptr && (Option::PowerTwo[piecePosition - 8] & wholeBoard) == 0)
                    {
                        for (int i = 2; i <= 5; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            newMove->value = ExchangeWithoutBeginPiece(blackAttacker.pieceCounts[newMove->endPlace], whiteAttacker.pieceCounts[newMove->endPlace], newMove->endPlace, 1, mainBoard[newMove->endPlace], 5 - (i - 2));
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][6] != nullptr && piecePosition - 7 == core.enPassantSquare)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][6]);
                        newMove->endPiece = 1;
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][7] != nullptr && piecePosition - 9 == core.enPassantSquare)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][7]);
                        newMove->endPiece = 1;
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][8] != nullptr && (Option::PowerTwo[piecePosition - 7] & whitePieces) != 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][8]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][9] != nullptr && (Option::PowerTwo[piecePosition - 7] & whitePieces) != 0)
                    {
                        for (int i = 9; i <= 12; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][13] != nullptr && (Option::PowerTwo[piecePosition - 9] & whitePieces) != 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][13]);
                        newMove->endPiece = mainBoard[newMove->endPlace];
                    }
                    if (PieceMoves::BlackPawnMoves[piecePosition][14] != nullptr && (Option::PowerTwo[piecePosition - 9] & whitePieces) != 0)
                    {
                        for (int i = 14; i <= 17; i++)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackPawnMoves[piecePosition][i]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                }
                break;
            case 10:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace = piecePosition + 17;
                    if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][0]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 10;
                    if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][2]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 15;
                    if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][4]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 6;
                    if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][6]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 10;
                    if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][8]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 17;
                    if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][10]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 15;
                    if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][12]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 6;
                    if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::KnightAttackPlaces[endPlace] & enemyKingBit) != 0)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][14]);
                                newMove->endPiece = mainBoard[newMove->endPlace];
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::KnightMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                }
                break;
            case 11:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::BishopMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 12:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::RookMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 13:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][0][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][1][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][2][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][3][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][4][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][5][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][6][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][7][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][8][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][9][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][10][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][11][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][12][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][13][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                    for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][14][counter]);
                        if ((Option::PowerTwo[newMove->endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay || (AttackPlaces::LineMask[newMove->endPlace][enemyKingPos] != 0 && whiteAttacker.pieceCounts[newMove->endPlace] != 0))))
                            {
                            }
                            else
                            {
                                moveList.Discard(newMove);
                                newMove = nullptr;
                            }
                        }
                        else if ((Option::PowerTwo[newMove->endPlace] & whitePieces) != 0)
                        {
                            moveList.Discard(newMove);
                            newMove = moveList.AppendCopy(PieceMoves::QueenMoves[piecePosition][15][counter]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                            break;
                        }
                        else
                        {
                            moveList.Discard(newMove);
                            newMove = nullptr;
                            break;
                        }
                    }
                }
                break;
            case 14:
                for (int piecePosition : positionCorePieceLists[piece])
                {
                    bool srcOnRay = (Option::PowerTwo[piecePosition] & friendlySliderRayMask) != 0;
                    int endPlace;
                    endPlace = piecePosition + 7;
                    if (PieceMoves::BlackKingMoves[piecePosition][0] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][0]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][1]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 8;
                    if (PieceMoves::BlackKingMoves[piecePosition][2] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][2]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][3]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 9;
                    if (PieceMoves::BlackKingMoves[piecePosition][4] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][4]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][5]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition + 1;
                    if (PieceMoves::BlackKingMoves[piecePosition][6] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][6]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][7]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 7;
                    if (PieceMoves::BlackKingMoves[piecePosition][8] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][8]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][9]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 8;
                    if (PieceMoves::BlackKingMoves[piecePosition][10] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][10]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][11]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 9;
                    if (PieceMoves::BlackKingMoves[piecePosition][12] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][12]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][13]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    endPlace = piecePosition - 1;
                    if (PieceMoves::BlackKingMoves[piecePosition][14] != nullptr && whiteAttacker.pieceCounts[endPlace] == 0)
                    {
                        if ((Option::PowerTwo[endPlace] & wholeBoard) == 0)
                        {
                            if (!onlyCapturesAndChecks || (includeQuietChecks && (srcOnRay)))
                            {
                                Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][14]);
                            }
                        }
                        else if ((Option::PowerTwo[endPlace] & whitePieces) != 0)
                        {
                            Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][15]);
                            newMove->endPiece = mainBoard[newMove->endPlace];
                        }
                    }
                    if ((core.castlingRights & 4) && (!onlyCapturesAndChecks || (includeQuietChecks && (AttackPlaces::LineMask[61][enemyKingPos] != 0 || AttackPlaces::LineMask[62][enemyKingPos] != 0))) && castleSquaresSafe(false, 60, 61, 62) && mainBoard[61] == 0 && mainBoard[62] == 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][16]);
                        newMove->value = 25;
                    }
                    if ((core.castlingRights & 8) && (!onlyCapturesAndChecks || (includeQuietChecks && (AttackPlaces::LineMask[59][enemyKingPos] != 0 || AttackPlaces::LineMask[58][enemyKingPos] != 0))) && castleSquaresSafe(false, 60, 59, 58) && mainBoard[59] == 0 && mainBoard[58] == 0 && mainBoard[57] == 0)
                    {
                        Move *newMove = moveList.AppendCopy(PieceMoves::BlackKingMoves[piecePosition][17]);
                        newMove->value = 25;
                    }
                }
                break;
            }
        }
    }
    // --- END REPLACEMENT OF VECTOR USAGE ---

    if (!onlyCapturesAndChecks && scoreAndSort)
    {
        for (int i = 0; i < moveList.count; ++i)
            moveList[i].givesCheck = MoveWouldGiveCheck(thisBoard, moveList[i]);
    }

    if (scoreAndSort)
    {
        ScoreAndSortMoves(thisBoard, moveList, depth, depthGone, whiteAttacker, blackAttacker);
    }
}


#if defined(HOWL_MOVE_GENERATOR_INTO_VERIFY) && HOWL_MOVE_GENERATOR_INTO_VERIFY
bool SameGeneratedMove(const Move& left, const Move& right)
{
    return left.beginPlace == right.beginPlace &&
           left.endPlace == right.endPlace &&
           left.endPiece == right.endPiece &&
           left.promotionPiece == right.promotionPiece &&
           left.CastleFlag == right.CastleFlag &&
           left.PublicFlag == right.PublicFlag &&
           left.unpassentPlace == right.unpassentPlace &&
           left.givesCheck == right.givesCheck &&
           left.givesCheckComputed == right.givesCheckComputed &&
           left.isRefuteWithoutNullMove == right.isRefuteWithoutNullMove &&
           left.pad == right.pad &&
           left.value == right.value;
}

[[noreturn]] void ReportMoveGeneratorMismatch(
    const Board& board, int depth, int depthGone,
    bool onlyCapturesAndChecks, bool scoreAndSort, bool includeQuietChecks,
    const MoveList& legacyList, const MoveList& positionCoreList, int index)
{
    const Move* legacyMove = index < legacyList.count ? &legacyList[index] : nullptr;
    const Move* positionCoreMove =
        index < positionCoreList.count ? &positionCoreList[index] : nullptr;
    std::cerr << "MoveGeneratorInto mismatch: hash=" << board.ZobristHashCode
              << " side=" << int(board.positionCore.sideToMove)
              << " depth=" << depth
              << " depthGone=" << depthGone
              << " onlyCapturesAndChecks=" << onlyCapturesAndChecks
              << " scoreAndSort=" << scoreAndSort
              << " includeQuietChecks=" << includeQuietChecks
              << " index=" << index
              << " legacyCount=" << legacyList.count
              << " positionCoreCount=" << positionCoreList.count << '\n';
    if (legacyMove != nullptr)
        std::cerr << " legacy=" << int(legacyMove->beginPlace) << '-' << int(legacyMove->endPlace)
                  << " endPiece=" << int(legacyMove->endPiece)
                  << " promotion=" << int(legacyMove->promotionPiece)
                  << " publicFlags=" << int(static_cast<unsigned char>(legacyMove->PublicFlag))
                  << " castleFlags=" << int(static_cast<unsigned char>(legacyMove->CastleFlag))
                  << " value=" << legacyMove->value << '\n';
    if (positionCoreMove != nullptr)
        std::cerr << " positionCore=" << int(positionCoreMove->beginPlace) << '-' << int(positionCoreMove->endPlace)
                  << " endPiece=" << int(positionCoreMove->endPiece)
                  << " promotion=" << int(positionCoreMove->promotionPiece)
                  << " publicFlags=" << int(static_cast<unsigned char>(positionCoreMove->PublicFlag))
                  << " castleFlags=" << int(static_cast<unsigned char>(positionCoreMove->CastleFlag))
                  << " value=" << positionCoreMove->value << '\n';
    std::abort();
}
#endif

void MoveLogic::MoveGeneratorInto(Board &thisBoard, int depth, int depthGone,
                                  bool onlyCapturesAndChecks, bool scoreAndSort,
                                  const AttackerState& whiteAttacker,
                                  const AttackerState& blackAttacker,
                                  MoveList& moveList, bool includeQuietChecks)
{
#if defined(HOWL_MOVE_GENERATOR_INTO_VERIFY) && HOWL_MOVE_GENERATOR_INTO_VERIFY
    MoveList legacyList;
    MoveList positionCoreList;
    LegacyMoveGeneratorInto(thisBoard, depth, depthGone, onlyCapturesAndChecks,
                            scoreAndSort, whiteAttacker, blackAttacker,
                            legacyList, includeQuietChecks);
    PositionCoreMoveGeneratorInto(thisBoard, depth, depthGone, onlyCapturesAndChecks,
                                  scoreAndSort, whiteAttacker, blackAttacker,
                                  positionCoreList, includeQuietChecks);
    if (legacyList.count != positionCoreList.count)
        ReportMoveGeneratorMismatch(thisBoard, depth, depthGone,
                                    onlyCapturesAndChecks, scoreAndSort,
                                    includeQuietChecks, legacyList,
                                    positionCoreList, 0);
    for (int i = 0; i < legacyList.count; ++i)
        if (!SameGeneratedMove(legacyList[i], positionCoreList[i]))
            ReportMoveGeneratorMismatch(thisBoard, depth, depthGone,
                                        onlyCapturesAndChecks, scoreAndSort,
                                        includeQuietChecks, legacyList,
                                        positionCoreList, i);
    moveList = positionCoreList;
#else
    PositionCoreMoveGeneratorInto(thisBoard, depth, depthGone, onlyCapturesAndChecks,
                                  scoreAndSort, whiteAttacker, blackAttacker,
                                  moveList, includeQuietChecks);
#endif
}

void MoveLogic::ScoreAndSortMoves(Board& thisBoard, MoveList& moveList, int depth, int depthGone, const AttackerState& whiteAttacker, const AttackerState& blackAttacker)
{
    for (int i = 0; i < moveList.count; ++i)
    {
        Move& move = moveList[i];
        ScoreMove(thisBoard, move, whiteAttacker, blackAttacker);
    }
    std::sort(moveList.begin(), moveList.end(), [](const Move& a, const Move& b)
              { return b.value < a.value; });
}

void MoveLogic::ScoreMove(Board& thisBoard, Move& move, const AttackerState& whiteAttacker, const AttackerState& blackAttacker)
{
    constexpr int state = 0;
    int* mainBoard = thisBoard.mainBoard;
    if (thisBoard.sideToMove)
    {
        int beginPiece = mainBoard[move.beginPlace] % 8;
        move.value = MoveLogic::Exchange(blackAttacker.pieceCounts[move.endPlace], whiteAttacker.pieceCounts[move.endPlace], move.endPlace, beginPiece, move.endPiece, move.promotionPiece);
        move.value += whiteAttacker.orderingScores[move.beginPlace] + whiteAttacker.orderingScores[move.endPlace];
        move.value += Option::MoveOrderingValueBlack[state][beginPiece][move.endPlace];
    }
    else
    {
        int beginPiece = mainBoard[move.beginPlace];
        move.value = MoveLogic::Exchange(whiteAttacker.pieceCounts[move.endPlace], blackAttacker.pieceCounts[move.endPlace], move.endPlace, beginPiece, move.endPiece % 8, move.promotionPiece);
        move.value += blackAttacker.orderingScores[move.beginPlace] + blackAttacker.orderingScores[move.endPlace];
        move.value += Option::MoveOrderingValueWhite[state][beginPiece][move.endPlace];
    }
}

namespace
{
constexpr int SeeValue[16] = {
    0, 100, 350, 350, 550, 975, 2500, 0,
    0, 100, 350, 350, 550, 975, 2500, 0
};

inline int SafeSeeValue(int piece)
{
    const int norm = (piece >= 0 && piece < 16) ? NormalizeExchangePiece(piece) : 0;
    return (norm >= 0 && norm <= 6) ? SeeValue[norm] : 0;
}

inline bool SeeAttacks(int piece, int from, int target, std::uint64_t occupancy)
{
    const bool white = piece < 8;
    const int type = NormalizeExchangePiece(piece);
    const std::uint64_t targetBit = Option::PowerTwo[target];
    switch (type)
    {
    case 1: return ((white ? AttackPlaces::WhitePawnAttackPlaces[from]
                           : AttackPlaces::BlackPawnAttackPlaces[from]) & targetBit) != 0;
    case 2: return (AttackPlaces::KnightAttackPlaces[from] & targetBit) != 0;
    case 3:
        if (!(AttackPlaces::BishopPseudoAttacks[from] & targetBit)) return false;
        return (AttackPlaces::BishopAttack[from][target] & occupancy) == targetBit;
    case 4:
        if (!(AttackPlaces::RookPseudoAttacks[from] & targetBit)) return false;
        return (AttackPlaces::RookAttack[from][target] & occupancy) == targetBit;
    case 5:
        if (!(AttackPlaces::QueenPseudoAttacks[from] & targetBit)) return false;
        return (AttackPlaces::QueenAttack[from][target] & occupancy) == targetBit;
    case 6: return (AttackPlaces::KingAttackPlaces[from] & targetBit) != 0;
    default: return false;
    }
}

struct SeePosition
{
    int pieces[64]{};
    std::uint64_t pieceBoards[15]{};
    std::uint64_t occupancy = 0;
    int kingSquare[2] = {-1, -1};

    inline void remove(int piece, int square)
    {
        if (!piece) return;
        pieceBoards[piece] &= ~Option::PowerTwo[square];
        pieces[square] = 0;
        occupancy &= ~Option::PowerTwo[square];
    }

    inline void add(int piece, int square)
    {
        pieceBoards[piece] |= Option::PowerTwo[square];
        pieces[square] = piece;
        occupancy |= Option::PowerTwo[square];
    }
};

bool SeeSquareAttacked(const SeePosition& position, int square, bool byWhite)
{
    const int offset = byWhite ? 0 : 8;
    for (int type = 1; type <= 6; ++type)
    {
        const int piece = offset + type;
        std::uint64_t candidates = position.pieceBoards[piece];
        while (candidates)
        {
            const int from = __builtin_ctzll(candidates);
            candidates &= candidates - 1;
            if (SeeAttacks(piece, from, square, position.occupancy))
                return true;
        }
    }
    return false;
}


int SeeExchangeKernel(SeePosition& position, int target, bool white)
{
    int best = 0;
    const int capturedPiece = position.pieces[target];
    const int victimValue = SafeSeeValue(capturedPiece);
    const int offset = white ? 0 : 8;
    const std::uint64_t occ = position.occupancy;

    for (int attackerType = 1; attackerType <= 6; ++attackerType)
    {
        const int attacker = offset + attackerType;
        std::uint64_t candidates = position.pieceBoards[attacker];
        if (!candidates)
            continue;

        // Quick geometric pre-filtering of candidates
        if (attackerType == 1)
        {
            candidates &= (white ? AttackPlaces::BlackPawnAttackPlaces[target]
                                 : AttackPlaces::WhitePawnAttackPlaces[target]);
        }
        else if (attackerType == 2)
        {
            candidates &= AttackPlaces::KnightAttackPlaces[target];
        }
        else if (attackerType == 3)
        {
            candidates &= AttackPlaces::BishopPseudoAttacks[target];
        }
        else if (attackerType == 4)
        {
            candidates &= AttackPlaces::RookPseudoAttacks[target];
        }
        else if (attackerType == 5)
        {
            candidates &= AttackPlaces::QueenPseudoAttacks[target];
        }
        else if (attackerType == 6)
        {
            candidates &= AttackPlaces::KingAttackPlaces[target];
        }

        while (candidates)
        {
            const int from = __builtin_ctzll(candidates);
            candidates &= candidates - 1;
            if (!SeeAttacks(attacker, from, target, occ))
                continue;
            const bool promotes = attackerType == 1 &&
                ((white && target >= 56) || (!white && target < 8));
            const int firstPromotion = promotes ? 2 : attackerType;
            const int lastPromotion = promotes ? 5 : attackerType;
            for (int resultType = firstPromotion; resultType <= lastPromotion; ++resultType)
            {
                const int resultPiece = offset + resultType;
                const int oldKingSquare = position.kingSquare[white ? 0 : 1];
                position.remove(attacker, from);
                position.remove(capturedPiece, target);
                position.add(resultPiece, target);
                if (attackerType == 6)
                    position.kingSquare[white ? 0 : 1] = target;
                const int kingSquare = position.kingSquare[white ? 0 : 1];
                if (kingSquare >= 0 &&
                    !SeeSquareAttacked(position, kingSquare, !white))
                {
                    const int promotionGain = promotes
                        ? SeeValue[resultType] - SeeValue[1] : 0;
                    best = std::max(best, victimValue + promotionGain -
                                          SeeExchangeKernel(position, target, !white));
                }
                position.remove(resultPiece, target);
                position.add(capturedPiece, target);
                position.add(attacker, from);
                position.kingSquare[white ? 0 : 1] = oldKingSquare;
            }
        }
    }
    return best;
}
}

#if defined(HOWL_SEE_VERIFY) && HOWL_SEE_VERIFY
bool MoveLogic::LegacySEE_GE(Board& board, Move& move, int threshold)
{
    const bool white = !board.sideToMove;
    const bool enPassant = (move.PublicFlag & Option::PowerTwo[6]) != 0;
    const int movingPiece = board.mainBoard[move.beginPlace];
    const int capturedPiece = enPassant
        ? board.mainBoard[move.endPlace + (white ? -8 : 8)]
        : board.mainBoard[move.endPlace];
    const int capturedValue = SafeSeeValue(capturedPiece);
    const int promotedNorm = move.promotionPiece > 0
        ? NormalizeExchangePiece(move.promotionPiece) : NormalizeExchangePiece(movingPiece);
    const int promotionGain = (move.promotionPiece > 0)
        ? (SeeValue[promotedNorm] - SeeValue[1]) : 0;
    const int initialGain = capturedValue + promotionGain;

    // Early threshold cutoff: if even capturing without any counter-captures cannot reach threshold,
    // we can immediately reject without building SeePosition.
    if (initialGain < threshold)
        return false;

    SeePosition position;
    for (int p = 1; p <= 14; ++p)
    {
        for (int i = 0; i < board.pieces[p].count; ++i)
        {
            const int sq = board.pieces[p].data[i];
            position.add(p, sq);
        }
    }
    position.kingSquare[0] = board.pieces[6].count > 0 ? board.pieces[6].front() : -1;
    position.kingSquare[1] = board.pieces[14].count > 0 ? board.pieces[14].front() : -1;


    position.remove(movingPiece, move.beginPlace);
    if (enPassant)
    {
        const int capturedSquare = move.endPlace + (white ? -8 : 8);
        position.remove(capturedPiece, capturedSquare);
    }
    else
        position.remove(capturedPiece, move.endPlace);
    const int resultPiece = white ? promotedNorm : promotedNorm + 8;
    position.add(resultPiece, move.endPlace);
    if (NormalizeExchangePiece(movingPiece) == 6)
        position.kingSquare[white ? 0 : 1] = move.endPlace;
    const int kingSquare = position.kingSquare[white ? 0 : 1];
    if (kingSquare >= 0 && SeeSquareAttacked(position, kingSquare, !white))
        return false;
    const int exchange = SeeExchangeKernel(position, move.endPlace, !white);
    return initialGain - exchange >= threshold;
}
#endif

namespace
{
void InitializeSeePositionFromCore(SeePosition& position, const PositionCore& core)
{
    position = SeePosition{};
    position.occupancy = core.colourOccupancy[0] | core.colourOccupancy[1];
    for (int square = 0; square < 64; ++square)
        position.pieces[square] = core.pieceAt[square];
    for (int colour = 0; colour < 2; ++colour)
        for (int type = 1; type <= 6; ++type)
            position.pieceBoards[colour * 8 + type] =
                core.pieceOccupancy[type - 1] & core.colourOccupancy[colour];
    position.kingSquare[0] = core.kingSquare[0] < 64 ? core.kingSquare[0] : -1;
    position.kingSquare[1] = core.kingSquare[1] < 64 ? core.kingSquare[1] : -1;
}
}

bool MoveLogic::PositionCoreSEE_GE(Board& board, Move& move, int threshold)
{
    const PositionCore& core = board.positionCore;
    const bool white = core.sideToMove == 0;
    const bool enPassant = (move.PublicFlag & Option::PowerTwo[6]) != 0;
    const int movingPiece = core.pieceAt[move.beginPlace];
    const int capturedSquare = enPassant
        ? move.endPlace + (white ? -8 : 8) : move.endPlace;
    const int capturedPiece = core.pieceAt[capturedSquare];
    const int capturedValue = SafeSeeValue(capturedPiece);
    const int promotedNorm = move.promotionPiece > 0
        ? NormalizeExchangePiece(move.promotionPiece)
        : NormalizeExchangePiece(movingPiece);
    const int promotionGain = move.promotionPiece > 0
        ? SeeValue[promotedNorm] - SeeValue[1] : 0;
    const int initialGain = capturedValue + promotionGain;
    if (initialGain < threshold)
        return false;

    SeePosition position;
    InitializeSeePositionFromCore(position, core);
    position.remove(movingPiece, move.beginPlace);
    if (enPassant)
        position.remove(capturedPiece, capturedSquare);
    else
        position.remove(capturedPiece, move.endPlace);
    const int resultPiece = white ? promotedNorm : promotedNorm + 8;
    position.add(resultPiece, move.endPlace);
    if (NormalizeExchangePiece(movingPiece) == 6)
        position.kingSquare[white ? 0 : 1] = move.endPlace;
    const int kingSquare = position.kingSquare[white ? 0 : 1];
    if (kingSquare >= 0 && SeeSquareAttacked(position, kingSquare, !white))
        return false;
    const int exchange = SeeExchangeKernel(position, move.endPlace, !white);
    return initialGain - exchange >= threshold;
}

bool MoveLogic::SEE_GE(Board& board, Move& move, int threshold)
{
#if defined(HOWL_SEE_VERIFY) && HOWL_SEE_VERIFY
    const bool legacy = LegacySEE_GE(board, move, threshold);
    const bool core = PositionCoreSEE_GE(board, move, threshold);
    if (legacy != core)
    {
        std::cerr << "SEE_GE mismatch move=" << int(move.beginPlace) << '-'
                  << int(move.endPlace) << " threshold=" << threshold
                  << " legacy=" << legacy << " core=" << core << '\n';
        std::abort();
    }
    return core;
#else
    return PositionCoreSEE_GE(board, move, threshold);
#endif
}

MoveList MoveLogic::QSearchStage1Generator(Board &thisBoard, int depth, int depthGone, DeferredMove* deferredMoves, int& deferredCount, const Move& prevMove, bool includeQuietChecks, bool deepResolution)
{
    deferredCount = 0;
    AttackerState whiteAttacker = SetWhiteAttacker(thisBoard);
    AttackerState blackAttacker = SetBlackAttacker(thisBoard);
    MoveList fullList = MoveGenerator(thisBoard, depth, depthGone, true, false, whiteAttacker, blackAttacker, includeQuietChecks);
    const int* mainBoard = thisBoard.mainBoard;
    static const int pieceValue100[15] = {0, 100, 350, 350, 550, 975, 2500, 0, 0, 100, 350, 350, 550, 975, 2500};

    MoveList stage1List;
    for (int i = 0; i < fullList.count; ++i)
    {
        Move* m = &fullList[i];
        bool isPromotion = (m->promotionPiece > 0);
        bool isCapture = (m->endPiece > 0);

        if (isPromotion)
        {
            const int promoType = NormalizeExchangePiece(m->promotionPiece);
            m->givesCheck = MoveWouldGiveCheck(thisBoard, *m);

            if (promoType == 5 || m->givesCheck)
            {
                stage1List.AppendValue(*m);
            }
        }
        else if (isCapture)
        {
            int beginPiece = thisBoard.sideToMove ? (mainBoard[m->beginPlace] % 8) : mainBoard[m->beginPlace];
            int endPiece = thisBoard.sideToMove ? m->endPiece : (m->endPiece % 8);
            int exch = thisBoard.sideToMove
                ? MoveLogic::Exchange(blackAttacker.pieceCounts[m->endPlace], whiteAttacker.pieceCounts[m->endPlace], m->endPlace, beginPiece, endPiece, m->promotionPiece)
                : MoveLogic::Exchange(whiteAttacker.pieceCounts[m->endPlace], blackAttacker.pieceCounts[m->endPlace], m->endPlace, beginPiece, endPiece, m->promotionPiece);

            if (exch > 0)
            {
                stage1List.AppendValue(*m);
            }
            else if (exch == 0)
            {
                if (prevMove.endPlace >= 0 && m->endPlace == prevMove.endPlace)
                {
                    stage1List.AppendValue(*m);
                }
            }
        }
        else
        {
            if (includeQuietChecks)
            {
                DeferredMove dm;
                dm.templateMove = *m;
                dm.endPiece = 0;
                deferredMoves[deferredCount++] = dm;
            }
        }
    }

    ScoreAndSortMoves(thisBoard, stage1List, depth, depthGone, whiteAttacker, blackAttacker);

    return stage1List;
}

MoveList MoveLogic::MaterializeStage2(Board &thisBoard, int depth, int depthGone, const DeferredMove* deferredMoves, int deferredCount)
{
    MoveList stage2List;
    for (int i = 0; i < deferredCount; ++i)
        stage2List.AppendValue(deferredMoves[i].templateMove);

    AttackerState whiteAttacker = SetWhiteAttacker(thisBoard);
    AttackerState blackAttacker = SetBlackAttacker(thisBoard);
    ScoreAndSortMoves(thisBoard, stage2List, depth, depthGone, whiteAttacker, blackAttacker);

    return stage2List;
}

bool MoveLogic::HasAnyLegalMove(Board &thisBoard, const Move& prevMove, int depthGone)
{
    const bool side = thisBoard.sideToMove;
    const int turn = side ? 1 : 0;
    const int offset = side ? 8 : 0;
    const long long wholeBoard = thisBoard.whitePieces | thisBoard.blackPieces;
    const long long ownPieces = side ? thisBoard.blackPieces : thisBoard.whitePieces;
    const long long enemyPieces = side ? thisBoard.whitePieces : thisBoard.blackPieces;
    const int* mainBoard = thisBoard.mainBoard;

    auto isLegal = [&](Move& m) -> bool {
        MissingInfoAboutPrevStateFromMove undo(thisBoard, m);
        GameLogic::DoMove(thisBoard, m, const_cast<Move&>(prevMove), depthGone, depthGone, &undo);
        bool legal = !BoardLogic::UnderAttack(
            thisBoard, thisBoard.pieces[turn * 8 + 6].front(), thisBoard.sideToMove);
        GameLogic::UndoMove(thisBoard, m, undo);
        return legal;
    };

    for (int pieceCounter = 1; pieceCounter < 7; pieceCounter++)
    {
        int piece = pieceMoveStack[pieceCounter + (side ? 8 : 0)];
        switch (piece)
        {
        case 2:
        case 10:
        {
            for (int pos : thisBoard.pieces[piece])
            {
                for (int i = 0; i < 8; ++i)
                {
                    Move* tm = PieceMoves::KnightMoves[pos][i * 2];
                    if (!tm) continue;
                    int dest = tm->endPlace;
                    if ((Option::PowerTwo[dest] & wholeBoard) == 0)
                    {
                        Move m = *tm;
                        m.endPiece = 0;
                        if (isLegal(m)) return true;
                    }
                    else if ((Option::PowerTwo[dest] & enemyPieces) != 0)
                    {
                        Move m = *PieceMoves::KnightMoves[pos][i * 2 + 1];
                        m.endPiece = mainBoard[dest];
                        if (isLegal(m)) return true;
                    }
                }
            }
            break;
        }
        case 3:
        case 11:
        {
            for (int pos : thisBoard.pieces[piece])
            {
                for (int r = 0; r < 4; ++r)
                {
                    int quietRay = r * 2;
                    int capRay = r * 2 + 1;
                    const auto& quietVec = PieceMoves::BishopMoves[pos][quietRay];
                    const auto& capVec = PieceMoves::BishopMoves[pos][capRay];
                    for (size_t c = 0; c < quietVec.size(); ++c)
                    {
                        int dest = quietVec[c]->endPlace;
                        if ((Option::PowerTwo[dest] & wholeBoard) == 0)
                        {
                            Move m = *quietVec[c];
                            m.endPiece = 0;
                            if (isLegal(m)) return true;
                        }
                        else
                        {
                            if ((Option::PowerTwo[dest] & enemyPieces) != 0)
                            {
                                Move m = *capVec[c];
                                m.endPiece = mainBoard[dest];
                                if (isLegal(m)) return true;
                            }
                            break;
                        }
                    }
                }
            }
            break;
        }
        case 4:
        case 12:
        {
            for (int pos : thisBoard.pieces[piece])
            {
                for (int r = 0; r < 4; ++r)
                {
                    int quietRay = r * 2;
                    int capRay = r * 2 + 1;
                    const auto& quietVec = PieceMoves::RookMoves[pos][quietRay];
                    const auto& capVec = PieceMoves::RookMoves[pos][capRay];
                    for (size_t c = 0; c < quietVec.size(); ++c)
                    {
                        int dest = quietVec[c]->endPlace;
                        if ((Option::PowerTwo[dest] & wholeBoard) == 0)
                        {
                            Move m = *quietVec[c];
                            m.endPiece = 0;
                            if (isLegal(m)) return true;
                        }
                        else
                        {
                            if ((Option::PowerTwo[dest] & enemyPieces) != 0)
                            {
                                Move m = *capVec[c];
                                m.endPiece = mainBoard[dest];
                                if (isLegal(m)) return true;
                            }
                            break;
                        }
                    }
                }
            }
            break;
        }
        case 5:
        case 13:
        {
            for (int pos : thisBoard.pieces[piece])
            {
                for (int r = 0; r < 8; ++r)
                {
                    int quietRay = r * 2;
                    int capRay = r * 2 + 1;
                    const auto& quietVec = PieceMoves::QueenMoves[pos][quietRay];
                    const auto& capVec = PieceMoves::QueenMoves[pos][capRay];
                    for (size_t c = 0; c < quietVec.size(); ++c)
                    {
                        int dest = quietVec[c]->endPlace;
                        if ((Option::PowerTwo[dest] & wholeBoard) == 0)
                        {
                            Move m = *quietVec[c];
                            m.endPiece = 0;
                            if (isLegal(m)) return true;
                        }
                        else
                        {
                            if ((Option::PowerTwo[dest] & enemyPieces) != 0)
                            {
                                Move m = *capVec[c];
                                m.endPiece = mainBoard[dest];
                                if (isLegal(m)) return true;
                            }
                            break;
                        }
                    }
                }
            }
            break;
        }
        case 1:
        {
            for (int pos : thisBoard.pieces[1])
            {
                if (PieceMoves::WhitePawnMoves[pos][0] != nullptr && (PieceMoves::pawnTwoMove[pos] & wholeBoard) == 0)
                {
                    Move m = *PieceMoves::WhitePawnMoves[pos][0];
                    m.endPiece = 0;
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::WhitePawnMoves[pos][1] != nullptr && (Option::PowerTwo[pos + 8] & wholeBoard) == 0)
                {
                    Move m = *PieceMoves::WhitePawnMoves[pos][1];
                    m.endPiece = 0;
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::WhitePawnMoves[pos][2] != nullptr && (Option::PowerTwo[pos + 8] & wholeBoard) == 0)
                {
                    for (int i = 2; i <= 5; ++i)
                    {
                        Move m = *PieceMoves::WhitePawnMoves[pos][i];
                        m.endPiece = 0;
                        if (isLegal(m)) return true;
                    }
                }
                if (PieceMoves::WhitePawnMoves[pos][6] != nullptr && pos + 7 == thisBoard.unpassentPlace)
                {
                    Move m = *PieceMoves::WhitePawnMoves[pos][6];
                    m.endPiece = 9;
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::WhitePawnMoves[pos][7] != nullptr && pos + 9 == thisBoard.unpassentPlace)
                {
                    Move m = *PieceMoves::WhitePawnMoves[pos][7];
                    m.endPiece = 9;
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::WhitePawnMoves[pos][8] != nullptr && (Option::PowerTwo[pos + 7] & enemyPieces) != 0)
                {
                    Move m = *PieceMoves::WhitePawnMoves[pos][8];
                    m.endPiece = mainBoard[pos + 7];
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::WhitePawnMoves[pos][9] != nullptr && (Option::PowerTwo[pos + 7] & enemyPieces) != 0)
                {
                    for (int i = 9; i <= 12; ++i)
                    {
                        Move m = *PieceMoves::WhitePawnMoves[pos][i];
                        m.endPiece = mainBoard[pos + 7];
                        if (isLegal(m)) return true;
                    }
                }
                if (PieceMoves::WhitePawnMoves[pos][13] != nullptr && (Option::PowerTwo[pos + 9] & enemyPieces) != 0)
                {
                    Move m = *PieceMoves::WhitePawnMoves[pos][13];
                    m.endPiece = mainBoard[pos + 9];
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::WhitePawnMoves[pos][14] != nullptr && (Option::PowerTwo[pos + 9] & enemyPieces) != 0)
                {
                    for (int i = 14; i <= 17; ++i)
                    {
                        Move m = *PieceMoves::WhitePawnMoves[pos][i];
                        m.endPiece = mainBoard[pos + 9];
                        if (isLegal(m)) return true;
                    }
                }
            }
            break;
        }
        case 9:
        {
            for (int pos : thisBoard.pieces[9])
            {
                if (PieceMoves::BlackPawnMoves[pos][0] != nullptr && (PieceMoves::pawnTwoMove[pos] & wholeBoard) == 0)
                {
                    Move m = *PieceMoves::BlackPawnMoves[pos][0];
                    m.endPiece = 0;
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::BlackPawnMoves[pos][1] != nullptr && (Option::PowerTwo[pos - 8] & wholeBoard) == 0)
                {
                    Move m = *PieceMoves::BlackPawnMoves[pos][1];
                    m.endPiece = 0;
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::BlackPawnMoves[pos][2] != nullptr && (Option::PowerTwo[pos - 8] & wholeBoard) == 0)
                {
                    for (int i = 2; i <= 5; ++i)
                    {
                        Move m = *PieceMoves::BlackPawnMoves[pos][i];
                        m.endPiece = 0;
                        if (isLegal(m)) return true;
                    }
                }
                if (PieceMoves::BlackPawnMoves[pos][6] != nullptr && pos - 7 == thisBoard.unpassentPlace)
                {
                    Move m = *PieceMoves::BlackPawnMoves[pos][6];
                    m.endPiece = 1;
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::BlackPawnMoves[pos][7] != nullptr && pos - 9 == thisBoard.unpassentPlace)
                {
                    Move m = *PieceMoves::BlackPawnMoves[pos][7];
                    m.endPiece = 1;
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::BlackPawnMoves[pos][8] != nullptr && (Option::PowerTwo[pos - 7] & enemyPieces) != 0)
                {
                    Move m = *PieceMoves::BlackPawnMoves[pos][8];
                    m.endPiece = mainBoard[pos - 7];
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::BlackPawnMoves[pos][9] != nullptr && (Option::PowerTwo[pos - 7] & enemyPieces) != 0)
                {
                    for (int i = 9; i <= 12; ++i)
                    {
                        Move m = *PieceMoves::BlackPawnMoves[pos][i];
                        m.endPiece = mainBoard[pos - 7];
                        if (isLegal(m)) return true;
                    }
                }
                if (PieceMoves::BlackPawnMoves[pos][13] != nullptr && (Option::PowerTwo[pos - 9] & enemyPieces) != 0)
                {
                    Move m = *PieceMoves::BlackPawnMoves[pos][13];
                    m.endPiece = mainBoard[pos - 9];
                    if (isLegal(m)) return true;
                }
                if (PieceMoves::BlackPawnMoves[pos][14] != nullptr && (Option::PowerTwo[pos - 9] & enemyPieces) != 0)
                {
                    for (int i = 14; i <= 17; ++i)
                    {
                        Move m = *PieceMoves::BlackPawnMoves[pos][i];
                        m.endPiece = mainBoard[pos - 9];
                        if (isLegal(m)) return true;
                    }
                }
            }
            break;
        }
        case 6:
        case 14:
        {
            if (thisBoard.pieces[piece].count > 0)
            {
                int kingPos = thisBoard.pieces[piece].front();
                Move** kingMoves = side ? PieceMoves::BlackKingMoves[kingPos] : PieceMoves::WhiteKingMoves[kingPos];
                for (int i = 0; i < 8; ++i)
                {
                    Move* tm = kingMoves[i * 2];
                    if (!tm) continue;
                    int dest = tm->endPlace;
                    if ((Option::PowerTwo[dest] & wholeBoard) == 0)
                    {
                        Move m = *tm;
                        m.endPiece = 0;
                        if (isLegal(m)) return true;
                    }
                    else if ((Option::PowerTwo[dest] & enemyPieces) != 0)
                    {
                        Move m = *kingMoves[i * 2 + 1];
                        m.endPiece = mainBoard[dest];
                        if (isLegal(m)) return true;
                    }
                }
                if (!side)
                {
                    if (thisBoard.whiteSmallCastle && mainBoard[5] == 0 && mainBoard[6] == 0
                        && !BoardLogic::UnderAttack(thisBoard, 4, true)
                        && !BoardLogic::UnderAttack(thisBoard, 5, true)
                        && !BoardLogic::UnderAttack(thisBoard, 6, true))
                    {
                        Move m = *PieceMoves::WhiteKingMoves[kingPos][16];
                        m.endPiece = 0;
                        if (isLegal(m)) return true;
                    }
                    if (thisBoard.whiteBigCastle && mainBoard[3] == 0 && mainBoard[2] == 0 && mainBoard[1] == 0
                        && !BoardLogic::UnderAttack(thisBoard, 4, true)
                        && !BoardLogic::UnderAttack(thisBoard, 3, true)
                        && !BoardLogic::UnderAttack(thisBoard, 2, true))
                    {
                        Move m = *PieceMoves::WhiteKingMoves[kingPos][17];
                        m.endPiece = 0;
                        if (isLegal(m)) return true;
                    }
                }
                else
                {
                    if (thisBoard.blackSmallCastle && mainBoard[61] == 0 && mainBoard[62] == 0
                        && !BoardLogic::UnderAttack(thisBoard, 60, false)
                        && !BoardLogic::UnderAttack(thisBoard, 61, false)
                        && !BoardLogic::UnderAttack(thisBoard, 62, false))
                    {
                        Move m = *PieceMoves::BlackKingMoves[kingPos][16];
                        m.endPiece = 0;
                        if (isLegal(m)) return true;
                    }
                    if (thisBoard.blackBigCastle && mainBoard[59] == 0 && mainBoard[58] == 0 && mainBoard[57] == 0
                        && !BoardLogic::UnderAttack(thisBoard, 60, false)
                        && !BoardLogic::UnderAttack(thisBoard, 59, false)
                        && !BoardLogic::UnderAttack(thisBoard, 58, false))
                    {
                        Move m = *PieceMoves::BlackKingMoves[kingPos][17];
                        m.endPiece = 0;
                        if (isLegal(m)) return true;
                    }
                }
            }
            break;
        }
        }
    }
    return false;
}

// NOTE: You must also replace all Moves->push_back and ComplicatedMoves->push_back in the body with the array logic as described above.
// The rest of the function logic remains the same, just replace vector operations with array operations.

#if defined(HOWL_ATTACKER_VERIFY) && HOWL_ATTACKER_VERIFY
AttackerState MoveLogic::LegacySetWhiteAttacker(Board &thisBoard)
{
    AttackerState whiteAttacker;
    long long whitePieces = thisBoard.whitePieces;
    long long blackPieces = thisBoard.blackPieces;
    int *mainBoard = thisBoard.mainBoard;
    long long wholeBoard = whitePieces | blackPieces;

    for (int piece = 6; piece > 0; piece--)
    {
        switch (piece)
        {
        case 6:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                int endPlace = piecePosition + 7;
                if (PieceMoves::WhiteKingMoves[piecePosition][0] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition + 8;
                if (PieceMoves::WhiteKingMoves[piecePosition][2] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition + 9;
                if (PieceMoves::WhiteKingMoves[piecePosition][4] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition + 1;
                if (PieceMoves::WhiteKingMoves[piecePosition][6] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 7;
                if (PieceMoves::WhiteKingMoves[piecePosition][8] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 8;
                if (PieceMoves::WhiteKingMoves[piecePosition][10] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 9;
                if (PieceMoves::WhiteKingMoves[piecePosition][12] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }

                endPlace = piecePosition - 1;
                if (PieceMoves::WhiteKingMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 6);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
            }
            break;
        case 5:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][8][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][10][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][12][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][14][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 5);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;
        case 4:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 4);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;

        case 3:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }

                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(whiteAttacker.pieceCounts[endPos], 3);
                        whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        whiteAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;

        case 2:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                int endPlace = piecePosition + 17;
                if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 10;
                if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 15;
                if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 6;
                if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 10;
                if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 17;
                if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 15;
                if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 6;
                if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[endPlace], 2);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    whiteAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
            }
            break;

        case 1:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                if (PieceMoves::WhitePawnMoves[piecePosition][8] != nullptr || PieceMoves::WhitePawnMoves[piecePosition][9] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[piecePosition + 7], 1);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 7]];
                    whiteAttacker.orderingScores[piecePosition + 7] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 7]];
                }
                if (PieceMoves::WhitePawnMoves[piecePosition][13] != nullptr || PieceMoves::WhitePawnMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(whiteAttacker.pieceCounts[piecePosition + 9], 1);
                    whiteAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 9]];
                    whiteAttacker.orderingScores[piecePosition + 9] += Option::AttackValueMovement[piece][mainBoard[piecePosition + 9]];
                }
            }
            break;
        }
    }
    return whiteAttacker;
}

AttackerState MoveLogic::LegacySetBlackAttacker(Board &thisBoard)
{
    AttackerState blackAttacker;
    long long whitePieces = thisBoard.whitePieces;
    long long blackPieces = thisBoard.blackPieces;
    int *mainBoard = thisBoard.mainBoard;
    long long wholeBoard = whitePieces | blackPieces;

    for (int piece = 14; piece > 8; piece--)
    {
        switch (piece)
        {
        case 14:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                int endPlace = piecePosition + 7;
                if (PieceMoves::BlackKingMoves[piecePosition][0] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 8;
                if (PieceMoves::BlackKingMoves[piecePosition][2] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 9;
                if (PieceMoves::BlackKingMoves[piecePosition][4] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 1;
                if (PieceMoves::BlackKingMoves[piecePosition][6] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 7;
                if (PieceMoves::BlackKingMoves[piecePosition][8] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 8;
                if (PieceMoves::BlackKingMoves[piecePosition][10] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 9;
                if (PieceMoves::BlackKingMoves[piecePosition][12] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 1;
                if (PieceMoves::BlackKingMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 6);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
            }
            break;
        case 13:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][8].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][8][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][10].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][10][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][12].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][12][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::QueenMoves[piecePosition][14].size(); counter++)
                {
                    int endPos = PieceMoves::QueenMoves[piecePosition][14][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 5);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;
        case 12:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::RookMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::RookMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 4);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;
        case 11:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][0].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][0][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][2].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][2][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][4].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][4][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
                for (int counter = 0; counter < PieceMoves::BishopMoves[piecePosition][6].size(); counter++)
                {
                    int endPos = PieceMoves::BishopMoves[piecePosition][6][counter]->endPlace;
                    if ((Option::PowerTwo[endPos] & wholeBoard) == 0)
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                    }
                    else
                    {
                        AddPackedAttacker(blackAttacker.pieceCounts[endPos], 3);
                        blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        blackAttacker.orderingScores[endPos] += Option::AttackValueMovement[piece][mainBoard[endPos]];
                        break;
                    }
                }
            }
            break;
        case 10:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                int endPlace = piecePosition + 17;
                if (PieceMoves::KnightMoves[piecePosition][0] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 10;
                if (PieceMoves::KnightMoves[piecePosition][2] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 15;
                if (PieceMoves::KnightMoves[piecePosition][4] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition + 6;
                if (PieceMoves::KnightMoves[piecePosition][6] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 10;
                if (PieceMoves::KnightMoves[piecePosition][8] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 17;
                if (PieceMoves::KnightMoves[piecePosition][10] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 15;
                if (PieceMoves::KnightMoves[piecePosition][12] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
                endPlace = piecePosition - 6;
                if (PieceMoves::KnightMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[endPlace], 2);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                    blackAttacker.orderingScores[endPlace] += Option::AttackValueMovement[piece][mainBoard[endPlace]];
                }
            }
            break;
        case 9:
            for (int piecePosition : thisBoard.pieces[piece])
            {
                if (PieceMoves::BlackPawnMoves[piecePosition][8] != nullptr || PieceMoves::BlackPawnMoves[piecePosition][9] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[piecePosition - 7], 1);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 7]];
                    blackAttacker.orderingScores[piecePosition - 7] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 7]];
                }
                if (PieceMoves::BlackPawnMoves[piecePosition][13] != nullptr || PieceMoves::BlackPawnMoves[piecePosition][14] != nullptr)
                {
                    AddPackedAttacker(blackAttacker.pieceCounts[piecePosition - 9], 1);
                    blackAttacker.orderingScores[piecePosition] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 9]];
                    blackAttacker.orderingScores[piecePosition - 9] += Option::AttackValueMovement[piece][mainBoard[piecePosition - 9]];
                }
            }
            break;
        }
    }
    return blackAttacker;
}
#endif

namespace
{
AttackerState BuildPositionCoreAttacker(const Board& board, bool white)
{
    AttackerState result;
    const PositionCore& core = board.positionCore;
    const Bitboard occupancy = core.colourOccupancy[0] | core.colourOccupancy[1];
    const int offset = white ? 0 : 8;
    const int pieceCode[7] = {0, offset + 1, offset + 2, offset + 3,
                              offset + 4, offset + 5, offset + 6};

    auto add = [&](int from, int to, int type)
    {
        AddPackedAttacker(result.pieceCounts[to], type);
        const int piece = pieceCode[type];
        const int targetPiece = core.pieceAt[to];
        result.orderingScores[from] += Option::AttackValueMovement[piece][targetPiece];
        result.orderingScores[to] += Option::AttackValueMovement[piece][targetPiece];
    };

    for (int type = 6; type >= 1; --type)
    {
        Bitboard pieces = core.pieceOccupancy[type - 1] & core.colourOccupancy[white ? 0 : 1];
        while (pieces != 0)
        {
            const int from = __builtin_ctzll(pieces);
            pieces &= pieces - 1;
            const int piece = pieceCode[type];

            if (type == 6 || type == 2)
            {
                Move* const* moves = (type == 6)
                    ? (white ? PieceMoves::WhiteKingMoves[from]
                             : PieceMoves::BlackKingMoves[from])
                    : PieceMoves::KnightMoves[from];
                const int limit = type == 6 ? 16 : 16;
                for (int index = 0; index < limit; index += 2)
                    if (moves[index] != nullptr)
                        add(from, moves[index]->endPlace, type);
                continue;
            }

            if (type == 1)
            {
                Move* const* moves = white ? PieceMoves::WhitePawnMoves[from]
                                            : PieceMoves::BlackPawnMoves[from];
                for (int index : {8, 9, 13, 14})
                    if (moves[index] != nullptr)
                        add(from, moves[index]->endPlace, type);
                continue;
            }

            const std::vector<Move*>* rays = nullptr;
            const int rayCount = type == 5 ? 8 : 4;
            if (type == 3)
                rays = PieceMoves::BishopMoves[from];
            else if (type == 4)
                rays = PieceMoves::RookMoves[from];
            else
                rays = PieceMoves::QueenMoves[from];

            for (int direction = 0; direction < rayCount; ++direction)
            {
                for (Move* rayMove : rays[direction * 2])
                {
                    const int to = rayMove->endPlace;
                    add(from, to, type);
                    if ((occupancy & Option::PowerTwo[to]) != 0)
                        break;
                }
            }
        }
    }
    return result;
}
}

AttackerState MoveLogic::PositionCoreSetWhiteAttacker(Board& board)
{
    return BuildPositionCoreAttacker(board, true);
}

AttackerState MoveLogic::PositionCoreSetBlackAttacker(Board& board)
{
    return BuildPositionCoreAttacker(board, false);
}

AttackerState MoveLogic::SetWhiteAttacker(Board& board)
{
#if defined(HOWL_ATTACKER_VERIFY) && HOWL_ATTACKER_VERIFY
    const AttackerState legacy = LegacySetWhiteAttacker(board);
    const AttackerState core = PositionCoreSetWhiteAttacker(board);
    for (int square = 0; square < 64; ++square)
        if (legacy.pieceCounts[square] != core.pieceCounts[square] ||
            legacy.orderingScores[square] != core.orderingScores[square])
        {
            std::cerr << "SetWhiteAttacker mismatch square=" << square
                      << " legacyCount=" << legacy.pieceCounts[square]
                      << " coreCount=" << core.pieceCounts[square]
                      << " legacyScore=" << legacy.orderingScores[square]
                      << " coreScore=" << core.orderingScores[square] << '\n';
            std::abort();
        }
    return core;
#else
    return PositionCoreSetWhiteAttacker(board);
#endif
}

AttackerState MoveLogic::SetBlackAttacker(Board& board)
{
#if defined(HOWL_ATTACKER_VERIFY) && HOWL_ATTACKER_VERIFY
    const AttackerState legacy = LegacySetBlackAttacker(board);
    const AttackerState core = PositionCoreSetBlackAttacker(board);
    for (int square = 0; square < 64; ++square)
        if (legacy.pieceCounts[square] != core.pieceCounts[square] ||
            legacy.orderingScores[square] != core.orderingScores[square])
        {
            std::cerr << "SetBlackAttacker mismatch square=" << square
                      << " legacyCount=" << legacy.pieceCounts[square]
                      << " coreCount=" << core.pieceCounts[square]
                      << " legacyScore=" << legacy.orderingScores[square]
                      << " coreScore=" << core.orderingScores[square] << '\n';
            std::abort();
        }
    return core;
#else
    return PositionCoreSetBlackAttacker(board);
#endif
}

Move *MoveLogic::MoveCopy(Move *move)
{
    Move *newMove = new Move();
    newMove->beginPlace = move->beginPlace;
    newMove->CastleFlag = move->CastleFlag;
    newMove->endPlace = move->endPlace;
    newMove->promotionPiece = move->promotionPiece;
    newMove->PublicFlag = move->PublicFlag;
    newMove->unpassentPlace = move->unpassentPlace;
    newMove->givesCheck = false;
    newMove->givesCheckComputed = false;
    return newMove;
}

int MoveLogic::Exchange(std::uint32_t attacker, std::uint32_t defender, int attackPlace, int beginPiece, int endPiece, int promotionPiece)
{
    const std::uint64_t exchangeHash =
        MakeExchangeKey(attacker, defender, beginPiece, endPiece, promotionPiece);
    endPiece = NormalizeExchangePiece(endPiece);
    std::optional<int> exchangeSavedValue;
    exchangeSavedValue = ExchangeCache.getFromCache(exchangeHash);
    if (exchangeSavedValue.has_value())
    {
        return exchangeSavedValue.value();
    }
    else
    {
        if (endPiece == 6)
        {
            return 200;
        }
        int beginPieceTemp = beginPiece;
        double exchangeValue = 0;
        std::uint32_t attackerTemp = attacker;
        std::uint32_t defenderTemp = defender;
        if (promotionPiece != 0)
        {
            exchangeValue = pieceValue[promotionPiece] - 1;
        }
        exchangeValue += pieceValue[endPiece];
        double attackList[30]; int attackListCount = 0;
        double defendList[30]; int defendListCount = 0;
        attackList[attackListCount++] = exchangeValue;
        bool attackerRemove = false;
        while (true)
        {
            if (defenderTemp == 0)
            {
                defendList[defendListCount++] = exchangeValue;
                break;
            }
            else if (IsSoleAttacker(defenderTemp, 6) && attackerTemp > 0 &&
                     !IsSoleAttacker(attackerTemp, beginPiece))
            {
                defendList[defendListCount++] = exchangeValue;
                break;
            }
            else
            {
                endPiece = PopLeastValuableAttacker(defenderTemp);
            }
            exchangeValue -= pieceValue[beginPiece];
            defendList[defendListCount++] = exchangeValue;
            if (attackerTemp == 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else if (IsSoleAttacker(attackerTemp, 6) && defenderTemp > 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else
            {
                beginPiece = PopLeastValuableAttacker(attackerTemp);
                if (!attackerRemove && beginPiece == beginPieceTemp)
                {
                    attackerRemove = true;
                    if (attackerTemp == 0)
                    {
                        attackList[attackListCount++] = exchangeValue;
                        break;
                    }
                    else if (IsSoleAttacker(attackerTemp, 6) && defenderTemp > 0)
                    {
                        attackList[attackListCount++] = exchangeValue;
                        break;
                    }
                    else
                    {
                        beginPiece = PopLeastValuableAttacker(attackerTemp);
                    }
                }
                exchangeValue += pieceValue[endPiece];
                attackList[attackListCount++] = exchangeValue;
            }
        }
        int attackExchangePlace = 0;
        int defendExchangePlace = 0;
        double attackValue = 1000;
        double defendValue = -1000;

        for (int counter = 0; counter < attackListCount; ++counter)
        {
            if (attackList[counter] < attackValue)
            {
                attackExchangePlace = counter;
                attackValue = attackList[counter];
            }
        }

        for (int counter = 0; counter < defendListCount; ++counter)
        {
            if (defendList[counter] > defendValue)
            {
                defendExchangePlace = counter;
                defendValue = defendList[counter];
            }
        }

        int exchangeValueTemp;
        if (defendExchangePlace == attackExchangePlace)
        {
            exchangeValueTemp = static_cast<int>(attackList[attackExchangePlace] * 100);
        }
        else if (defendExchangePlace > attackExchangePlace)
        {
            exchangeValueTemp = static_cast<int>(attackList[attackExchangePlace] * 100);
        }
        else
        {
            exchangeValueTemp = static_cast<int>(defendList[defendExchangePlace] * 100);
        }

        ExchangeCache.addToCache(exchangeHash, exchangeValueTemp);
        return exchangeValueTemp;
    }
}

int MoveLogic::ExchangeWithoutBeginPiece(std::uint32_t attacker, std::uint32_t defender, int attackPlace, int beginPiece, int endPiece, int promotionPiece)
{
    const std::uint64_t exchangeHash =
        MakeExchangeKey(attacker, defender, beginPiece, endPiece, promotionPiece);
    endPiece = NormalizeExchangePiece(endPiece);
    std::optional<int> exchangeSavedValue;
    exchangeSavedValue = ExchangeCacheWithoutBeginPiece.getFromCache(exchangeHash);
    if (exchangeSavedValue.has_value())
    {
        return exchangeSavedValue.value();
    }
    else
    {
        if (endPiece == 6)
        {
            return 200;
        }

        double exchangeValue = 0;
        std::uint32_t attackerTemp = attacker;
        std::uint32_t defenderTemp = defender;
        if (promotionPiece != 0)
        {
            exchangeValue = pieceValue[promotionPiece] - 1;
        }
        exchangeValue += pieceValue[endPiece];
        double attackList[30]; int attackListCount = 0;
        double defendList[30]; int defendListCount = 0;
        attackList[attackListCount++] = exchangeValue;
        while (true)
        {
            if (defenderTemp == 0)
            {
                defendList[defendListCount++] = exchangeValue;
                break;
            }
            else if (IsSoleAttacker(defenderTemp, 6) && attackerTemp > 0)
            {
                defendList[defendListCount++] = exchangeValue;
                break;
            }
            else
            {
                endPiece = PopLeastValuableAttacker(defenderTemp);
            }
            exchangeValue -= pieceValue[beginPiece];
            defendList[defendListCount++] = exchangeValue;

            if (attackerTemp == 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else if (IsSoleAttacker(attackerTemp, 6) && defenderTemp > 0)
            {
                attackList[attackListCount++] = exchangeValue;
                break;
            }
            else
            {
                beginPiece = PopLeastValuableAttacker(attackerTemp);
                exchangeValue += pieceValue[endPiece];
                attackList[attackListCount++] = exchangeValue;
            }
        }
        int attackExchangePlace = 0;
        int defendExchangePlace = 0;
        double attackValue = 1000;
        double defendValue = -1000;

        for (int counter = 0; counter < attackListCount; ++counter)
        {
            if (attackList[counter] < attackValue)
            {
                attackExchangePlace = counter;
                attackValue = attackList[counter];
            }
        }

        for (int counter = 0; counter < defendListCount; ++counter)
        {
            if (defendList[counter] > defendValue)
            {
                defendExchangePlace = counter;
                defendValue = defendList[counter];
            }
        }

        int exchangeValueTemp;
        if (defendExchangePlace == attackExchangePlace)
        {
            exchangeValueTemp = static_cast<int>(attackList[attackExchangePlace] * 100);
        }
        else if (defendExchangePlace > attackExchangePlace)
        {
            exchangeValueTemp = static_cast<int>(attackList[attackExchangePlace] * 100);
        }
        else
        {
            exchangeValueTemp = static_cast<int>(defendList[defendExchangePlace] * 100);
        }

        ExchangeCacheWithoutBeginPiece.addToCache(exchangeHash, exchangeValueTemp);
        return exchangeValueTemp;
    }
}

bool MoveLogic::Same(Move &move2, Move &move3, Move &move4, Move &move)
{
    if (move2.beginPlace == move4.endPlace &&
        move3.beginPlace == move.endPlace &&
        move2.endPiece == move4.endPiece &&
        move3.endPiece == move.endPiece &&
        move2.endPlace == move4.endPlace &&
        move3.endPlace == move.endPlace)
    {
        Search::moveCount++;
        return true;
    }
    return false;
}

std::size_t MoveLogic::ExchangeCacheSize()
{
    return ExchangeCache.size();
}

std::size_t MoveLogic::ExchangeWithoutBeginPieceCacheSize()
{
    return ExchangeCacheWithoutBeginPiece.size();
}

ExchangeCacheStatistics MoveLogic::ExchangeCacheStats()
{
    return ExchangeCache.statistics();
}

ExchangeCacheStatistics MoveLogic::ExchangeWithoutBeginPieceCacheStats()
{
    return ExchangeCacheWithoutBeginPiece.statistics();
}

void MoveLogic::ResetExchangeCacheStats()
{
    ExchangeCache.resetStatistics();
    ExchangeCacheWithoutBeginPiece.resetStatistics();
}

bool MoveLogic::ResizeExchangeCache(std::size_t capacityBytes)
{
    return ExchangeCache.resize(capacityBytes);
}

bool MoveLogic::ResizeExchangeWithoutBeginPieceCache(std::size_t capacityBytes)
{
    return ExchangeCacheWithoutBeginPiece.resize(capacityBytes);
}

std::size_t MoveLogic::ExchangeCacheCapacityBytes()
{
    return ExchangeCache.capacityBytes();
}

std::size_t MoveLogic::ExchangeWithoutBeginPieceCacheCapacityBytes()
{
    return ExchangeCacheWithoutBeginPiece.capacityBytes();
}

#if HOWL_CORRECTNESS_TESTING
void MoveLogic::SetExchangeCacheAllocationFailureThresholdForTesting(
    std::size_t capacityBytes)
{
    ExchangeChessCache::SetAllocationFailureThresholdForTesting(capacityBytes);
}
#endif

void MoveLogic::Cleanup()
{
    // Clean up static cache objects
    ExchangeCache.clear();
    ExchangeCacheWithoutBeginPiece.clear();

    // Note: The main memory allocations in this class are temporary:
    // 1. whiteAttacker and blackAttacker arrays in SetWhiteAttacker/SetBlackAttacker
    //    - these are properly cleaned up in MoveGenerator function
    // 2. Move objects created in MoveCopy - these are managed by the caller
    // 3. The static cache objects above have been cleared
}
