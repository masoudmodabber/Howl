#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "Option.h"

char Option::charPowerTwo[8] = {};
char Option::charPowerTwoC[8] = {};
int Option::MultiPV = 1;
int Option::nullWindowSize = 1;
int Option::checkExtension = 4;
int Option::checkExtensionNonPV = 1;
int Option::SafetyMargin = 100;
int Option::reductiondepth = 4;
int Option::futilityMargin = 20;
int Option::extendedFutilityMargin = 40;
int Option::superExtendedFutilityMargin = 80;
int Option::PawnValue = 100;
int Option::KnightValue = 350;
int Option::BishopValue = 350;
int Option::RookValue = 550;
int Option::QueenValue = 975;
int Option::KingValue = 1200;
int Option::EndPawnValue = -20;
int Option::DoubledPawnValue = -20;
int Option::pieceMovement[2][7][120] = {};

int Option::WhitePassedPawnValueMiddleGam[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                               5, 8, 10, 12, 12, 10, 8, 5,
                                               8, 10, 12, 15, 15, 12, 10, 8,
                                               10, 12, 15, 18, 18, 15, 12, 10,
                                               12, 15, 18, 20, 20, 18, 15, 12,
                                               15, 18, 20, 22, 22, 20, 18, 15,
                                               18, 20, 22, 25, 25, 22, 20, 18,
                                               0, 0, 0, 0, 0, 0, 0, 0};

int Option::PawnInValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                            -20, 0, 0, 0, 0, 0, 0, -20,
                                            -20, 0, 0, 0, 0, 0, 0, -20,
                                            -20, 0, 0, 0, 0, 0, 0, -20,
                                            -20, 0, 0, 0, 0, 0, 0, -20,
                                            -20, 0, 0, 0, 0, 0, 0, -20,
                                            -20, 0, 0, 0, 0, 0, 0, -20,
                                            0, 0, 0, 0, 0, 0, 0, 0};

int Option::KnightInValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 4, 8, 8, 4, 0, 0,
                                              0, 4, 17, 26, 26, 17, 4, 0,
                                              0, 8, 26, 35, 35, 26, 8, 0,
                                              0, 4, 17, 17, 17, 17, 4, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0};

int Option::BishopInValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 5, 5, 5, 5, 0, 0,
                                              0, 5, 10, 10, 10, 10, 5, 0,
                                              0, 10, 21, 21, 21, 21, 10, 0,
                                              0, 5, 8, 8, 8, 8, 5, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0};

int Option::RookInValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            47, 47, 47, 47, 47, 47, 47, 47,
                                            0, 0, 0, 0, 0, 0, 0, 0};

int Option::QueenInValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             27, 27, 27, 27, 27, 27, 27, 27,
                                             0, 0, 0, 0, 0, 0, 0, 0};

int Option::KingInValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0};

int Option::PawnMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 1, 0, 0, 0, 0, 0,
                                                      0, 0, 3, 7, 7, 1, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int Option::KnightMoveOrderingValueWhiteMiddleGame[] = {-6, -5, -4, -3, -3, -4, -5, -6,
                                                        -4, -3, -2, -1, -1, -2, -3, -4,
                                                        0, 0, 2, 0, 0, 2, 0, 0,
                                                        0, 0, 3, 7, 7, 3, 0, 0,
                                                        0, 5, 10, 15, 15, 10, 5, 0,
                                                        0, 10, 15, 20, 20, 15, 10, 0,
                                                        0, 5, 10, 15, 15, 10, 5, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int Option::BishopMoveOrderingValueWhiteMiddleGame[] = {-5, -4, -3, -2, -2, -3, -4, -5,
                                                        0, 1, 0, 0, 0, 0, 1, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 2, 6, 6, 2, 0, 0,
                                                        0, 4, 8, 10, 10, 8, 4, 0,
                                                        0, 8, 10, 12, 12, 10, 8, 0,
                                                        0, 4, 8, 10, 10, 8, 4, 0};

int Option::RookMoveOrderingValueWhiteMiddleGame[] = {0, -8, 1, 0, 0, 1, -8, 0,
                                                      -8, -8, -8, -8, -8, -8, -8, -8,
                                                      -8, -8, -8, -8, -8, -8, -8, -8,
                                                      -8, -8, -8, -8, -8, -8, -8, -8,
                                                      -8, -8, -8, -8, -8, -8, -8, -8,
                                                      -8, -8, -8, -8, -8, -8, -8, -8,
                                                      25, 25, 25, 25, 25, 25, 25, 25,
                                                      5, 5, 5, 5, 5, 5, 5, 5};

int Option::QueenMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       5, 5, 5, 5, 5, 5, 5, 5,
                                                       0, 0, 0, 0, 0, 0, 0, 0};

