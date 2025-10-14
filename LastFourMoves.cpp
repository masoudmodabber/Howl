#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "LastFourMoves.h"


LastFourMoves::LastFourMoves() {
    Move1 = nullptr;
    Move2 = nullptr;
    Move3 = nullptr;
    Move4 = nullptr;
}

LastFourMoves::~LastFourMoves() {
    delete Move1;
    delete Move2;
    delete Move3;
    delete Move4;
}