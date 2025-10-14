#ifndef KINGSETUP_H
#define KINGSETUP_H

#include <vector>

class KingSetup
{
public:
    // Distance To King
    static int DistanceToKing[64][64];

    // KingInFront
    static std::vector<int> WhiteKingInFront[64];
    static std::vector<int> BlackKingInFront[64];

    static void Initialize();
    static void Cleanup();

private:
    static bool initialized;

    static void setDistanceToKing();
    static void setKingInFront();
};

#endif