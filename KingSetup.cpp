#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "KingSetup.h"

// Initialize static members
int KingSetup::DistanceToKing[64][64] = {0};
std::vector<int> KingSetup::WhiteKingInFront[64];
std::vector<int> KingSetup::BlackKingInFront[64];
bool KingSetup::initialized = false;

void KingSetup::Initialize()
{
    if (!initialized)
    {
        setDistanceToKing();
        setKingInFront();
        initialized = true;
    }
}

void KingSetup::setDistanceToKing()
{
    for (int counter = 0; counter < 64; counter++)
    {
        if (counter / 8 > 0)
        {
            if (counter % 8 > 0)
            {
                DistanceToKing[counter][counter - 9] = 2;
            }
            if (counter % 8 > 1)
            {
                DistanceToKing[counter][counter - 10] = 3;
            }
            if (counter % 8 < 7)
            {
                DistanceToKing[counter][counter - 7] = 2;
            }
            if (counter % 8 < 6)
            {
                DistanceToKing[counter][counter - 6] = 3;
            }
            DistanceToKing[counter][counter - 8] = 2;
        }

        if (counter / 8 > 1)
        {
            if (counter % 8 > 0)
            {
                DistanceToKing[counter][counter - 17] = 3;
            }
            if (counter % 8 > 1)
            {
                DistanceToKing[counter][counter - 18] = 3;
            }
            if (counter % 8 < 7)
            {
                DistanceToKing[counter][counter - 15] = 3;
            }
            if (counter % 8 < 6)
            {
                DistanceToKing[counter][counter - 14] = 3;
            }
            DistanceToKing[counter][counter - 16] = 3;
        }

        if (counter / 8 < 7)
        {
            if (counter % 8 > 0)
            {
                DistanceToKing[counter][counter + 7] = 2;
            }
            if (counter % 8 > 1)
            {
                DistanceToKing[counter][counter + 6] = 3;
            }
            if (counter % 8 < 7)
            {
                DistanceToKing[counter][counter + 9] = 2;
            }
            if (counter % 8 < 6)
            {
                DistanceToKing[counter][counter + 10] = 3;
            }
            DistanceToKing[counter][counter + 8] = 2;
        }

        if (counter / 8 < 6)
        {
            if (counter % 8 > 0)
            {
                DistanceToKing[counter][counter + 15] = 3;
            }
            if (counter % 8 > 1)
            {
                DistanceToKing[counter][counter + 14] = 3;
            }
            if (counter % 8 < 7)
            {
                DistanceToKing[counter][counter + 17] = 3;
            }
            if (counter % 8 < 6)
            {
                DistanceToKing[counter][counter + 18] = 3;
            }
            DistanceToKing[counter][counter + 16] = 3;
        }
        {
            if (counter % 8 > 0)
            {
                DistanceToKing[counter][counter - 1] = 2;
            }
            if (counter % 8 > 1)
            {
                DistanceToKing[counter][counter - 2] = 3;
            }
            if (counter % 8 < 7)
            {
                DistanceToKing[counter][counter + 1] = 2;
            }
            if (counter % 8 < 6)
            {
                DistanceToKing[counter][counter + 2] = 3;
            }
            {
                DistanceToKing[counter][counter] = 1;
            }
        }
    }
}

void KingSetup::setKingInFront()
{
    for (int counter = 0; counter < 64; counter++)
    {
        WhiteKingInFront[counter].clear();
        BlackKingInFront[counter].clear();
        if (counter / 8 < 7)
        {
            WhiteKingInFront[counter].push_back(counter + 8);
            if (counter % 8 > 0)
            {
                WhiteKingInFront[counter].push_back(counter + 7);
            }
            if (counter % 8 < 7)
            {
                WhiteKingInFront[counter].push_back(counter + 9);
            }
        }

        if (counter / 8 > 0)
        {
            BlackKingInFront[counter].push_back(counter - 8);
            if (counter % 8 > 0)
            {
                BlackKingInFront[counter].push_back(counter - 9);
            }
            if (counter % 8 < 7)
            {
                BlackKingInFront[counter].push_back(counter - 7);
            }
        }
    }
}

void KingSetup::Cleanup()
{
    if (initialized)
    {
        initialized = false;
        for (int i = 0; i < 64; i++)
        {
            WhiteKingInFront[i].clear();
            BlackKingInFront[i].clear();
            for (int j = 0; j < 64; j++)
            {
                DistanceToKing[i][j] = 0;
            }
        }
    }
}