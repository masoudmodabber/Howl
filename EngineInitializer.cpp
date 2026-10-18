#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "EngineInitializer.h"
#include "Mobility.h"
#include "PassedPawn.h"
#include "PieceSquareModel.h"

char EngineInitializer::charPowerTwo[8] = {};
char EngineInitializer::charPowerTwoC[8] = {};
int EngineInitializer::PawnValue = 120;
int EngineInitializer::KnightValue = 320;
int EngineInitializer::BishopValue = 375;
int EngineInitializer::RookValue = 545;
int EngineInitializer::QueenValue = 1135;
int EngineInitializer::KingValue = 1200;
int EngineInitializer::EndPawnValue = -20;
int EngineInitializer::DoubledPawnValue = -20;
int EngineInitializer::IsolatedPawnMiddleGame = -10;
int EngineInitializer::IsolatedPawnEndGame = -8;
int EngineInitializer::RookOpenFileMiddleGame = 12;
int EngineInitializer::RookOpenFileEndGame = 2;
int EngineInitializer::RookSemiOpenFileMiddleGame = -4;
int EngineInitializer::RookSemiOpenFileEndGame = -26;
int EngineInitializer::KnightOutpostMiddleGame = 18;
int EngineInitializer::KnightOutpostEndGame = -3;
int EngineInitializer::KnightSupportedOutpostMiddleGame = 44;
int EngineInitializer::KnightSupportedOutpostEndGame = 48;
int EngineInitializer::RookBehindPassedPawnMiddleGame = 4;
int EngineInitializer::RookBehindPassedPawnEndGame = 12;
int EngineInitializer::BishopPairValue = 132;
int EngineInitializer::BishopOpenFilePawnScale = 2;
int EngineInitializer::TempoMiddleGame = 16;
int EngineInitializer::TempoEndGame = 41;
int EngineInitializer::OppositeColorBishopMiddleGameScalePermille = 940;
int EngineInitializer::OppositeColorBishopEndGameScalePermille = 794;
int EngineInitializer::EndgamePawnAdvancementRankMultiplier = 2;
int EngineInitializer::PieceAttackScalePercent = 135;
int EngineInitializer::LoneKingBase = 36;
int EngineInitializer::LoneKingEdgeWeight = -72;
int EngineInitializer::LoneKingCornerWeight = 22;
int EngineInitializer::LoneKingConfinementWeight = -10;
int EngineInitializer::LoneKingRestrictedNeighbourWeight = -26;
int EngineInitializer::KingAttackerPawnWeight = 0;
int EngineInitializer::KingAttackerMinorWeight = 9;
int EngineInitializer::KingAttackerRookWeight = 8;
int EngineInitializer::KingAttackerQueenWeight = 8;
int EngineInitializer::KingDefenderPawnWeight = 12;
int EngineInitializer::KingDefenderMinorWeight = 108;
int EngineInitializer::KingDefenderRookWeight = 6;
int EngineInitializer::KingDefenderQueenWeight = 135;
int EngineInitializer::KingShelterSecondRankDanger = 1;
int EngineInitializer::KingShelterAdvancedPawnDanger = 20;
int EngineInitializer::KingShelterMissingPawnDanger = 2;
int EngineInitializer::KingShelterOpenFileDanger = 0;
int EngineInitializer::KingUndefendedZoneDanger = 4;
int EngineInitializer::KingAdditionalZoneAttackerDanger = 55;
int EngineInitializer::KingSemiOpenLineDanger = 36;
int EngineInitializer::KingOpenLineDanger = 26;
int EngineInitializer::KingDiagonalLineDanger = 9;
int EngineInitializer::KingControlledEscapeDanger = 22;
int EngineInitializer::KingBlockedEscapeDanger = 0;
int EngineInitializer::KingTrappedEscapeDanger = 0;
int EngineInitializer::KingHeavyBatteryDanger = 0;
int EngineInitializer::KingPinnedShelterPawnWeight = 0;
int EngineInitializer::KingInfiltratedQueenWeight = 19;
int EngineInitializer::pieceMovement[2][7][120] = {};

int EngineInitializer::PassedPawnMiddleGameParameters[] = {9, 2, 3, 2, 3, 2};
int EngineInitializer::PassedPawnEndGameParameters[] = {15, 10, 15, 20, 35, 55};
int EngineInitializer::PassedPawnMiddleGameFileAmplitude = 7;
int EngineInitializer::WhitePassedPawnValueMiddleGam[64] = {};

int EngineInitializer::PawnPieceSquareMiddleGameParameters[] = {-52, 6, 0, 14, -28, 6, 0};
int EngineInitializer::KnightPieceSquareMiddleGameParameters[] = {-2, 0, 0, 0, -5, -3, -2, 4, 4, -14, 29};
int EngineInitializer::BishopPieceSquareMiddleGameParameters[] = {-49, -8, -5, -1, -12, -77, 0, -2, 12, 24, 29};
int EngineInitializer::RookPieceSquareMiddleGameParameters[] = {-108, 6, 0, 10, 10, 2};
int EngineInitializer::QueenPieceSquareMiddleGameParameters[] = {18, 0, 0, -6, 14, -19};
int EngineInitializer::KingPieceSquareMiddleGameParameters[] = {18, 38, 16, 28, 2, -30, -26};

int EngineInitializer::PawnInValueWhiteMiddleGame[64] = {};
int EngineInitializer::KnightInValueWhiteMiddleGame[64] = {};
int EngineInitializer::BishopInValueWhiteMiddleGame[64] = {};
int EngineInitializer::RookInValueWhiteMiddleGame[64] = {};
int EngineInitializer::QueenInValueWhiteMiddleGame[64] = {};
int EngineInitializer::KingInValueWhiteMiddleGame[64] = {};