int Option::KingMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

// till820
/*
int Option::PawnMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int Option::KnightMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int Option::BishopMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int Option::RookMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int Option::QueenMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0};

int Option::KingMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int Option::PawnMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int Option::KnightMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int Option::BishopMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int Option::RookMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int Option::QueenMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0};

int Option::KingMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int Option::PawnMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int Option::KnightMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int Option::BishopMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0,
                                                        0, 0, 0, 0, 0, 0, 0, 0};

int Option::RookMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};

int Option::QueenMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0, 0, 0, 0};

int Option::KingMoveOrderingValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 0};
*/
int Option::PawnMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0};

int Option::KnightMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0};

int Option::BishopMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0, 0, 0};

int Option::RookMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0};

int Option::QueenMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0, 0, 0, 0};

int Option::KingMoveValueWhiteMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0};

int Option::PawnMoveValueWhite[3][64] = {};
int Option::KnightMoveValueWhite[3][64] = {};
int Option::BishopMoveValueWhite[3][64] = {};
int Option::RookMoveValueWhite[3][64] = {};
int Option::QueenMoveValueWhite[3][64] = {};
int Option::KingMoveValueWhite[3][64] = {};

int Option::BlackPassedPawnValueMiddleGam[64] = {};
int Option::PawnInValueBlackMiddleGame[64] = {};
int Option::KnightInValueBlackMiddleGame[64] = {};
int Option::BishopInValueBlackMiddleGame[64] = {};
int Option::RookInValueBlackMiddleGame[64] = {};
int Option::QueenInValueBlackMiddleGame[64] = {};
int Option::KingInValueBlackMiddleGame[64] = {};

int Option::PawnMoveOrderingValueBlackMiddleGame[64] = {};
int Option::KnightMoveOrderingValueBlackMiddleGame[64] = {};
int Option::BishopMoveOrderingValueBlackMiddleGame[64] = {};
int Option::RookMoveOrderingValueBlackMiddleGame[64] = {};
int Option::QueenMoveOrderingValueBlackMiddleGame[64] = {};
int Option::KingMoveOrderingValueBlackMiddleGame[64] = {};

int Option::PawnMoveValueBlackMiddleGame[64] = {};
int Option::KnightMoveValueBlackMiddleGame[64] = {};
int Option::BishopMoveValueBlackMiddleGame[64] = {};
int Option::RookMoveValueBlackMiddleGame[64] = {};
int Option::QueenMoveValueBlackMiddleGame[64] = {};
int Option::KingMoveValueBlackMiddleGame[64] = {};

int Option::PawnMoveCountValueMiddleGame[3] = {0, 0, 0};
int Option::PawnMoveCountValue[3][3] = {};

