#ifndef PVSSearch_H
#define PVSSearch_H

#include "MovePrintValue.h"
#include "Move.h"
#include "Board.h"
#include "MoveLogic.h"
#include <algorithm>
#include <cmath>

class PVSSearch {
public:
    static MovePrintValue* PVS(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool MAtESearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch, bool selectiveSearch = false, bool cutNode = false);
    static void deleteMoveList(MoveList moveList);
    struct KillerMove {
        int beginPlace = -1;
        int endPlace = -1;
        int promotionPiece = 0;

        bool operator==(const Move& m) const {
            return beginPlace == m.beginPlace && endPlace == m.endPlace &&
                   (promotionPiece == 0 ? (m.promotionPiece <= 0) : (m.promotionPiece == promotionPiece));
        }
        bool operator!=(const Move& m) const {
            return !(*this == m);
        }
        bool isValid() const {
            return beginPlace >= 0 && endPlace >= 0;
        }
    };

    static constexpr int MaxKillerPly = 128;
    static KillerMove killers[MaxKillerPly][2];

    static void ResetKillers();
    static void SetVerificationIsolation(bool enabled);
    static void ResetHistory();
    static void RecordKiller(int ply, const Move& move);
    static int QCaptureOrderingScore(const Board& board, const Move& move);
    static int QQuietEvasionOrderingScore(int side, int ply,
                                          const Board& board, const Move& move);

private:
    static int moveOrderingDepth[20];
    static double NullMoveReduction(bool isPVNode, int alpha, int beta, int depth, Move& prevMove, Move& move1, Move& move2, Move& move3, Board& board4, bool mateSearch, bool isNullMoveAllowed, int depthGone, bool previousMoveWasCheck, bool nullWindowSearch);
};

#endif
