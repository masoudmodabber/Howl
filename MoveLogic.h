#ifndef MoveLogic_h
#define MoveLogic_h

#include "ChessCache.h"
#include "Move.h"
#include "Board.h"

struct MoveList {
    Move* moves[80];
    int count = 0;
};

class MoveLogic
{
public:
    static void Initialize();
    static MoveList MoveGenerator(Board &thisBoard, int depth, int depthGone);
    static int **SetWhiteAttacker(Board &thisBoard);
    static int **SetBlackAttacker(Board &thisBoard);
    static Move *MoveCopy(Move *move);
    static int Exchange(int attacker, int defender, int attackPlace, int beginPiece, int endPiece, int promotionPiece);
    static int ExchangeWithoutBeginPiece(int attacker, int defender, int attackPlace, int beginPiece, int endPiece, int promotionPiece);
    static bool Same(Move &move2, Move &move3, Move &move4, Move &move);
    static void Cleanup();

private:
    static double pieceValue[15];
    static int pieceMoveStack[15];
    static ChessCache ExchangeCache;
    static ChessCache ExchangeCacheWithoutBeginPiece;
    static bool initialized;
};

#endif