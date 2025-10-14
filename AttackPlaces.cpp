#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "AttackPlaces.h"
#include "Option.h"

long long AttackPlaces::WhitePawnAttackPlaces[64] = {0};
long long AttackPlaces::BlackPawnAttackPlaces[64] = {0};
long long AttackPlaces::KnightAttackPlaces[64] = {0};
long long AttackPlaces::KingAttackPlaces[64] = {0};
long long AttackPlaces::BishopAttack[64][64] = {{0}};
long long AttackPlaces::RookAttack[64][64] = {{0}};
long long AttackPlaces::QueenAttack[64][64] = {{0}};

void AttackPlaces::Initialize()
{
    static bool initialized = false;
    if (!initialized)
    {
        SetPawnAttackPlaces();
        SetKingAttackPlaces();
        SetKnightAttackPlaces();
        SetBishopAttackPlaces();
        SetRookAttackPlaces();
        SetQueenAttackPlaces();
        initialized = true;
    }
}

void AttackPlaces::SetPawnAttackPlaces()
{
    for (int counter = 8; counter < 56; counter++)
    {
        if (counter % 8 == 0)
        {
            WhitePawnAttackPlaces[counter] = Option::PowerTwo[counter + 9];
        }
        else if (counter % 8 == 7)
        {
            WhitePawnAttackPlaces[counter] = Option::PowerTwo[counter + 7];
        }
        else
        {
            WhitePawnAttackPlaces[counter] = Option::PowerTwo[counter + 9] + Option::PowerTwo[counter + 7];
        }
        if (counter % 8 == 0)
        {
            BlackPawnAttackPlaces[counter] = Option::PowerTwo[counter - 7];
        }
        else if (counter % 8 == 7)
        {
            BlackPawnAttackPlaces[counter] = Option::PowerTwo[counter - 9];
        }
        else
        {
            BlackPawnAttackPlaces[counter] = Option::PowerTwo[counter - 7] + Option::PowerTwo[counter - 9];
        }
    }
}

void AttackPlaces::SetKingAttackPlaces()
{
    for (int counter = 0; counter < 64; counter++)
    {
        if (counter % 8 != 7)
        {
            KingAttackPlaces[counter] += Option::PowerTwo[counter + 1];
        }
        if (counter % 8 != 0)
        {
            KingAttackPlaces[counter] += Option::PowerTwo[counter - 1];
        }
        if (counter / 8 != 0)
        {
            KingAttackPlaces[counter] += Option::PowerTwo[counter - 8];
        }
        if (counter / 8 != 7)
        {
            KingAttackPlaces[counter] += Option::PowerTwo[counter + 8];
        }
        if (counter % 8 != 7 && counter / 8 != 7)
        {
            KingAttackPlaces[counter] += Option::PowerTwo[counter + 9];
        }
        if (counter % 8 != 0 && counter / 8 != 7)
        {
            KingAttackPlaces[counter] += Option::PowerTwo[counter + 7];
        }
        if (counter % 8 != 7 && counter / 8 != 0)
        {
            KingAttackPlaces[counter] += Option::PowerTwo[counter - 7];
        }
        if (counter % 8 != 0 && counter / 8 != 0)
        {
            KingAttackPlaces[counter] += Option::PowerTwo[counter - 9];
        }
    }
}

void AttackPlaces::SetKnightAttackPlaces()
{
    for (int counter = 0; counter < 64; counter++)
    {
        if (counter % 8 < 7 && counter / 8 < 6)
        {
            KnightAttackPlaces[counter] += Option::PowerTwo[counter + 17];
        }
        if (counter % 8 < 6 && counter / 8 < 7)
        {
            KnightAttackPlaces[counter] += Option::PowerTwo[counter + 10];
        }
        if (counter % 8 < 6 && counter / 8 > 0)
        {
            KnightAttackPlaces[counter] += Option::PowerTwo[counter - 6];
        }
        if (counter % 8 < 7 && counter / 8 > 1)
        {
            KnightAttackPlaces[counter] += Option::PowerTwo[counter - 15];
        }
        if (counter % 8 > 0 && counter / 8 > 1)
        {
            KnightAttackPlaces[counter] += Option::PowerTwo[counter - 17];
        }
        if (counter % 8 > 1 && counter / 8 > 0)
        {
            KnightAttackPlaces[counter] += Option::PowerTwo[counter - 10];
        }
        if (counter % 8 > 1 && counter / 8 < 7)
        {
            KnightAttackPlaces[counter] += Option::PowerTwo[counter + 6];
        }
        if (counter % 8 > 0 && counter / 8 < 6)
        {
            KnightAttackPlaces[counter] += Option::PowerTwo[counter + 15];
        }
    }
}

