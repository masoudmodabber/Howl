#ifndef BOARDINITIALIZER_H
#define BOARDINITIALIZER_H

#include <vector>
#include <random>
#include "Board.h"
#include "Option.h"

class BoardInitializer
{
public:
    static Board *beginBoard;
    static long long ZCode[16][64];
    static long long ZCodeFlag[8];
    static long long ZCodeUnpassentPlace[64];

    static void Initialize();
    static void Cleanup();

private:
    static bool initialized;
};

#endif