int Option::KnightMoveCountValueMiddleGame[9] = {-38, -25, -12, 0, 12, 25, 31, 38, 38};
int Option::KnightMoveCountValue[3][9] = {};
int Option::BishopMoveCountValueMiddleGame[16] = {-25, -11, 3, 17, 31, 45, 57, 65, 71, 74, 76, 78, 79, 80, 81, 81};
int Option::BishopMoveCountValue[3][16] = {};
int Option::RookMoveCountValueMiddleGame[16] = {-20, -14, -8, -2, 4, 10, 14, 19, 23, 26, 27, 28, 29, 30, 31, 32};
int Option::RookMoveCountValue[3][16] = {};
int Option::QueenMoveCountValueMiddleGame[33] = {-10, -8, -6, -3, -1, 1, 3, 5, 8, 10, 12, 15, 16, 17, 18, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20};
int Option::QueenMoveCountValue[3][33] = {};
int Option::KingMoveCountValueMiddleGame[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int Option::KingMoveCountValue[3][9] = {};

int Option::PawnAttackValueMiddleGame[16] = {0, 0, 56, 56, 76, 86, 0, 0, 0, 0, 56, 56, 76, 86, 0, 0};
int Option::PawnAttackValue[3][16] = {};
int Option::KnightAttackValueMiddleGame[16] = {0, 7, 0, 24, 41, 41, 0, 0, 0, 7, 0, 24, 41, 41, 0, 0};
int Option::KnightAttackValue[3][16] = {};
int Option::BishopAttackValueMiddleGame[16] = {0, 7, 24, 0, 41, 41, 0, 0, 0, 7, 24, 0, 41, 41, 0, 0};
int Option::BishopAttackValue[3][16] = {};
int Option::RookAttackValueMiddleGame[16] = {0, -1, 15, 15, 0, 24, 0, 0, 0, -1, 15, 15, 0, 24, 0, 0};
int Option::RookAttackValue[3][16] = {};
int Option::QueenAttackValueMiddleGame[16] = {0, 15, 15, 15, 15, 0, 0, 0, 0, 15, 15, 15, 15, 0, 0, 0};
int Option::QueenAttackValue[3][16] = {};
int Option::KingAttackValueMiddleGame[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int Option::KingAttackValue[3][16] = {};

int Option::WhitePawnAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 35, 10, 10, 20, 40, 0, 0};
int Option::WhiteKnightAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 50, 50, 10, 20, 0, 0};
int Option::WhiteBishopAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 50, 50, 10, 20, 0, 0};
int Option::WhiteRookAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 25, 35, 50, 150, 0, 0};
int Option::WhiteQueenAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 20, 35, 50, 10, 0, 0};
int Option::WhiteKingAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int Option::BlackPawnAttackValueMovement[16] = {0, 35, 10, 10, 20, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int Option::BlackKnightAttackValueMovement[16] = {0, 15, 50, 50, 10, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int Option::BlackBishopAttackValueMovement[16] = {0, 15, 50, 50, 10, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int Option::BlackRookAttackValueMovement[16] = {0, 10, 25, 35, 50, 150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int Option::BlackQueenAttackValueMovement[16] = {0, 5, 20, 35, 50, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int Option::BlackKingAttackValueMovement[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int Option::WhitePassedPawnValueEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                             30, 50, 70, 60, 60, 70, 50, 30,
                                             40, 60, 80, 70, 70, 80, 60, 40,
                                             50, 70, 90, 80, 80, 90, 70, 50,
                                             60, 80, 100, 90, 90, 100, 80, 60,
                                             70, 90, 110, 100, 100, 110, 90, 70,
                                             80, 100, 120, 110, 110, 120, 100, 80,
                                             0, 0, 0, 0, 0, 0, 0, 0};

int Option::PawnInValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                         -20, 0, 0, 0, 0, 0, 0, -20,
                                         -20, 0, 0, 0, 0, 0, 0, -20,
                                         -20, 0, 0, 0, 0, 0, 0, -20,
                                         -20, 0, 0, 0, 0, 0, 0, -20,
                                         -20, 0, 0, 0, 0, 0, 0, -20,
                                         -20, 0, 0, 0, 0, 0, 0, -20,
                                         0, 0, 0, 0, 0, 0, 0, 0};

int Option::KnightInValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 4, 8, 8, 4, 0, 0,
                                           0, 4, 17, 26, 26, 17, 4, 0,
                                           0, 8, 26, 35, 35, 26, 8, 0,
                                           0, 4, 17, 17, 17, 17, 4, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

int Option::BishopInValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 5, 5, 5, 5, 0, 0,
                                           0, 5, 10, 10, 10, 10, 5, 0,
                                           0, 10, 21, 21, 21, 21, 10, 0,
                                           0, 5, 8, 8, 8, 8, 5, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

int Option::RookInValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         98, 98, 98, 98, 98, 98, 98, 98,
                                         0, 0, 0, 0, 0, 0, 0, 0};

int Option::QueenInValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          54, 54, 54, 54, 54, 54, 54, 54,
                                          0, 0, 0, 0, 0, 0, 0, 0};

int Option::KingInValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0};

int Option::PawnMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   -20, 0, 0, 0, 0, 0, 0, -20,
                                                   0, 0, 0, 0, 0, 0, 0, 0};

