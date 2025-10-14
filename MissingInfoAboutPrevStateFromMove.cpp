#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "MissingInfoAboutPrevStateFromMove.h"

MissingInfoAboutPrevStateFromMove::MissingInfoAboutPrevStateFromMove(Board& board4)
{
    previousUnpassentPlace = board4.unpassentPlace;
    previousWhiteBigCastle = board4.whiteBigCastle;
    previousWhiteSmallCastle = board4.whiteSmallCastle;
    previousBlackBigCastle = board4.blackBigCastle;
    previousBlackSmallCastle = board4.blackSmallCastle;
}