int EngineInitializer::PawnMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 1, 0, 0, 0, 0, 0,
                                                      0, 0, 3, 7, 7, 1, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KnightMoveOrderingValueWhiteMiddleGame[] = {-6, -5, -4, -3, -3, -4, -5, -6,
                                                        -4, -3, -2, -1, -1, -2, -3, -4,
                                                        0, 0, 2, 0, 0, 2, 0, 0,
                                                        0, 0, 3, 7, 7, 3, 0, 0,
                                                        0, 5, 10, 15, 15, 10, 5, 0,
                                                        0, 10, 15, 20, 20, 15, 10, 0,
                                                        0, 5, 10, 15, 15, 10, 5, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::BishopMoveOrderingValueWhiteMiddleGame[] = {-5, -4, -3, -2, -2, -3, -4, -5,
                                                        0, 1, 0, 0, 0, 0, 1, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 2, 6, 6, 2, 0, 0,
                                                        0, 4, 8, 10, 10, 8, 4, 0,
                                                        0, 8, 10, 12, 12, 10, 8, 0,
                                                        0, 4, 8, 10, 10, 8, 4, 0};

int EngineInitializer::RookMoveOrderingValueWhiteMiddleGame[] = {0, -8, 1, 0, 0, 1, -8, 0,
                                                      -8, -8, -8, -8, -8, -8, -8, -8,
                                                      -8, -8, -8, -8, -8, -8, -8, -8,
                                                      -8, -8, -8, -8, -8, -8, -8, -8,
                                                      -8, -8, -8, -8, -8, -8, -8, -8,
                                                      -8, -8, -8, -8, -8, -8, -8, -8,
                                                      25, 25, 25, 25, 25, 25, 25, 25,
                                                      5, 5, 5, 5, 5, 5, 5, 5};

int EngineInitializer::QueenMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       5, 5, 5, 5, 5, 5, 5, 5,
                                                       0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KingMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

// till820
/*
int EngineInitializer::PawnMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KnightMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::BishopMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::RookMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::QueenMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KingMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::PawnMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KnightMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::BishopMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::RookMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::QueenMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KingMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::PawnMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KnightMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::BishopMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::RookMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::QueenMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KingMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};
*/
int EngineInitializer::PawnMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KnightMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::BishopMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::RookMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::QueenMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KingMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::PawnMoveValueWhite[3][64] = {};
int EngineInitializer::KnightMoveValueWhite[3][64] = {};
int EngineInitializer::BishopMoveValueWhite[3][64] = {};
int EngineInitializer::RookMoveValueWhite[3][64] = {};
int EngineInitializer::QueenMoveValueWhite[3][64] = {};
int EngineInitializer::KingMoveValueWhite[3][64] = {};

int EngineInitializer::BlackPassedPawnValueMiddleGam[64] = {};
int EngineInitializer::PawnInValueBlackMiddleGame[64] = {};
int EngineInitializer::KnightInValueBlackMiddleGame[64] = {};
int EngineInitializer::BishopInValueBlackMiddleGame[64] = {};
int EngineInitializer::RookInValueBlackMiddleGame[64] = {};
int EngineInitializer::QueenInValueBlackMiddleGame[64] = {};
int EngineInitializer::KingInValueBlackMiddleGame[64] = {};

int EngineInitializer::PawnMoveOrderingValueBlackMiddleGame[64] = {};
int EngineInitializer::KnightMoveOrderingValueBlackMiddleGame[64] = {};
int EngineInitializer::BishopMoveOrderingValueBlackMiddleGame[64] = {};
int EngineInitializer::RookMoveOrderingValueBlackMiddleGame[64] = {};
int EngineInitializer::QueenMoveOrderingValueBlackMiddleGame[64] = {};
int EngineInitializer::KingMoveOrderingValueBlackMiddleGame[64] = {};

int EngineInitializer::PawnMoveValueBlackMiddleGame[64] = {};
int EngineInitializer::KnightMoveValueBlackMiddleGame[64] = {};
int EngineInitializer::BishopMoveValueBlackMiddleGame[64] = {};
int EngineInitializer::RookMoveValueBlackMiddleGame[64] = {};
int EngineInitializer::QueenMoveValueBlackMiddleGame[64] = {};
int EngineInitializer::KingMoveValueBlackMiddleGame[64] = {};

int EngineInitializer::PawnMoveCountValueMiddleGame[3] = {0, 0, 0};
int EngineInitializer::PawnMoveCountValue[3][3] = {};

int EngineInitializer::KnightMobilityMiddleGameParameters[] = {-46, 26, 10, 8};
int EngineInitializer::KnightMobilityEndGameParameters[] = {21, 24, 20, 0};
int EngineInitializer::KnightMoveCountValueMiddleGame[9] = {};
int EngineInitializer::KnightMoveCountValue[3][9] = {};
int EngineInitializer::BishopMobilityMiddleGameParameters[] = {-60, 28, 14, -1, 37};
int EngineInitializer::BishopMobilityEndGameParameters[] = {-25, 61, 12, 25, 3};
int EngineInitializer::BishopMoveCountValueMiddleGame[14] = {};
int EngineInitializer::BishopMoveCountValue[3][14] = {};
int EngineInitializer::RookMobilityMiddleGameParameters[] = {-30, 14, 25, 8, -1};
int EngineInitializer::RookMobilityEndGameParameters[] = {13, 62, 30, 34, 3};
int EngineInitializer::RookMoveCountValueMiddleGame[15] = {};
int EngineInitializer::RookMoveCountValue[3][15] = {};
int EngineInitializer::QueenMobilityMiddleGameParameters[] = {-6, 3, 15, 4, 4};
int EngineInitializer::QueenMobilityEndGameParameters[] = {10, 30, 33, 31, 27};
int EngineInitializer::QueenMoveCountValueMiddleGame[28] = {};
int EngineInitializer::QueenMoveCountValue[3][28] = {};
int EngineInitializer::KingMoveCountValueMiddleGame[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int EngineInitializer::KingMoveCountValue[3][9] = {};
bool EngineInitializer::UseAttackModel = true;
int EngineInitializer::AttackEndgameMultiplierPercent = 140;
bool EngineInitializer::UsePieceSquareModel = true;
bool EngineInitializer::UseQueenMobilityModel = true;
bool EngineInitializer::UseKingSafetyModel = true;
bool EngineInitializer::UseEndgameWeightsModel = true;
bool EngineInitializer::UseInlineModel = true;

int EngineInitializer::CandLoneKingPushWeight = 22;
int EngineInitializer::CandLoneKingConfinementWeight = -10;
int EngineInitializer::CandLoneKingRestrictedNeighbourWeight = -26;

int EngineInitializer::CandBishopOpenFilePawnScale = 2;
int EngineInitializer::CandTempoMiddleGame = 16;
int EngineInitializer::CandTempoEndGame = 41;
int EngineInitializer::CandOppositeColorBishopMiddleGameScalePermille = 940;
int EngineInitializer::CandOppositeColorBishopEndGameScalePermille = 794;
int EngineInitializer::CandEndgamePawnAdvancementRankMultiplier = 2;
int EngineInitializer::CandPieceAttackScalePercent = 135;

bool EngineInitializer::UseRookFileModel = true;
int EngineInitializer::CandRookOpenFileMiddleGame = 12;
int EngineInitializer::CandRookOpenFileEndGame = 2;
int EngineInitializer::CandRookSemiOpenFileMiddleGame = -4;
int EngineInitializer::CandRookSemiOpenFileEndGame = -26;

int EngineInitializer::CandKingAttackerMinorWeight = 9;
int EngineInitializer::CandKingAttackerRookWeight = 8;
int EngineInitializer::CandKingAttackerQueenWeight = 8;
int EngineInitializer::CandKingDefenderPawnWeight = 12;
int EngineInitializer::CandKingDefenderMinorWeight = 108;
int EngineInitializer::CandKingDefenderRookWeight = 6;
int EngineInitializer::CandKingDefenderQueenWeight = 135;
int EngineInitializer::CandKingShelterSecondRankDanger = 1;
int EngineInitializer::CandKingShelterAdvancedPawnDanger = 20;
int EngineInitializer::CandKingShelterMissingPawnDanger = 2;
int EngineInitializer::CandKingUndefendedZoneDanger = 4;
int EngineInitializer::CandKingAdditionalZoneAttackerDanger = 55;
int EngineInitializer::CandKingSemiOpenLineDanger = 36;
int EngineInitializer::CandKingOpenLineDanger = 26;
int EngineInitializer::CandKingDiagonalLineDanger = 9;
int EngineInitializer::CandKingControlledEscapeDanger = 22;
int EngineInitializer::CandKingInfiltratedQueenWeight = 19;

int EngineInitializer::CandQueenMobilityMiddleGameParameters[] = {-6, 3, 15, 4};
int EngineInitializer::CandQueenMobilityEndGameParameters[] = {10, 30, 33, 31};

int EngineInitializer::CandPawnPieceSquareMiddleGameParameters[] = {-52, 6, 14, -28, 6};
int EngineInitializer::CandKnightPieceSquareMiddleGameParameters[] = {-2, 0, 0, -5, -3, -2, 4, 4, -14, 29};
int EngineInitializer::CandBishopPieceSquareMiddleGameParameters[] = {-49, -8, -5, -1, -12, -77, 0, -2, 12, 24, 29};
int EngineInitializer::CandRookPieceSquareMiddleGameParameters[] = {-108, 6, 10, 10, 2};
int EngineInitializer::CandQueenPieceSquareMiddleGameParameters[] = {18, 0, -6, 14, -19};

int EngineInitializer::CandPawnPieceSquareEndGameParameters[] = {40, 6, -16, -18, 20};
int EngineInitializer::CandKnightPieceSquareEndGameParameters[] = {58, 14, 4, -2, 9, 12, 24, 12, 20, -24};
int EngineInitializer::CandBishopPieceSquareEndGameParameters[] = {83, 4, 9, -1, 18, 93, 10, 2, 12, 10, -33};
int EngineInitializer::CandRookPieceSquareEndGameParameters[] = {126, 10, -6, 32, -8};
int EngineInitializer::CandQueenPieceSquareEndGameParameters[] = {26, 8, 28, -58, 12};

int EngineInitializer::PawnAttackValueMiddleGame[] = {0, 0, 20, 20, 76, 86, 0, 0, 0, 0, 20, 20, 76, 86, 0, 0};
int EngineInitializer::PawnAttackValue[3][16] = {};
int EngineInitializer::KnightAttackValueMiddleGame[] = {0, 7, 0, 24, 41, 41, 0, 0, 0, 7, 0, 24, 41, 41, 0, 0};
int EngineInitializer::KnightAttackValue[3][16] = {};
int EngineInitializer::BishopAttackValueMiddleGame[] = {0, 7, 24, 0, 41, 41, 0, 0, 0, 7, 24, 0, 41, 41, 0, 0};
int EngineInitializer::BishopAttackValue[3][16] = {};
int EngineInitializer::RookAttackValueMiddleGame[] = {0, -1, 15, 15, 0, 24, 0, 0, 0, -1, 15, 15, 0, 24, 0, 0};
int EngineInitializer::RookAttackValue[3][16] = {};
int EngineInitializer::QueenAttackValueMiddleGame[] = {0, 3, 8, 8, 16, 0, 0, 0, 0, 3, 8, 8, 16, 0, 0, 0};
int EngineInitializer::QueenAttackValue[3][16] = {};
int EngineInitializer::KingAttackValueMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int EngineInitializer::KingAttackValue[3][16] = {};

int EngineInitializer::WhitePawnAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 35, 10, 10, 20, 40, 0, 0};
int EngineInitializer::WhiteKnightAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 50, 50, 10, 20, 0, 0};
int EngineInitializer::WhiteBishopAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 50, 50, 10, 20, 0, 0};
int EngineInitializer::WhiteRookAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 25, 35, 50, 150, 0, 0};
int EngineInitializer::WhiteQueenAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 20, 35, 50, 10, 0, 0};
int EngineInitializer::WhiteKingAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::BlackPawnAttackValueMovement[16] = {0, 35, 10, 10, 20, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int EngineInitializer::BlackKnightAttackValueMovement[16] = {0, 15, 50, 50, 10, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int EngineInitializer::BlackBishopAttackValueMovement[16] = {0, 15, 50, 50, 10, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int EngineInitializer::BlackRookAttackValueMovement[16] = {0, 10, 25, 35, 50, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int EngineInitializer::BlackQueenAttackValueMovement[16] = {0, 5, 20, 35, 50, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int EngineInitializer::BlackKingAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::WhitePassedPawnValueEndGame[64] = {};

int EngineInitializer::PawnPieceSquareEndGameParameters[] = {40, 6, 0, -16, -18, 20, 0};
int EngineInitializer::KnightPieceSquareEndGameParameters[] = {58, 14, 4, 0, -2, 9, 12, 24, 12, 20, -24};
int EngineInitializer::BishopPieceSquareEndGameParameters[] = {83, 4, 9, -1, 18, 93, 10, 2, 12, 10, -33};
int EngineInitializer::RookPieceSquareEndGameParameters[] = {126, 10, 0, -6, 32, -8};
int EngineInitializer::QueenPieceSquareEndGameParameters[] = {26, 8, 0, 28, -58, 12};
int EngineInitializer::KingPieceSquareEndGameParameters[] = {1, 42, 46, 62, 48, 64, 96};

int EngineInitializer::PawnInValueWhiteEndGame[64] = {};
int EngineInitializer::KnightInValueWhiteEndGame[64] = {};
int EngineInitializer::BishopInValueWhiteEndGame[64] = {};
int EngineInitializer::RookInValueWhiteEndGame[64] = {};
int EngineInitializer::QueenInValueWhiteEndGame[64] = {};
int EngineInitializer::KingInValueWhiteEndGame[64] = {};

int EngineInitializer::PawnMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KnightMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 4, 8, 8, 4, 0, 0,
                                                     0, 4, 17, 26, 26, 17, 4, 0,
                                                     0, 8, 26, 35, 35, 26, 8, 0,
                                                     0, 4, 17, 17, 17, 17, 4, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::BishopMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 5, 5, 5, 5, 0, 0,
                                                     0, 5, 10, 10, 10, 10, 5, 0,
                                                     0, 10, 21, 21, 21, 21, 10, 0,
                                                     0, 5, 8, 8, 8, 8, 5, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::RookMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   47, 47, 47, 47, 47, 47, 47, 47,
                                                   0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::QueenMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0,
                                                    27, 27, 27, 27, 27, 27, 27, 27,
                                                    0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KingMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::PawnMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KnightMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::BishopMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::RookMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::QueenMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KingMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::PawnInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 10, 15, 15, 10, 0, 0,
                                        0, 0, 0, 5, 5, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::PawnInValueWhite[3][64] = {{0}};
int EngineInitializer::KnightInValueWhite[3][64] = {{0}};
int EngineInitializer::BishopInValueWhite[3][64] = {{0}};
int EngineInitializer::RookInValueWhite[3][64] = {{0}};
int EngineInitializer::QueenInValueWhite[3][64] = {{0}};
int EngineInitializer::KingInValueWhite[3][64] = {{0}};

int EngineInitializer::PawnMoveCenterValueBlack[64] = {0};
unsigned long long EngineInitializer::MoveCenterNonzero[2][7] = {};
int EngineInitializer::PawnInCenterValueBlack[64] = {0};
int EngineInitializer::KnightMoveCenterValueBlack[64] = {0};
int EngineInitializer::KnightInCenterValueBlack[64] = {0};
int EngineInitializer::BishopMoveCenterValueBlack[64] = {0};
int EngineInitializer::BishopInCenterValueBlack[64] = {0};
int EngineInitializer::RookMoveCenterValueBlack[64] = {0};
int EngineInitializer::RookInCenterValueBlack[64] = {0};
int EngineInitializer::QueenMoveCenterValueBlack[64] = {0};
int EngineInitializer::QueenInCenterValueBlack[64] = {0};
int EngineInitializer::KingMoveCenterValueBlack[64] = {0};
int EngineInitializer::KingInCenterValueBlack[64] = {0};

int EngineInitializer::PawnMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 2, 4, 4, 2, 0, 0,
                                          0, 0, 3, 7, 7, 3, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KnightInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 2, 2, 2, 2, 0, 0,
                                          0, 0, 2, 4, 4, 2, 0, 0,
                                          0, 0, 2, 4, 4, 2, 0, 0,
                                          0, 0, 2, 2, 2, 2, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KnightMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 2, 2, 2, 2, 0, 0,
                                            0, 0, 2, 4, 4, 2, 0, 0,
                                            0, 0, 2, 4, 4, 2, 0, 0,
                                            0, 0, 2, 2, 2, 2, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::BishopInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 2, 2, 2, 2, 0, 0,
                                          0, 0, 2, 3, 3, 2, 0, 0,
                                          0, 0, 2, 3, 3, 2, 0, 0,
                                          0, 0, 2, 2, 2, 2, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::BishopMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 2, 2, 2, 2, 0, 0,
                                            0, 0, 2, 3, 3, 2, 0, 0,
                                            0, 0, 2, 3, 3, 2, 0, 0,
                                            0, 0, 2, 2, 2, 2, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::RookInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 1, 1, 1, 1, 0, 0,
                                        0, 0, 1, 2, 2, 1, 0, 0,
                                        0, 0, 1, 2, 2, 1, 0, 0,
                                        0, 0, 1, 1, 1, 1, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::RookMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 1, 1, 1, 1, 0, 0,
                                          0, 0, 1, 2, 2, 1, 0, 0,
                                          0, 0, 1, 2, 2, 1, 0, 0,
                                          0, 0, 1, 1, 1, 1, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0};
int EngineInitializer::QueenInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 1, 1, 1, 1, 0, 0,
                                         0, 0, 1, 2, 2, 1, 0, 0,
                                         0, 0, 1, 2, 2, 1, 0, 0,
                                         0, 0, 1, 1, 1, 1, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::QueenMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 1, 1, 1, 1, 0, 0,
                                           0, 0, 1, 2, 2, 1, 0, 0,
                                           0, 0, 1, 2, 2, 1, 0, 0,
                                           0, 0, 1, 1, 1, 1, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KingInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::KingMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0};
int EngineInitializer::BlackPassedPawnValueEndGam[64] = {0};

int EngineInitializer::PawnInValueBlackEndGame[64] = {0};
int EngineInitializer::KnightInValueBlackEndGame[64] = {0};
int EngineInitializer::BishopInValueBlackEndGame[64] = {0};
int EngineInitializer::RookInValueBlackEndGame[64] = {0};
int EngineInitializer::QueenInValueBlackEndGame[64] = {0};
int EngineInitializer::KingInValueBlackEndGame[64] = {0};

int EngineInitializer::PawnMoveOrderingValueBlackEndGame[64] = {0};
int EngineInitializer::KnightMoveOrderingValueBlackEndGame[64] = {0};
int EngineInitializer::BishopMoveOrderingValueBlackEndGame[64] = {0};
int EngineInitializer::RookMoveOrderingValueBlackEndGame[64] = {0};
int EngineInitializer::QueenMoveOrderingValueBlackEndGame[64] = {0};
int EngineInitializer::KingMoveOrderingValueBlackEndGame[64] = {0};

int EngineInitializer::PawnMoveValueBlackEndGame[64] = {0};
int EngineInitializer::KnightMoveValueBlackEndGame[64] = {0};
int EngineInitializer::BishopMoveValueBlackEndGame[64] = {0};
int EngineInitializer::RookMoveValueBlackEndGame[64] = {0};
int EngineInitializer::QueenMoveValueBlackEndGame[64] = {0};
int EngineInitializer::KingMoveValueBlackEndGame[64] = {0};

int EngineInitializer::PawnMoveCountValueEndGame[3] = {0};
int EngineInitializer::KnightMoveCountValueEndGame[9] = {};
int EngineInitializer::BishopMoveCountValueEndGame[14] = {};
int EngineInitializer::RookMoveCountValueEndGame[15] = {};
int EngineInitializer::QueenMoveCountValueEndGame[28] = {};
int EngineInitializer::KingMoveCountValueEndGame[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::PawnAttackValueEndGame[] = {0, 0, 20, 20, 99, 118, 0, 0, 0, 0, 20, 20, 99, 118, 0, 0};
int EngineInitializer::KnightAttackValueEndGame[] = {0, 10, 0, 39, 49, 49, 0, 0, 0, 10, 0, 39, 49, 49, 0, 0};
int EngineInitializer::BishopAttackValueEndGame[] = {0, 10, 39, 0, 49, 49, 0, 0, 0, 10, 39, 0, 49, 49, 0, 0};
int EngineInitializer::RookAttackValueEndGame[] = {0, 29, 49, 49, 0, 49, 0, 0, 0, 29, 49, 49, 0, 49, 0, 0};
int EngineInitializer::QueenAttackValueEndGame[] = {0, 6, 15, 15, 30, 0, 0, 0, 0, 6, 15, 15, 30, 0, 0, 0};
int EngineInitializer::KingAttackValueEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int EngineInitializer::PieceArroundTheKingMiddleGame[16] = {0, 5, 10, 10, 12, 25, 0, 0, 0, 5, 10, 10, 12, 25, 0, 0};
int EngineInitializer::PieceArroundTheKing[3][16] = {{0}};
int EngineInitializer::PieceArroundTheKingEndGame[16] = {0, 5, 10, 10, 12, 25, 0, 0, 0, 5, 10, 10, 12, 25, 0, 0};
int EngineInitializer::PieceAttackArroundTheKingMiddleGame[16] = {0, 1, 3, 3, 5, 9, 0, 0, 0, 1, 3, 3, 5, 9, 0, 0};
int EngineInitializer::PieceAttackArroundTheKing[3][16] = {{0}};
int EngineInitializer::PieceAttackArroundTheKingEndGame[16] = {0, 1, 3, 3, 5, 9, 0, 0, 0, 1, 3, 3, 5, 9, 0, 0};

int EngineInitializer::WhiteKingPlaceSafetyMiddleGame[] = {-4, -0, -6, -20, -15, -6, -0, -4,
                                                -8, -6, -12, -20, -20, -12, -6, -8,
                                                -18, -18, -22, -30, -30, -22, -18, -18,
                                                -35, -35, -35, -35, -35, -35, -35, -35,
                                                -35, -35, -35, -35, -35, -35, -35, -35,
                                                -35, -35, -35, -35, -35, -35, -35, -35,
                                                -35, -35, -35, -35, -35, -35, -35, -35,
                                                -35, -35, -35, -35, -35, -35, -35, -35};
int EngineInitializer::WhiteKingPlacePawnShieldMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                    -7, -7, -11, -9, -9, -11, -7, -7,
                                                    -16, -16, -16, -16, -16, -16, -16, -16,
                                                    -16, -16, -16, -16, -16, -16, -16, -16,
                                                    -16, -16, -16, -16, -16, -16, -16, -16,
                                                    -16, -16, -16, -16, -16, -16, -16, -16,
                                                    -16, -16, -16, -16, -16, -16, -16, -16,
                                                    -16, -16, -16, -16, -16, -16, -16, -16};

int EngineInitializer::BlackKingPlaceSafetyMiddleGame[64] = {0};
int EngineInitializer::BlackKingPlacePawnShieldMiddleGame[64] = {0};

int EngineInitializer::ArroundTheKingDangerMiddleGame[4] = {0, 5, 3, 1};
int EngineInitializer::ArroundTheKingDangerEndGame[4] = {0, 5, 3, 1};

int EngineInitializer::MoveOrderingValueWhite[3][7][64] = {{{0}}};
int EngineInitializer::MoveOrderingValueBlack[3][7][64] = {{{0}}};

int EngineInitializer::AttackValueMovement[15][16] = {{0}};

long long EngineInitializer::PowerTwo[64] = {0};
long long EngineInitializer::PowerTwoComplement[64] = {0};

int EngineInitializer::PawnInValueBlack[3][64] = {{0}};
int EngineInitializer::KnightInValueBlack[3][64] = {{0}};
int EngineInitializer::BishopInValueBlack[3][64] = {{0}};
int EngineInitializer::RookInValueBlack[3][64] = {{0}};
int EngineInitializer::QueenInValueBlack[3][64] = {{0}};
int EngineInitializer::KingInValueBlack[3][64] = {{0}};

int EngineInitializer::PawnMoveValueBlack[3][64] = {{0}};
int EngineInitializer::KnightMoveValueBlack[3][64] = {{0}};
int EngineInitializer::BishopMoveValueBlack[3][64] = {{0}};
int EngineInitializer::RookMoveValueBlack[3][64] = {{0}};
int EngineInitializer::QueenMoveValueBlack[3][64] = {{0}};
int EngineInitializer::KingMoveValueBlack[3][64] = {{0}};

bool EngineInitializer::initialized = false;

void EngineInitializer::Initialize()
{
    if (!initialized)
    {
        PassedPawn::Generate(PassedPawnMiddleGameParameters,
                               PassedPawnMiddleGameFileAmplitude,
                               PassedPawnEndGameParameters,
                               WhitePassedPawnValueMiddleGam,
                               WhitePassedPawnValueEndGame);

        Mobility::GenerateKnight(KnightMobilityMiddleGameParameters, KnightMoveCountValueMiddleGame);
        Mobility::GenerateKnight(KnightMobilityEndGameParameters, KnightMoveCountValueEndGame);
        Mobility::GenerateBishop(BishopMobilityMiddleGameParameters, BishopMoveCountValueMiddleGame);
        Mobility::GenerateBishop(BishopMobilityEndGameParameters, BishopMoveCountValueEndGame);
        Mobility::GenerateRook(RookMobilityMiddleGameParameters, RookMoveCountValueMiddleGame);
        Mobility::GenerateRook(RookMobilityEndGameParameters, RookMoveCountValueEndGame);

        if (__builtin_expect(!UseQueenMobilityModel, 1))
        {
            Mobility::GenerateQueenMiddleGame(QueenMobilityMiddleGameParameters, QueenMoveCountValueMiddleGame);
            Mobility::GenerateQueenEndGame(QueenMobilityEndGameParameters, QueenMoveCountValueEndGame);
        }
        else
        {
            Mobility::GenerateCandQueenMiddleGame(CandQueenMobilityMiddleGameParameters, QueenMoveCountValueMiddleGame);
            Mobility::GenerateCandQueenEndGame(CandQueenMobilityEndGameParameters, QueenMoveCountValueEndGame);
        }

        if (__builtin_expect(!UsePieceSquareModel, 1))
        {
            PieceSquareModel::GeneratePawn(PawnPieceSquareMiddleGameParameters, PawnInValueWhiteMiddleGame);
            PieceSquareModel::GenerateMinor(KnightPieceSquareMiddleGameParameters, KnightInValueWhiteMiddleGame);
            PieceSquareModel::GenerateMinor(BishopPieceSquareMiddleGameParameters, BishopInValueWhiteMiddleGame);
            PieceSquareModel::GenerateMajor(RookPieceSquareMiddleGameParameters, RookInValueWhiteMiddleGame);
            PieceSquareModel::GenerateMajor(QueenPieceSquareMiddleGameParameters, QueenInValueWhiteMiddleGame);
            PieceSquareModel::GeneratePawn(PawnPieceSquareEndGameParameters, PawnInValueWhiteEndGame);
            PieceSquareModel::GenerateMinor(KnightPieceSquareEndGameParameters, KnightInValueWhiteEndGame);
            PieceSquareModel::GenerateMinor(BishopPieceSquareEndGameParameters, BishopInValueWhiteEndGame);
            PieceSquareModel::GenerateMajor(RookPieceSquareEndGameParameters, RookInValueWhiteEndGame);
            PieceSquareModel::GenerateMajor(QueenPieceSquareEndGameParameters, QueenInValueWhiteEndGame);
        }
        else
        {
            PieceSquareModel::GenerateCandPawn(CandPawnPieceSquareMiddleGameParameters, PawnInValueWhiteMiddleGame);
            PieceSquareModel::GenerateCandMinor(CandKnightPieceSquareMiddleGameParameters, KnightInValueWhiteMiddleGame);
            PieceSquareModel::GenerateMinor(CandBishopPieceSquareMiddleGameParameters, BishopInValueWhiteMiddleGame);
            PieceSquareModel::GenerateCandMajor(CandRookPieceSquareMiddleGameParameters, RookInValueWhiteMiddleGame);
            PieceSquareModel::GenerateCandMajor(CandQueenPieceSquareMiddleGameParameters, QueenInValueWhiteMiddleGame);
            PieceSquareModel::GenerateCandPawn(CandPawnPieceSquareEndGameParameters, PawnInValueWhiteEndGame);
            PieceSquareModel::GenerateCandMinor(CandKnightPieceSquareEndGameParameters, KnightInValueWhiteEndGame);
            PieceSquareModel::GenerateMinor(CandBishopPieceSquareEndGameParameters, BishopInValueWhiteEndGame);
            PieceSquareModel::GenerateCandMajor(CandRookPieceSquareEndGameParameters, RookInValueWhiteEndGame);
            PieceSquareModel::GenerateCandMajor(CandQueenPieceSquareEndGameParameters, QueenInValueWhiteEndGame);
        }
        PieceSquareModel::GenerateKing(KingPieceSquareMiddleGameParameters, KingInValueWhiteMiddleGame);
        PieceSquareModel::GenerateKing(KingPieceSquareEndGameParameters, KingInValueWhiteEndGame);

        charPowerTwo[0] = (char)1;
        charPowerTwo[1] = (char)2;
        charPowerTwo[2] = (char)4;
        charPowerTwo[3] = (char)8;
        charPowerTwo[4] = (char)16;
        charPowerTwo[5] = (char)32;
        charPowerTwo[6] = (char)64;
        charPowerTwo[7] = (char)128;
        charPowerTwoC[0] = (char)(255 - 1);
        charPowerTwoC[1] = (char)(255 - 2);
        charPowerTwoC[2] = (char)(255 - 4);
        charPowerTwoC[3] = (char)(255 - 8);
        charPowerTwoC[4] = (char)(255 - 16);
        charPowerTwoC[5] = (char)(255 - 32);
        charPowerTwoC[6] = (char)(255 - 64);
        charPowerTwoC[7] = (char)(255 - 128);
        long long biggest = 1;
        PowerTwo[0] = 1;
        for (int counter = 1; counter < 64; counter++)
        {
            PowerTwo[counter] = PowerTwo[counter - 1] << 1;
            biggest |= PowerTwo[counter];
        }
        for (int counter = 0; counter < 64; counter++)
        {
            PowerTwoComplement[counter] = biggest ^ PowerTwo[counter];
        }
        for (int counter = 0; counter < 64; counter++)
        {
            PawnInCenterValueBlack[counter] = PawnInCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            PawnMoveCenterValueBlack[counter] = PawnMoveCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            KnightInCenterValueBlack[counter] = KnightInCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            KnightMoveCenterValueBlack[counter] = KnightMoveCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            BishopInCenterValueBlack[counter] = BishopInCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            BishopMoveCenterValueBlack[counter] = BishopMoveCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            RookInCenterValueBlack[counter] = RookInCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            RookMoveCenterValueBlack[counter] = RookMoveCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            QueenInCenterValueBlack[counter] = QueenInCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            QueenMoveCenterValueBlack[counter] = QueenMoveCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            KingInCenterValueBlack[counter] = KingInCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            KingMoveCenterValueBlack[counter] = KingMoveCenterValueWhite[(7 - (counter / 8)) * 8 + counter % 8];
            const int* centerValues[2][7] = {
                {nullptr, PawnMoveCenterValueWhite, KnightMoveCenterValueWhite, BishopMoveCenterValueWhite,
                 RookMoveCenterValueWhite, QueenMoveCenterValueWhite, KingMoveCenterValueWhite},
                {nullptr, PawnMoveCenterValueBlack, KnightMoveCenterValueBlack, BishopMoveCenterValueBlack,
                 RookMoveCenterValueBlack, QueenMoveCenterValueBlack, KingMoveCenterValueBlack}
            };
            for (int side = 0; side < 2; ++side)
                for (int type = 1; type <= 6; ++type)
                    if (centerValues[side][type][counter] != 0)
                        MoveCenterNonzero[side][type] |= 1ULL << counter;
            BlackKingPlacePawnShieldMiddleGame[counter] = WhiteKingPlacePawnShieldMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            BlackKingPlaceSafetyMiddleGame[counter] = WhiteKingPlaceSafetyMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            PawnInValueBlackMiddleGame[counter] = PawnInValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            KnightInValueBlackMiddleGame[counter] = KnightInValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            BishopInValueBlackMiddleGame[counter] = BishopInValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            RookInValueBlackMiddleGame[counter] = RookInValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            QueenInValueBlackMiddleGame[counter] = QueenInValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            KingInValueBlackMiddleGame[counter] = KingInValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            PawnMoveValueBlackMiddleGame[counter] = PawnMoveValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            KnightMoveValueBlackMiddleGame[counter] = KnightMoveValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            BishopMoveValueBlackMiddleGame[counter] = BishopMoveValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            RookMoveValueBlackMiddleGame[counter] = RookMoveValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            QueenMoveValueBlackMiddleGame[counter] = QueenMoveValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            KingMoveValueBlackMiddleGame[counter] = KingMoveValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            BlackPassedPawnValueMiddleGam[counter] = WhitePassedPawnValueMiddleGam[(7 - (counter / 8)) * 8 + counter % 8];
            PawnMoveOrderingValueBlackMiddleGame[counter] = PawnMoveOrderingValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            KnightMoveOrderingValueBlackMiddleGame[counter] = KnightMoveOrderingValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            BishopMoveOrderingValueBlackMiddleGame[counter] = BishopMoveOrderingValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            RookMoveOrderingValueBlackMiddleGame[counter] = RookMoveOrderingValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            QueenMoveOrderingValueBlackMiddleGame[counter] = QueenMoveOrderingValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];
            KingMoveOrderingValueBlackMiddleGame[counter] = KingMoveOrderingValueWhiteMiddleGame[(7 - (counter / 8)) * 8 + counter % 8];

            PawnInValueBlackEndGame[counter] = PawnInValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            KnightInValueBlackEndGame[counter] = KnightInValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            BishopInValueBlackEndGame[counter] = BishopInValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            RookInValueBlackEndGame[counter] = RookInValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            QueenInValueBlackEndGame[counter] = QueenInValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            KingInValueBlackEndGame[counter] = KingInValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            PawnMoveValueBlackEndGame[counter] = PawnMoveValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            KnightMoveValueBlackEndGame[counter] = KnightMoveValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            BishopMoveValueBlackEndGame[counter] = BishopMoveValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            RookMoveValueBlackEndGame[counter] = RookMoveValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            QueenMoveValueBlackEndGame[counter] = QueenMoveValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            KingMoveValueBlackEndGame[counter] = KingMoveValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            BlackPassedPawnValueEndGam[counter] = WhitePassedPawnValueEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            PawnMoveOrderingValueBlackEndGame[counter] = PawnMoveOrderingValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            KnightMoveOrderingValueBlackEndGame[counter] = KnightMoveOrderingValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            BishopMoveOrderingValueBlackEndGame[counter] = BishopMoveOrderingValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            RookMoveOrderingValueBlackEndGame[counter] = RookMoveOrderingValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            QueenMoveOrderingValueBlackEndGame[counter] = QueenMoveOrderingValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
            KingMoveOrderingValueBlackEndGame[counter] = KingMoveOrderingValueWhiteEndGame[(7 - (counter / 8)) * 8 + counter % 8];
        }
        for (int counter = 0; counter < 64; counter++)
        {
            MoveOrderingValueWhite[0][1][counter] = PawnMoveOrderingValueWhiteMiddleGame[counter];
            MoveOrderingValueWhite[0][2][counter] = KnightMoveOrderingValueWhiteMiddleGame[counter];
            MoveOrderingValueWhite[0][3][counter] = BishopMoveOrderingValueWhiteMiddleGame[counter];
            MoveOrderingValueWhite[0][4][counter] = RookMoveOrderingValueWhiteMiddleGame[counter];
            MoveOrderingValueWhite[0][5][counter] = QueenMoveOrderingValueWhiteMiddleGame[counter];
            MoveOrderingValueWhite[0][6][counter] = KingMoveOrderingValueWhiteMiddleGame[counter];
            MoveOrderingValueWhite[2][1][counter] = PawnMoveOrderingValueWhiteEndGame[counter];
            MoveOrderingValueWhite[2][2][counter] = KnightMoveOrderingValueWhiteEndGame[counter];
            MoveOrderingValueWhite[2][3][counter] = BishopMoveOrderingValueWhiteEndGame[counter];
            MoveOrderingValueWhite[2][4][counter] = RookMoveOrderingValueWhiteEndGame[counter];
            MoveOrderingValueWhite[2][5][counter] = QueenMoveOrderingValueWhiteEndGame[counter];
            MoveOrderingValueWhite[2][6][counter] = KingMoveOrderingValueWhiteEndGame[counter];
            MoveOrderingValueBlack[0][1][counter] = PawnMoveOrderingValueBlackMiddleGame[counter];
            MoveOrderingValueBlack[0][2][counter] = KnightMoveOrderingValueBlackMiddleGame[counter];
            MoveOrderingValueBlack[0][3][counter] = BishopMoveOrderingValueBlackMiddleGame[counter];
            MoveOrderingValueBlack[0][4][counter] = RookMoveOrderingValueBlackMiddleGame[counter];
            MoveOrderingValueBlack[0][5][counter] = QueenMoveOrderingValueBlackMiddleGame[counter];
            MoveOrderingValueBlack[0][6][counter] = KingMoveOrderingValueBlackMiddleGame[counter];
            MoveOrderingValueBlack[2][1][counter] = PawnMoveOrderingValueBlackEndGame[counter];
            MoveOrderingValueBlack[2][2][counter] = KnightMoveOrderingValueBlackEndGame[counter];
            MoveOrderingValueBlack[2][3][counter] = BishopMoveOrderingValueBlackEndGame[counter];
            MoveOrderingValueBlack[2][4][counter] = RookMoveOrderingValueBlackEndGame[counter];
            MoveOrderingValueBlack[2][5][counter] = QueenMoveOrderingValueBlackEndGame[counter];
            MoveOrderingValueBlack[2][6][counter] = KingMoveOrderingValueBlackEndGame[counter];
        }
        for (int counter = 0; counter < 16; counter++)
        {
            AttackValueMovement[1][counter] = WhitePawnAttackValueMovement[counter];
            AttackValueMovement[2][counter] = WhiteKnightAttackValueMovement[counter];
            AttackValueMovement[3][counter] = WhiteBishopAttackValueMovement[counter];
            AttackValueMovement[4][counter] = WhiteRookAttackValueMovement[counter];
            AttackValueMovement[5][counter] = WhiteQueenAttackValueMovement[counter];
            AttackValueMovement[6][counter] = WhiteKingAttackValueMovement[counter];
            AttackValueMovement[9][counter] = BlackPawnAttackValueMovement[counter];
            AttackValueMovement[10][counter] = BlackKnightAttackValueMovement[counter];
            AttackValueMovement[11][counter] = BlackBishopAttackValueMovement[counter];
            AttackValueMovement[12][counter] = BlackRookAttackValueMovement[counter];
            AttackValueMovement[13][counter] = BlackQueenAttackValueMovement[counter];
            AttackValueMovement[14][counter] = BlackKingAttackValueMovement[counter];
        }
        for (int counter = 0; counter < 16; counter++)
        {
            PawnAttackValue[0][counter] = PawnAttackValueMiddleGame[counter];
            PawnAttackValue[2][counter] = PawnAttackValueEndGame[counter];
            KnightAttackValue[0][counter] = KnightAttackValueMiddleGame[counter];
            KnightAttackValue[2][counter] = KnightAttackValueEndGame[counter];
            BishopAttackValue[0][counter] = BishopAttackValueMiddleGame[counter];
            BishopAttackValue[2][counter] = BishopAttackValueEndGame[counter];
            RookAttackValue[0][counter] = RookAttackValueMiddleGame[counter];
            RookAttackValue[2][counter] = RookAttackValueEndGame[counter];
            QueenAttackValue[0][counter] = QueenAttackValueMiddleGame[counter];
            QueenAttackValue[2][counter] = QueenAttackValueEndGame[counter];
            KingAttackValue[0][counter] = KingAttackValueMiddleGame[counter];
            KingAttackValue[2][counter] = KingAttackValueEndGame[counter];
        }
        for (int counter = 0; counter < 3; counter++)
        {
            PawnMoveCountValue[0][counter] = PawnMoveCountValueMiddleGame[counter];
            PawnMoveCountValue[2][counter] = PawnMoveCountValueEndGame[counter];
        }
        for (int counter = 0; counter < 9; counter++)
        {
            KnightMoveCountValue[0][counter] = KnightMoveCountValueMiddleGame[counter];
            KnightMoveCountValue[2][counter] = KnightMoveCountValueEndGame[counter];
        }
        for (int counter = 0; counter < 14; counter++)
        {
            BishopMoveCountValue[0][counter] = BishopMoveCountValueMiddleGame[counter];
            BishopMoveCountValue[2][counter] = BishopMoveCountValueEndGame[counter];
        }
        for (int counter = 0; counter < 15; counter++)
        {
            RookMoveCountValue[0][counter] = RookMoveCountValueMiddleGame[counter];
            RookMoveCountValue[2][counter] = RookMoveCountValueEndGame[counter];
        }
        for (int counter = 0; counter < 28; counter++)
        {
            QueenMoveCountValue[0][counter] = QueenMoveCountValueMiddleGame[counter];
            QueenMoveCountValue[2][counter] = QueenMoveCountValueEndGame[counter];
        }
        for (int counter = 0, counterEnd = 0; counter < 9; counter++, counterEnd += 8)
        {
            KingMoveCountValue[0][counter] = KingMoveCountValueMiddleGame[counter];
            KingMoveCountValue[2][counter] = KingMoveCountValueEndGame[counter];
        }
        for (int counter = 0; counter < 16; counter++)
        {
            PieceArroundTheKing[0][counter] = PieceArroundTheKingMiddleGame[counter];
            PieceArroundTheKing[2][counter] = PieceArroundTheKingEndGame[counter];
        }
        for (int counter = 0; counter < 16; counter++)
        {
            PieceAttackArroundTheKing[0][counter] = PieceAttackArroundTheKingMiddleGame[counter];
            PieceAttackArroundTheKing[2][counter] = PieceAttackArroundTheKingEndGame[counter];
        }
        for (int counter = 0; counter < 64; counter++)
        {
            PawnInValueWhite[0][counter] = PawnInValueWhiteMiddleGame[counter];
            PawnInValueWhite[2][counter] = PawnInValueWhiteEndGame[counter];
            KnightInValueWhite[0][counter] = KnightInValueWhiteMiddleGame[counter];
            KnightInValueWhite[2][counter] = KnightInValueWhiteEndGame[counter];
            BishopInValueWhite[0][counter] = BishopInValueWhiteMiddleGame[counter];
            BishopInValueWhite[2][counter] = BishopInValueWhiteEndGame[counter];
            RookInValueWhite[0][counter] = RookInValueWhiteMiddleGame[counter];
            RookInValueWhite[2][counter] = RookInValueWhiteEndGame[counter];
            QueenInValueWhite[0][counter] = QueenInValueWhiteMiddleGame[counter];
            QueenInValueWhite[2][counter] = QueenInValueWhiteEndGame[counter];
            KingInValueWhite[0][counter] = KingInValueWhiteMiddleGame[counter];
            KingInValueWhite[2][counter] = KingInValueWhiteEndGame[counter];
        }
        for (int counter = 0; counter < 64; counter++)
        {
            PawnMoveValueWhite[0][counter] = PawnMoveValueWhiteMiddleGame[counter];
            PawnMoveValueWhite[2][counter] = PawnMoveValueWhiteEndGame[counter];
            KnightMoveValueWhite[0][counter] = KnightMoveValueWhiteMiddleGame[counter];
            KnightMoveValueWhite[2][counter] = KnightMoveValueWhiteEndGame[counter];
            BishopMoveValueWhite[0][counter] = BishopMoveValueWhiteMiddleGame[counter];
            BishopMoveValueWhite[2][counter] = BishopMoveValueWhiteEndGame[counter];
            RookMoveValueWhite[0][counter] = RookMoveValueWhiteMiddleGame[counter];
            RookMoveValueWhite[2][counter] = RookMoveValueWhiteEndGame[counter];
            QueenMoveValueWhite[0][counter] = QueenMoveValueWhiteMiddleGame[counter];
            QueenMoveValueWhite[2][counter] = QueenMoveValueWhiteEndGame[counter];
            KingMoveValueWhite[0][counter] = KingMoveValueWhiteMiddleGame[counter];
            KingMoveValueWhite[2][counter] = KingMoveValueWhiteEndGame[counter];
        }
        for (int counter = 0; counter < 64; counter++)
        {
            PawnInValueBlack[0][counter] = PawnInValueBlackMiddleGame[counter];
            PawnInValueBlack[2][counter] = PawnInValueBlackEndGame[counter];
            KnightInValueBlack[0][counter] = KnightInValueBlackMiddleGame[counter];
            KnightInValueBlack[2][counter] = KnightInValueBlackEndGame[counter];
            BishopInValueBlack[0][counter] = BishopInValueBlackMiddleGame[counter];
            BishopInValueBlack[2][counter] = BishopInValueBlackEndGame[counter];
            RookInValueBlack[0][counter] = RookInValueBlackMiddleGame[counter];
            RookInValueBlack[2][counter] = RookInValueBlackEndGame[counter];
            QueenInValueBlack[0][counter] = QueenInValueBlackMiddleGame[counter];
            QueenInValueBlack[2][counter] = QueenInValueBlackEndGame[counter];
            KingInValueBlack[0][counter] = KingInValueBlackMiddleGame[counter];
            KingInValueBlack[2][counter] = KingInValueBlackEndGame[counter];
        }
        for (int counter = 0; counter < 64; counter++)
        {
            PawnMoveValueBlack[0][counter] = PawnMoveValueBlackMiddleGame[counter];
            PawnMoveValueBlack[2][counter] = PawnMoveValueBlackEndGame[counter];
            KnightMoveValueBlack[0][counter] = KnightMoveValueBlackMiddleGame[counter];
            KnightMoveValueBlack[2][counter] = KnightMoveValueBlackEndGame[counter];
            BishopMoveValueBlack[0][counter] = BishopMoveValueBlackMiddleGame[counter];
            BishopMoveValueBlack[2][counter] = BishopMoveValueBlackEndGame[counter];
            RookMoveValueBlack[0][counter] = RookMoveValueBlackMiddleGame[counter];
            RookMoveValueBlack[2][counter] = RookMoveValueBlackEndGame[counter];
            QueenMoveValueBlack[0][counter] = QueenMoveValueBlackMiddleGame[counter];
            QueenMoveValueBlack[2][counter] = QueenMoveValueBlackEndGame[counter];
            KingMoveValueBlack[0][counter] = KingMoveValueBlackMiddleGame[counter];
            KingMoveValueBlack[2][counter] = KingMoveValueBlackEndGame[counter];
        }

        initialized = true;
    }
}