int Option::KnightMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 4, 8, 8, 4, 0, 0,
                                                     0, 4, 17, 26, 26, 17, 4, 0,
                                                     0, 8, 26, 35, 35, 26, 8, 0,
                                                     0, 4, 17, 17, 17, 17, 4, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0};

int Option::BishopMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 5, 5, 5, 5, 0, 0,
                                                     0, 5, 10, 10, 10, 10, 5, 0,
                                                     0, 10, 21, 21, 21, 21, 10, 0,
                                                     0, 5, 8, 8, 8, 8, 5, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0};

int Option::RookMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   47, 47, 47, 47, 47, 47, 47, 47,
                                                   0, 0, 0, 0, 0, 0, 0, 0};

int Option::QueenMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0,
                                                    27, 27, 27, 27, 27, 27, 27, 27,
                                                    0, 0, 0, 0, 0, 0, 0, 0};

int Option::KingMoveOrderingValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0, 0, 0, 0};

int Option::PawnMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

int Option::KnightMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0};

int Option::BishopMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0, 0, 0};

int Option::RookMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

int Option::QueenMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0};

int Option::KingMoveValueWhiteEndGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

int Option::PawnInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 10, 15, 15, 10, 0, 0,
                                        0, 0, 0, 5, 5, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0};

int Option::PawnInValueWhite[3][64] = {{0}};
int Option::KnightInValueWhite[3][64] = {{0}};
int Option::BishopInValueWhite[3][64] = {{0}};
int Option::RookInValueWhite[3][64] = {{0}};
int Option::QueenInValueWhite[3][64] = {{0}};
int Option::KingInValueWhite[3][64] = {{0}};

int Option::PawnMoveCenterValueBlack[64] = {0};
int Option::PawnInCenterValueBlack[64] = {0};
int Option::KnightMoveCenterValueBlack[64] = {0};
int Option::KnightInCenterValueBlack[64] = {0};
int Option::BishopMoveCenterValueBlack[64] = {0};
int Option::BishopInCenterValueBlack[64] = {0};
int Option::RookMoveCenterValueBlack[64] = {0};
int Option::RookInCenterValueBlack[64] = {0};
int Option::QueenMoveCenterValueBlack[64] = {0};
int Option::QueenInCenterValueBlack[64] = {0};
int Option::KingMoveCenterValueBlack[64] = {0};
int Option::KingInCenterValueBlack[64] = {0};

int Option::PawnMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 2, 4, 4, 2, 0, 0,
                                          0, 0, 3, 7, 7, 3, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0};

int Option::KnightInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 2, 2, 2, 2, 0, 0,
                                          0, 0, 2, 4, 4, 2, 0, 0,
                                          0, 0, 2, 4, 4, 2, 0, 0,
                                          0, 0, 2, 2, 2, 2, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0};

int Option::KnightMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 2, 2, 2, 2, 0, 0,
                                            0, 0, 2, 4, 4, 2, 0, 0,
                                            0, 0, 2, 4, 4, 2, 0, 0,
                                            0, 0, 2, 2, 2, 2, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0};

int Option::BishopInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 2, 2, 2, 2, 0, 0,
                                          0, 0, 2, 3, 3, 2, 0, 0,
                                          0, 0, 2, 3, 3, 2, 0, 0,
                                          0, 0, 2, 2, 2, 2, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0};

int Option::BishopMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 2, 2, 2, 2, 0, 0,
                                            0, 0, 2, 3, 3, 2, 0, 0,
                                            0, 0, 2, 3, 3, 2, 0, 0,
                                            0, 0, 2, 2, 2, 2, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0};

int Option::RookInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 1, 1, 1, 1, 0, 0,
                                        0, 0, 1, 2, 2, 1, 0, 0,
                                        0, 0, 1, 2, 2, 1, 0, 0,
                                        0, 0, 1, 1, 1, 1, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0};

