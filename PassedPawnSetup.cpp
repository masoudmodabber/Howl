#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "PassedPawnSetup.h"
#include "Option.h"

// Initialize static members
long long PassedPawnSetup::WhitePassedMask[64] = {0};
long long PassedPawnSetup::BlackPassedMask[64] = {0};
bool PassedPawnSetup::initialized = false;

void PassedPawnSetup::Initialize()
{
    if (!initialized)
    {
        setPassedPawnMask();
        initialized = true;
    }
}

void PassedPawnSetup::setPassedPawnMask()
{
    for (int pawnPlace = 0; pawnPlace < 64; pawnPlace++)
    {
        if (pawnPlace % 8 == 0)
        {
            for (int counter = pawnPlace / 8 + 1; counter < 8; counter++)
            {
                WhitePassedMask[pawnPlace] += Option::PowerTwo[counter * 8];
                WhitePassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + 1];
            }
            for (int counter = 0; counter < pawnPlace / 8; counter++)
            {
                BlackPassedMask[pawnPlace] += Option::PowerTwo[counter * 8];
                BlackPassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + 1];
            }
        }
        else if (pawnPlace % 8 == 7)
        {
            for (int counter = pawnPlace / 8 + 1; counter < 8; counter++)
            {
                WhitePassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + 7];
                WhitePassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + 6];
            }
            for (int counter = 0; counter < pawnPlace / 8; counter++)
            {
                BlackPassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + 7];
                BlackPassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + 6];
            }
        }
        else
        {
            for (int counter = pawnPlace / 8 + 1; counter < 8; counter++)
            {
                WhitePassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + pawnPlace % 8];
                WhitePassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + pawnPlace % 8 + 1];
                WhitePassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + pawnPlace % 8 - 1];
            }
            for (int counter = 0; counter < pawnPlace / 8; counter++)
            {
                BlackPassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + pawnPlace % 8];
                BlackPassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + pawnPlace % 8 + 1];
                BlackPassedMask[pawnPlace] += Option::PowerTwo[counter * 8 + pawnPlace % 8 - 1];
            }
        }
    }
}

void PassedPawnSetup::Cleanup()
{
    if (initialized)
    {
        initialized = false;
    }
}