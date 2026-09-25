
#ifndef OPTION_H
#define OPTION_H

#include <string>

class Option
{
public:
    static void Initialize();
    static void Cleanup();

    static char charPowerTwo[8];
    static char charPowerTwoC[8];

    // Option
    static int MultiPV;
    static std::string SyzygyPath;
    static int SyzygyProbeLimit;
    static int nullWindowSize;
    static int checkExtension;
    static int checkExtensionNonPV;
    static int SafetyMargin;
    static int reductiondepth;

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
    static int IsolatedPawnMiddleGame;
    static int IsolatedPawnEndGame;

    // Rook File Value
    static int RookOpenFileMiddleGame;
    static int RookOpenFileEndGame;
    static int RookSemiOpenFileMiddleGame;
    static int RookSemiOpenFileEndGame;

    // Knight Outpost Value
    static int KnightOutpostMiddleGame;
    static int KnightOutpostEndGame;
    static int KnightSupportedOutpostMiddleGame;
    static int KnightSupportedOutpostEndGame;

    // Rook Behind Passed Pawn Value
    static int RookBehindPassedPawnMiddleGame;
    static int RookBehindPassedPawnEndGame;

    // Inline production evaluation parameters.
    static int BishopPairValue;
    static int BishopOpenFilePawnScale;
    static int TempoMiddleGame;
    static int TempoEndGame;
    static int OppositeColorBishopMiddleGameScalePermille;
    static int OppositeColorBishopEndGameScalePermille;
    static int EndgamePawnAdvancementRankMultiplier;
    static int PieceAttackScalePercent;
    static int LoneKingBase;
    static int LoneKingEdgeWeight;
    static int LoneKingCornerWeight;
    static int LoneKingConfinementWeight;
    static int LoneKingRestrictedNeighbourWeight;
    static int KingAttackerPawnWeight;
    static int KingAttackerMinorWeight;
    static int KingAttackerRookWeight;
    static int KingAttackerQueenWeight;
    static int KingDefenderPawnWeight;
    static int KingDefenderMinorWeight;
    static int KingDefenderRookWeight;
    static int KingDefenderQueenWeight;
    static int KingShelterSecondRankDanger;
    static int KingShelterAdvancedPawnDanger;
    static int KingShelterMissingPawnDanger;
    static int KingShelterOpenFileDanger;
    static int KingUndefendedZoneDanger;
    static int KingAdditionalZoneAttackerDanger;
    static int KingSemiOpenLineDanger;
    static int KingOpenLineDanger;
    static int KingDiagonalLineDanger;
    static int KingControlledEscapeDanger;
    static int KingBlockedEscapeDanger;
    static int KingTrappedEscapeDanger;
    static int KingHeavyBatteryDanger;
    static int KingPinnedShelterPawnWeight;
    static int KingInfiltratedQueenWeight;