int Option::RookMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 1, 1, 1, 1, 0, 0,
                                          0, 0, 1, 2, 2, 1, 0, 0,
                                          0, 0, 1, 2, 2, 1, 0, 0,
                                          0, 0, 1, 1, 1, 1, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0};
int Option::QueenInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 1, 1, 1, 1, 0, 0,
                                         0, 0, 1, 2, 2, 1, 0, 0,
                                         0, 0, 1, 2, 2, 1, 0, 0,
                                         0, 0, 1, 1, 1, 1, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0,
                                         0, 0, 0, 0, 0, 0, 0, 0};

int Option::QueenMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 1, 1, 1, 1, 0, 0,
                                           0, 0, 1, 2, 2, 1, 0, 0,
                                           0, 0, 1, 2, 2, 1, 0, 0,
                                           0, 0, 1, 1, 1, 1, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};

int Option::KingInCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0};

int Option::KingMoveCenterValueWhite[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0};
int Option::BlackPassedPawnValueEndGam[64] = {0};

int Option::PawnInValueBlackEndGame[64] = {0};
int Option::KnightInValueBlackEndGame[64] = {0};
int Option::BishopInValueBlackEndGame[64] = {0};
int Option::RookInValueBlackEndGame[64] = {0};
int Option::QueenInValueBlackEndGame[64] = {0};
int Option::KingInValueBlackEndGame[64] = {0};

int Option::PawnMoveOrderingValueBlackEndGame[64] = {0};
int Option::KnightMoveOrderingValueBlackEndGame[64] = {0};
int Option::BishopMoveOrderingValueBlackEndGame[64] = {0};
int Option::RookMoveOrderingValueBlackEndGame[64] = {0};
int Option::QueenMoveOrderingValueBlackEndGame[64] = {0};
int Option::KingMoveOrderingValueBlackEndGame[64] = {0};

int Option::PawnMoveValueBlackEndGame[64] = {0};
int Option::KnightMoveValueBlackEndGame[64] = {0};
int Option::BishopMoveValueBlackEndGame[64] = {0};
int Option::RookMoveValueBlackEndGame[64] = {0};
int Option::QueenMoveValueBlackEndGame[64] = {0};
int Option::KingMoveValueBlackEndGame[64] = {0};