void AttackPlaces::SetBishopAttackPlaces()
{
    for (int beginPlace = 0; beginPlace < 64; beginPlace++)
    {
        for (int endPlace = 0; endPlace < 64; endPlace++)
        {
            if (endPlace > beginPlace)
            {
                if (endPlace % 8 < beginPlace % 8 && (endPlace / 8 - beginPlace / 8) == (beginPlace % 8 - endPlace % 8))
                {
                    for (int counter = 1; counter <= (endPlace / 8 - beginPlace / 8); counter++)
                    {
                        BishopAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace + 7 * counter];
                    }
                }
                else if (endPlace % 8 > beginPlace % 8 && (endPlace / 8 - beginPlace / 8) == (endPlace % 8 - beginPlace % 8))
                {
                    for (int counter = 1; counter <= (endPlace / 8 - beginPlace / 8); counter++)
                    {
                        BishopAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace + 9 * counter];
                    }
                }
            }
            if (endPlace < beginPlace)
            {
                if (endPlace % 8 < beginPlace % 8 && (beginPlace / 8 - endPlace / 8) == (beginPlace % 8 - endPlace % 8))
                {
                    for (int counter = 1; counter <= (beginPlace / 8 - endPlace / 8); counter++)
                    {
                        BishopAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace - 9 * counter];
                    }
                }
                else if (endPlace % 8 > beginPlace % 8 && (beginPlace / 8 - endPlace / 8) == (endPlace % 8 - beginPlace % 8))
                {
                    for (int counter = 1; counter <= (beginPlace / 8 - endPlace / 8); counter++)
                    {
                        BishopAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace - 7 * counter];
                    }
                }
            }
        }
    }
}

void AttackPlaces::SetRookAttackPlaces()
{
    for (int beginPlace = 0; beginPlace < 64; beginPlace++)
    {
        for (int endPlace = 0; endPlace < 64; endPlace++)
        {
            if (endPlace > beginPlace && endPlace % 8 == beginPlace % 8)
            {
                for (int counter = 1; counter <= (endPlace / 8 - beginPlace / 8); counter++)
                {
                    RookAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace + 8 * counter];
                }
            }
            if (endPlace > beginPlace && endPlace / 8 == beginPlace / 8)
            {
                for (int counter = 1; counter <= (endPlace % 8 - beginPlace % 8); counter++)
                {
                    RookAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace + 1 * counter];
                }
            }
            if (endPlace < beginPlace && endPlace % 8 == beginPlace % 8)
            {
                for (int counter = 1; counter <= (beginPlace / 8 - endPlace / 8); counter++)
                {
                    RookAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace - 8 * counter];
                }
            }
            if (endPlace < beginPlace && endPlace / 8 == beginPlace / 8)
            {
                for (int counter = 1; counter <= (beginPlace % 8 - endPlace % 8); counter++)
                {
                    RookAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace - 1 * counter];
                }
            }
        }
    }
}

void AttackPlaces::SetQueenAttackPlaces()
{
    for (int beginPlace = 0; beginPlace < 64; beginPlace++)
    {
        for (int endPlace = 0; endPlace < 64; endPlace++)
        {
            if (endPlace > beginPlace)
            {
                if (endPlace % 8 < beginPlace % 8 && (endPlace / 8 - beginPlace / 8) == (beginPlace % 8 - endPlace % 8))
                {
                    for (int counter = 1; counter <= (endPlace / 8 - beginPlace / 8); counter++)
                    {
                        QueenAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace + 7 * counter];
                    }
                }
                else if (endPlace % 8 > beginPlace % 8 && (endPlace / 8 - beginPlace / 8) == (endPlace % 8 - beginPlace % 8))
                {
                    for (int counter = 1; counter <= (endPlace / 8 - beginPlace / 8); counter++)
                    {
                        QueenAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace + 9 * counter];
                    }
                }
            }
            if (endPlace < beginPlace)
            {
                if (endPlace % 8 < beginPlace % 8 && (beginPlace / 8 - endPlace / 8) == (beginPlace % 8 - endPlace % 8))
                {
                    for (int counter = 1; counter <= (beginPlace / 8 - endPlace / 8); counter++)
                    {
                        QueenAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace - 9 * counter];
                    }
                }
                else if (endPlace % 8 > beginPlace % 8 && (beginPlace / 8 - endPlace / 8) == (endPlace % 8 - beginPlace % 8))
                {
                    for (int counter = 1; counter <= (beginPlace / 8 - endPlace / 8); counter++)
                    {
                        QueenAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace - 7 * counter];
                    }
                }
            }
            if (endPlace > beginPlace && endPlace % 8 == beginPlace % 8)
            {
                for (int counter = 1; counter <= (endPlace / 8 - beginPlace / 8); counter++)
                {
                    QueenAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace + 8 * counter];
                }
            }
            if (endPlace > beginPlace && endPlace / 8 == beginPlace / 8)
            {
                for (int counter = 1; counter <= (endPlace % 8 - beginPlace % 8); counter++)
                {
                    QueenAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace + 1 * counter];
                }
            }
            if (endPlace < beginPlace && endPlace % 8 == beginPlace % 8)
            {
                for (int counter = 1; counter <= (beginPlace / 8 - endPlace / 8); counter++)
                {
                    QueenAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace - 8 * counter];
                }
            }
            if (endPlace < beginPlace && endPlace / 8 == beginPlace / 8)
            {
                for (int counter = 1; counter <= (beginPlace % 8 - endPlace % 8); counter++)
                {
                    QueenAttack[beginPlace][endPlace] += Option::PowerTwo[beginPlace - 1 * counter];
                }
            }
        }
    }
}

void AttackPlaces::Cleanup()
{
    // Reset all attack places to zero
    for (int i = 0; i < 64; i++)
    {
        WhitePawnAttackPlaces[i] = 0;
        BlackPawnAttackPlaces[i] = 0;
        KnightAttackPlaces[i] = 0;
        KingAttackPlaces[i] = 0;
        for (int j = 0; j < 64; j++)
        {
            BishopAttack[i][j] = 0;
            RookAttack[i][j] = 0;
            QueenAttack[i][j] = 0;
        }
    }
}