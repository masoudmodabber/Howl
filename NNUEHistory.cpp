#include "NNUEHistory.h"
#include <cstdlib>
#include <iostream>

namespace NNUEHistoryLogic
{
void SaveSnapshot(Board& board)
{
    NNUEHistory& history = *board.nnueHistory;
    if (history.snapshotCount >= NNUEHistory::MaxSnapshots)
    {
        std::cerr << "NNUE SNAPSHOT OVERFLOW: " << history.snapshotCount << '\n';
        std::abort();
    }
    history.snapshots[history.snapshotCount++] = board.nnueState;
}

NNUEState RestoreSnapshot(Board& board)
{
    NNUEHistory& history = *board.nnueHistory;
    if (history.snapshotCount <= 0)
    {
        std::cerr << "NNUE SNAPSHOT UNDERFLOW\n";
        std::abort();
    }
    return history.snapshots[history.snapshotCount - 1];
}
}
