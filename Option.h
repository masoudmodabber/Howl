
#ifndef OPTION_H
#define OPTION_H

class Option
{
public:
    static void Initialize();
    static void Cleanup();

    static char charPowerTwo[8];
    static char charPowerTwoC[8];

    // Option
    static int MultiPV;
    static int nullWindowSize;
    static int checkExtension;
    static int checkExtensionNonPV;
    static int hashSize;
    static int SafetyMargin;
    static int reductiondepth;

    // Memory Share
    static double EvalDictionaryitemSize;
    static double PawnDictionaryitemSize;
    static double ExchangeDictionaryitemSize;

    static double EvalDictionarySharePercent;
    static double PawnDictionarySharePercent;

    static int EvalDictionaryShare;
    static int PawnDictionaryShare;
    static int ExchangeDictionaryShare;

    // Const
    static int futilityMargin;
    static int extendedFutilityMargin;
    static int superExtendedFutilityMargin;

    // Piece Value
    static int PawnValue;
    static int KnightValue;
    static int BishopValue;
    static int RookValue;
    static int QueenValue;
    static int KingValue;

    // Pawn Structure Value
    static int EndPawnValue;
    static int DoubledPawnValue;

    // Changeable
    static int pieceMovement[2][7][120];
    static int WhitePassedPawnValueMiddleGam[64];
    static int PawnInValueWhiteMiddleGame[64];
    static int KnightInValueWhiteMiddleGame[64];
    static int BishopInValueWhiteMiddleGame[64];
    static int RookInValueWhiteMiddleGame[64];
    static int QueenInValueWhiteMiddleGame[64];
    static int KingInValueWhiteMiddleGame[64];
    static int PawnMoveOrderingValueWhiteMiddleGame[64];
    static int KnightMoveOrderingValueWhiteMiddleGame[64];
    static int BishopMoveOrderingValueWhiteMiddleGame[64];
    static int RookMoveOrderingValueWhiteMiddleGame[64];
    static int QueenMoveOrderingValueWhiteMiddleGame[64];
    static int KingMoveOrderingValueWhiteMiddleGame[64];
    static int PawnMoveValueWhiteMiddleGame[64];
    static int KnightMoveValueWhiteMiddleGame[64];
    static int BishopMoveValueWhiteMiddleGame[64];
    static int RookMoveValueWhiteMiddleGame[64];
    static int QueenMoveValueWhiteMiddleGame[64];
    static int KingMoveValueWhiteMiddleGame[64];

    static int PawnMoveValueWhite[3][64];
    static int KnightMoveValueWhite[3][64];
    static int BishopMoveValueWhite[3][64];
    static int RookMoveValueWhite[3][64];
    static int QueenMoveValueWhite[3][64];
    static int KingMoveValueWhite[3][64];

    static int BlackPassedPawnValueMiddleGam[64];
    static int PawnInValueBlackMiddleGame[64];
    static int KnightInValueBlackMiddleGame[64];
    static int BishopInValueBlackMiddleGame[64];
    static int RookInValueBlackMiddleGame[64];
    static int QueenInValueBlackMiddleGame[64];
    static int KingInValueBlackMiddleGame[64];

    static int PawnMoveOrderingValueBlackMiddleGame[64];
    static int KnightMoveOrderingValueBlackMiddleGame[64];
    static int BishopMoveOrderingValueBlackMiddleGame[64];
    static int RookMoveOrderingValueBlackMiddleGame[64];
    static int QueenMoveOrderingValueBlackMiddleGame[64];
    static int KingMoveOrderingValueBlackMiddleGame[64];

    static int PawnMoveValueBlackMiddleGame[64];
    static int KnightMoveValueBlackMiddleGame[64];
    static int BishopMoveValueBlackMiddleGame[64];
    static int RookMoveValueBlackMiddleGame[64];
    static int QueenMoveValueBlackMiddleGame[64];
    static int KingMoveValueBlackMiddleGame[64];

    static int PawnMoveCountValueMiddleGame[3];
    static int PawnMoveCountValue[3][3];
    static int KnightMoveCountValueMiddleGame[9];
    static int KnightMoveCountValue[3][9];
    static int BishopMoveCountValueMiddleGame[16];
    static int BishopMoveCountValue[3][16];
    static int RookMoveCountValueMiddleGame[16];
    static int RookMoveCountValue[3][16];
    static int QueenMoveCountValueMiddleGame[33];
    static int QueenMoveCountValue[3][33];
    static int KingMoveCountValueMiddleGame[9];
    static int KingMoveCountValue[3][9];

    static int PawnAttackValueMiddleGame[16];
    static int PawnAttackValue[3][16];
    static int KnightAttackValueMiddleGame[16];
    static int KnightAttackValue[3][16];
    static int BishopAttackValueMiddleGame[16];
    static int BishopAttackValue[3][16];
    static int RookAttackValueMiddleGame[16];
    static int RookAttackValue[3][16];
    static int QueenAttackValueMiddleGame[16];
    static int QueenAttackValue[3][16];
    static int KingAttackValueMiddleGame[16];
    static int KingAttackValue[3][16];

    static int WhitePawnAttackValueMovement[16];
    static int WhiteKnightAttackValueMovement[16];
    static int WhiteBishopAttackValueMovement[16];
    static int WhiteRookAttackValueMovement[16];
    static int WhiteQueenAttackValueMovement[16];
    static int WhiteKingAttackValueMovement[16];

    static int BlackPawnAttackValueMovement[16];
    static int BlackKnightAttackValueMovement[16];
    static int BlackBishopAttackValueMovement[16];
    static int BlackRookAttackValueMovement[16];
    static int BlackQueenAttackValueMovement[16];
    static int BlackKingAttackValueMovement[16];

