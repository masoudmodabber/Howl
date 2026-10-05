#ifndef GAME_PHASE_H
#define GAME_PHASE_H

#include "PositionCore.h"
#include <algorithm>

namespace GamePhase
{
inline int Calculate(const PositionCore& position)
{
    const PositionCorePieceListsView pieces{position};
    const int phase = static_cast<int>(pieces[2].size() + pieces[3].size()) +
        2 * static_cast<int>(pieces[4].size()) +
        4 * static_cast<int>(pieces[5].size()) +
        static_cast<int>(pieces[10].size() + pieces[11].size()) +
        2 * static_cast<int>(pieces[12].size()) +
        4 * static_cast<int>(pieces[13].size());
    return std::clamp(phase, 0, 24);
}
}

#endif
