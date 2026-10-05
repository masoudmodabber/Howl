#ifndef NNUE_HISTORY_H
#define NNUE_HISTORY_H

#include "Board.h"

namespace NNUEHistoryLogic
{
void SaveSnapshot(Board& board);
NNUEState RestoreSnapshot(Board& board);
}

#endif