    static int WhitePassedPawnValueEndGame[64];
    static int PawnInValueWhiteEndGame[64];
    static int KnightInValueWhiteEndGame[64];
    static int BishopInValueWhiteEndGame[64];
    static int RookInValueWhiteEndGame[64];
    static int QueenInValueWhiteEndGame[64];
    static int KingInValueWhiteEndGame[64];

    static int PawnMoveOrderingValueWhiteEndGame[64];
    static int KnightMoveOrderingValueWhiteEndGame[64];
    static int BishopMoveOrderingValueWhiteEndGame[64];
    static int RookMoveOrderingValueWhiteEndGame[64];
    static int QueenMoveOrderingValueWhiteEndGame[64];
    static int KingMoveOrderingValueWhiteEndGame[64];

    static int PawnMoveValueWhiteEndGame[64];
    static int KnightMoveValueWhiteEndGame[64];
    static int BishopMoveValueWhiteEndGame[64];
    static int RookMoveValueWhiteEndGame[64];
    static int QueenMoveValueWhiteEndGame[64];
    static int KingMoveValueWhiteEndGame[64];

    static int PawnInCenterValueWhite[64];
    static int PawnMoveCenterValueWhite[64];
    static int KnightInCenterValueWhite[64];
    static int KnightMoveCenterValueWhite[64];
    static int BishopInCenterValueWhite[64];
    static int BishopMoveCenterValueWhite[64];
    static int RookInCenterValueWhite[64];
    static int RookMoveCenterValueWhite[64];
    static int QueenInCenterValueWhite[64];
    static int QueenMoveCenterValueWhite[64];
    static int KingInCenterValueWhite[64];
    static int KingMoveCenterValueWhite[64];

    static int WhiteKingPlaceSafetyMiddleGame[64];
    static int WhiteKingPlacePawnShieldMiddleGame[64];
    static int BlackKingPlaceSafetyMiddleGame[64];
    static int BlackKingPlacePawnShieldMiddleGame[64];

    static int PawnInValueWhite[3][64];
    static int KnightInValueWhite[3][64];
    static int BishopInValueWhite[3][64];
    static int RookInValueWhite[3][64];
    static int QueenInValueWhite[3][64];
    static int KingInValueWhite[3][64];

    static int PawnMoveCenterValueBlack[64];
    static int PawnInCenterValueBlack[64];
    static int KnightMoveCenterValueBlack[64];
    static int KnightInCenterValueBlack[64];
    static int BishopMoveCenterValueBlack[64];
    static int BishopInCenterValueBlack[64];
    static int RookMoveCenterValueBlack[64];
    static int RookInCenterValueBlack[64];
    static int QueenMoveCenterValueBlack[64];
    static int QueenInCenterValueBlack[64];
    static int KingMoveCenterValueBlack[64];
    static int KingInCenterValueBlack[64];

    static int BlackPassedPawnValueEndGam[64];
    static int PawnInValueBlackEndGame[64];
    static int KnightInValueBlackEndGame[64];
    static int BishopInValueBlackEndGame[64];
    static int RookInValueBlackEndGame[64];
    static int QueenInValueBlackEndGame[64];
    static int KingInValueBlackEndGame[64];

    static int PawnMoveOrderingValueBlackEndGame[64];
    static int KnightMoveOrderingValueBlackEndGame[64];
    static int BishopMoveOrderingValueBlackEndGame[64];
    static int RookMoveOrderingValueBlackEndGame[64];
    static int QueenMoveOrderingValueBlackEndGame[64];
    static int KingMoveOrderingValueBlackEndGame[64];

    static int PawnMoveValueBlackEndGame[64];
    static int KnightMoveValueBlackEndGame[64];
    static int BishopMoveValueBlackEndGame[64];
    static int RookMoveValueBlackEndGame[64];
    static int QueenMoveValueBlackEndGame[64];
    static int KingMoveValueBlackEndGame[64];

    static int PawnMoveCountValueEndGame[3];
    static int KnightMoveCountValueEndGame[9];
    static int BishopMoveCountValueEndGame[16];
    static int RookMoveCountValueEndGame[16];
    static int QueenMoveCountValueEndGame[33];
    static int KingMoveCountValueEndGame[9];

    static int PawnAttackValueEndGame[16];
    static int KnightAttackValueEndGame[16];
    static int BishopAttackValueEndGame[16];
    static int RookAttackValueEndGame[16];
    static int QueenAttackValueEndGame[16];
    static int KingAttackValueEndGame[16];

    static int PieceArroundTheKingMiddleGame[16];
    static int PieceArroundTheKing[3][16];
    static int PieceArroundTheKingEndGame[16];
    static int PieceAttackArroundTheKingMiddleGame[16];
    static int PieceAttackArroundTheKing[3][16];
    static int PieceAttackArroundTheKingEndGame[16];

    static int ArroundTheKingDangerMiddleGame[4];
    static int ArroundTheKingDangerEndGame[4];

    static int MoveOrderingValueWhite[3][7][64];
    static int MoveOrderingValueBlack[3][7][64];

    static int AttackValueMovement[15][16];

    static long long PowerTwo[64];
    static long long PowerTwoComplement[64];

    static int PawnInValueBlack[3][64];
    static int KnightInValueBlack[3][64];
    static int BishopInValueBlack[3][64];
    static int RookInValueBlack[3][64];
    static int QueenInValueBlack[3][64];
    static int KingInValueBlack[3][64];

    static int PawnMoveValueBlack[3][64];
    static int KnightMoveValueBlack[3][64];
    static int BishopMoveValueBlack[3][64];
    static int RookMoveValueBlack[3][64];
    static int QueenMoveValueBlack[3][64];
    static int KingMoveValueBlack[3][64];

private:
    static bool initialized;
};

#endif