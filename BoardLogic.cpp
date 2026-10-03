#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "BoardLogic.h"
#include "PositionCore.h"

static bool UnderAttackCore(Board &thisBoard, int position, bool attackerSide)
{
    const PositionCore& core = thisBoard.positionCore;
    const Bitboard wholeBoard = core.colourOccupancy[0] | core.colourOccupancy[1];
    const Bitboard positionBit = Option::PowerTwo[position];
    const int colour = attackerSide ? 1 : 0;
    const Bitboard attackers = core.colourOccupancy[colour];

    const Bitboard pawns = core.pieceOccupancy[0] & attackers;
    if (pawns != 0)
    {
        const Bitboard reversePawnAttacks = attackerSide
            ? static_cast<Bitboard>(AttackPlaces::WhitePawnAttackPlaces[position])
            : static_cast<Bitboard>(AttackPlaces::BlackPawnAttackPlaces[position]);
        if ((pawns & reversePawnAttacks) != 0)
            return true;
    }

    const Bitboard knights = core.pieceOccupancy[1] & attackers;
    if ((static_cast<Bitboard>(AttackPlaces::KnightAttackPlaces[position]) & knights) != 0)
        return true;

    const Bitboard bishops = core.pieceOccupancy[2] & attackers;
    if ((static_cast<Bitboard>(AttackPlaces::BishopPseudoAttacks[position]) & bishops) != 0)
    {
        Bitboard pieces = bishops;
        while (pieces != 0)
        {
            const int from = __builtin_ctzll(pieces);
            pieces &= pieces - 1;
            if ((static_cast<Bitboard>(AttackPlaces::BishopAttack[from][position]) & wholeBoard) == positionBit)
                return true;
        }
    }

    const Bitboard rooks = core.pieceOccupancy[3] & attackers;
    if ((static_cast<Bitboard>(AttackPlaces::RookPseudoAttacks[position]) & rooks) != 0)
    {
        Bitboard pieces = rooks;
        while (pieces != 0)
        {
            const int from = __builtin_ctzll(pieces);
            pieces &= pieces - 1;
            if ((static_cast<Bitboard>(AttackPlaces::RookAttack[from][position]) & wholeBoard) == positionBit)
                return true;
        }
    }

    const Bitboard queens = core.pieceOccupancy[4] & attackers;
    if ((static_cast<Bitboard>(AttackPlaces::QueenPseudoAttacks[position]) & queens) != 0)
    {
        Bitboard pieces = queens;
        while (pieces != 0)
        {
            const int from = __builtin_ctzll(pieces);
            pieces &= pieces - 1;
            if ((static_cast<Bitboard>(AttackPlaces::QueenAttack[from][position]) & wholeBoard) == positionBit)
                return true;
        }
    }

    const Bitboard kings = core.pieceOccupancy[5] & attackers;
    if ((static_cast<Bitboard>(AttackPlaces::KingAttackPlaces[position]) & kings) != 0)
        return true;

    return false;
}

bool BoardLogic::UnderAttack(Board& board, int position, bool attackerSide)
{
    return UnderAttackCore(board, position, attackerSide);
}