    // Changeable
    static int pieceMovement[2][7][120];
    static int WhitePassedPawnValueMiddleGam[64];
    static int PassedPawnMiddleGameParameters[6];
    static int PassedPawnEndGameParameters[6];
    static int PassedPawnMiddleGameFileAmplitude;
    static int PawnPieceSquareMiddleGameParameters[7];
    static int KnightPieceSquareMiddleGameParameters[11];
    static int BishopPieceSquareMiddleGameParameters[11];
    static int RookPieceSquareMiddleGameParameters[6];
    static int QueenPieceSquareMiddleGameParameters[6];
    static int KingPieceSquareMiddleGameParameters[7];
    static int PawnPieceSquareEndGameParameters[7];
    static int KnightPieceSquareEndGameParameters[11];
    static int BishopPieceSquareEndGameParameters[11];
    static int RookPieceSquareEndGameParameters[6];
    static int QueenPieceSquareEndGameParameters[6];
    static int KingPieceSquareEndGameParameters[7];
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
    static int KnightMobilityMiddleGameParameters[4];
    static int KnightMobilityEndGameParameters[4];
    static int KnightMoveCountValueMiddleGame[9];
    static int KnightMoveCountValue[3][9];
    static int BishopMobilityMiddleGameParameters[5];
    static int BishopMobilityEndGameParameters[5];
    static int BishopMoveCountValueMiddleGame[14];
    static int BishopMoveCountValue[3][14];
    static int RookMobilityMiddleGameParameters[5];
    static int RookMobilityEndGameParameters[5];
    static int RookMoveCountValueMiddleGame[15];
    static int RookMoveCountValue[3][15];
    static int QueenMobilityMiddleGameParameters[5];
    static int QueenMobilityEndGameParameters[5];
    static int QueenMoveCountValueMiddleGame[28];
    static int QueenMoveCountValue[3][28];
    static int KingMoveCountValueMiddleGame[9];
    static int KingMoveCountValue[3][9];
    // Experimental Representation Switch (Candidate Attack Model)
    static bool UseExperimentalAttackModel;
    static int AttackEndgameMultiplierPercent;
    // Experimental Representation Switch (Candidate PieceSquare Model: 86 parameters)
    static bool UseExperimentalPieceSquareModel;
    // Experimental Representation Switch (Candidate QueenMobility Model: 8 parameters)
    static bool UseExperimentalQueenMobilityModel;
    // Experimental Representation Switch (Candidate KingSafety Model: 17 parameters)
    static bool UseExperimentalKingSafetyModel;
    // Experimental Representation Switch (Candidate EndgameWeights Model: 3 parameters)
    static bool UseExperimentalEndgameWeightsModel;
    static int CandLoneKingPushWeight;
    static int CandLoneKingConfinementWeight;
    static int CandLoneKingRestrictedNeighbourWeight;
    // Experimental Representation Switch (Candidate Inline Model: 7 parameters)
    static bool UseExperimentalInlineModel;
    static int CandBishopOpenFilePawnScale;
    static int CandTempoMiddleGame;
    static int CandTempoEndGame;
    static int CandOppositeColorBishopMiddleGameScalePermille;
    static int CandOppositeColorBishopEndGameScalePermille;
    static int CandEndgamePawnAdvancementRankMultiplier;
    static int CandPieceAttackScalePercent;
    // Experimental Representation Switch (Candidate RookFile Model: 4 parameters)
    static bool UseExperimentalRookFileModel;
    static int CandRookOpenFileMiddleGame;
    static int CandRookOpenFileEndGame;
    static int CandRookSemiOpenFileMiddleGame;
    static int CandRookSemiOpenFileEndGame;
    static int CandKingAttackerMinorWeight;
    static int CandKingAttackerRookWeight;
    static int CandKingAttackerQueenWeight;
    static int CandKingDefenderPawnWeight;
    static int CandKingDefenderMinorWeight;
    static int CandKingDefenderRookWeight;
    static int CandKingDefenderQueenWeight;
    static int CandKingShelterSecondRankDanger;
    static int CandKingShelterAdvancedPawnDanger;
    static int CandKingShelterMissingPawnDanger;
    static int CandKingUndefendedZoneDanger;
    static int CandKingAdditionalZoneAttackerDanger;
    static int CandKingSemiOpenLineDanger;
    static int CandKingOpenLineDanger;
    static int CandKingDiagonalLineDanger;
    static int CandKingControlledEscapeDanger;
    static int CandKingInfiltratedQueenWeight;
    static int CandQueenMobilityMiddleGameParameters[4];
    static int CandQueenMobilityEndGameParameters[4];
    static int CandPawnPieceSquareMiddleGameParameters[5];
    static int CandKnightPieceSquareMiddleGameParameters[10];
    static int CandBishopPieceSquareMiddleGameParameters[11];
    static int CandRookPieceSquareMiddleGameParameters[5];
    static int CandQueenPieceSquareMiddleGameParameters[5];
    static int CandPawnPieceSquareEndGameParameters[5];
    static int CandKnightPieceSquareEndGameParameters[10];
    static int CandBishopPieceSquareEndGameParameters[11];
    static int CandRookPieceSquareEndGameParameters[5];
    static int CandQueenPieceSquareEndGameParameters[5];

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
    static unsigned long long MoveCenterNonzero[2][7];
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
    static int BishopMoveCountValueEndGame[14];
    static int RookMoveCountValueEndGame[15];
    static int QueenMoveCountValueEndGame[28];
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