int Option::PawnMoveCountValueEndGame[3] = {0};
int Option::KnightMoveCountValueEndGame[9] = {-33, -23, -13, -3, 7, 17, 22, 27, 27};
int Option::BishopMoveCountValueEndGame[16] = {-30, -16, -2, 12, 26, 40, 52, 60, 65, 69, 71, 73, 74, 75, 76, 76};
int Option::RookMoveCountValueEndGame[16] = {-36, -19, -3, 13, 29, 46, 62, 79, 95, 16, 111, 114, 116, 117, 118, 118};
int Option::QueenMoveCountValueEndGame[33] = {-18, -13, -7, -2, 3, 8, 13, 19, 23, 27, 32, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35};
int Option::KingMoveCountValueEndGame[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

int Option::PawnAttackValueEndGame[16] = {0, 0, 70, 70, 99, 118, 0, 0, 0, 0, 70, 70, 99, 118, 0, 0};
int Option::KnightAttackValueEndGame[16] = {0, 39, 0, 49, 10, 10, 0, 0, 0, 39, 0, 49, 10, 10, 0, 0};
int Option::BishopAttackValueEndGame[16] = {0, 39, 49, 0, 10, 10, 0, 0, 0, 39, 49, 0, 10, 10, 0, 0};
int Option::RookAttackValueEndGame[16] = {0, 29, 49, 49, 0, 49, 0, 0, 0, 29, 49, 49, 0, 49, 0, 0};
int Option::QueenAttackValueEndGame[16] = {0, 39, 39, 39, 39, 0, 0, 0, 0, 39, 39, 39, 39, 0, 0, 0};
int Option::KingAttackValueEndGame[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int Option::PieceArroundTheKingMiddleGame[16] = {0, 5, 10, 10, 12, 25, 0, 0, 0, 5, 10, 10, 12, 25, 0, 0};
int Option::PieceArroundTheKing[3][16] = {{0}};
int Option::PieceArroundTheKingEndGame[16] = {0, 5, 10, 10, 12, 25, 0, 0, 0, 5, 10, 10, 12, 25, 0, 0};
int Option::PieceAttackArroundTheKingMiddleGame[16] = {0, 1, 3, 3, 5, 9, 0, 0, 0, 1, 3, 3, 5, 9, 0, 0};
int Option::PieceAttackArroundTheKing[3][16] = {{0}};
int Option::PieceAttackArroundTheKingEndGame[16] = {0, 1, 3, 3, 5, 9, 0, 0, 0, 1, 3, 3, 5, 9, 0, 0};

int Option::WhiteKingPlaceSafetyMiddleGame[] = {-4, -0, -6, -20, -15, -6, -0, -4,
                                                -8, -6, -12, -20, -20, -12, -6, -8,
                                                -18, -18, -22, -30, -30, -22, -18, -18,
                                                -35, -35, -35, -35, -35, -35, -35, -35,
                                                -35, -35, -35, -35, -35, -35, -35, -35,
                                                -35, -35, -35, -35, -35, -35, -35, -35,
                                                -35, -35, -35, -35, -35, -35, -35, -35,
                                                -35, -35, -35, -35, -35, -35, -35, -35};
int Option::WhiteKingPlacePawnShieldMiddleGame[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                    -7, -7, -11, -9, -9, -11, -7, -7,
                                                    -16, -16, -16, -16, -16, -16, -16, -16,
                                                    -16, -16, -16, -16, -16, -16, -16, -16,
                                                    -16, -16, -16, -16, -16, -16, -16, -16,
                                                    -16, -16, -16, -16, -16, -16, -16, -16,
                                                    -16, -16, -16, -16, -16, -16, -16, -16,
                                                    -16, -16, -16, -16, -16, -16, -16, -16};

int Option::BlackKingPlaceSafetyMiddleGame[64] = {0};
int Option::BlackKingPlacePawnShieldMiddleGame[64] = {0};

int Option::ArroundTheKingDangerMiddleGame[4] = {0, 5, 3, 1};
int Option::ArroundTheKingDangerEndGame[4] = {0, 5, 3, 1};

int Option::MoveOrderingValueWhite[3][7][64] = {{{0}}};
int Option::MoveOrderingValueBlack[3][7][64] = {{{0}}};

int Option::AttackValueMovement[15][16] = {{0}};

long long Option::PowerTwo[64] = {0};
long long Option::PowerTwoComplement[64] = {0};

int Option::PawnInValueBlack[3][64] = {{0}};
int Option::KnightInValueBlack[3][64] = {{0}};
int Option::BishopInValueBlack[3][64] = {{0}};
int Option::RookInValueBlack[3][64] = {{0}};
int Option::QueenInValueBlack[3][64] = {{0}};
int Option::KingInValueBlack[3][64] = {{0}};

int Option::PawnMoveValueBlack[3][64] = {{0}};
int Option::KnightMoveValueBlack[3][64] = {{0}};
int Option::BishopMoveValueBlack[3][64] = {{0}};
int Option::RookMoveValueBlack[3][64] = {{0}};
int Option::QueenMoveValueBlack[3][64] = {{0}};
int Option::KingMoveValueBlack[3][64] = {{0}};
// public static int[] AttackArroundTheKingDangerMiddleGame = new int[4];
// public static int[] AttackArroundTheKingDangerEndGame = new int[4];

bool Option::initialized = false;

void Option::Initialize()
{
    if (!initialized)
    {
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
        for (int counter = 0; counter < 16; counter++)
        {
            BishopMoveCountValue[0][counter] = BishopMoveCountValueMiddleGame[counter];
            BishopMoveCountValue[2][counter] = BishopMoveCountValueEndGame[counter];
        }
        for (int counter = 0; counter < 16; counter++)
        {
            RookMoveCountValue[0][counter] = RookMoveCountValueMiddleGame[counter];
            RookMoveCountValue[2][counter] = RookMoveCountValueEndGame[counter];
        }
        for (int counter = 0; counter < 33; counter++)
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
