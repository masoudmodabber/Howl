#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
// BoardInitializer.cpp
#include "BoardInitializer.h"
#include <climits>

Board *BoardInitializer::beginBoard = nullptr;
long long BoardInitializer::ZCode[16][64];
long long BoardInitializer::ZCodeFlag[8];
long long BoardInitializer::ZCodeUnpassentPlace[64];
bool BoardInitializer::initialized = false;

void BoardInitializer::Initialize()
{
    if (!initialized)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<long long> dis(LLONG_MIN, LLONG_MAX);

        for (int counter = 0; counter < 64; counter++)
        {
            for (int counter2 = 0; counter2 < 16; counter2++)
                ZCode[counter2][counter] = dis(gen);
        }
        for (int counter = 0; counter < 8; counter++)
        {
            ZCodeFlag[counter] = dis(gen);
        }
        for (int counter = 1; counter < 64; counter++)
        {
            ZCodeUnpassentPlace[counter] = dis(gen);
        }

        beginBoard = new Board();
        beginBoard->pieces[4].push_back(0);
        beginBoard->mainBoard[0] = 4;
        beginBoard->pieces[2].push_back(1);
        beginBoard->mainBoard[1] = 2;
        beginBoard->pieces[3].push_back(2);
        beginBoard->mainBoard[2] = 3;
        beginBoard->pieces[5].push_back(3);
        beginBoard->mainBoard[3] = 5;
        beginBoard->pieces[6].push_back(4);
        beginBoard->mainBoard[4] = 6;
        beginBoard->pieces[3].push_back(5);
        beginBoard->mainBoard[5] = 3;
        beginBoard->pieces[2].push_back(6);
        beginBoard->mainBoard[6] = 2;
        beginBoard->pieces[4].push_back(7);
        beginBoard->mainBoard[7] = 4;
        beginBoard->pieces[1].push_back(8);
        beginBoard->mainBoard[8] = 1;
        beginBoard->pieces[1].push_back(9);
        beginBoard->mainBoard[9] = 1;
        beginBoard->pieces[1].push_back(10);
        beginBoard->mainBoard[10] = 1;
        beginBoard->pieces[1].push_back(11);
        beginBoard->mainBoard[11] = 1;
        beginBoard->pieces[1].push_back(12);
        beginBoard->mainBoard[12] = 1;
        beginBoard->pieces[1].push_back(13);
        beginBoard->mainBoard[13] = 1;
        beginBoard->pieces[1].push_back(14);
        beginBoard->mainBoard[14] = 1;
        beginBoard->pieces[1].push_back(15);
        beginBoard->mainBoard[15] = 1;
        beginBoard->pieces[9].push_back(48);
        beginBoard->mainBoard[48] = 9;
        beginBoard->pieces[9].push_back(49);
        beginBoard->mainBoard[49] = 9;
        beginBoard->pieces[9].push_back(50);
        beginBoard->mainBoard[50] = 9;
        beginBoard->pieces[9].push_back(51);
        beginBoard->mainBoard[51] = 9;
        beginBoard->pieces[9].push_back(52);
        beginBoard->mainBoard[52] = 9;
        beginBoard->pieces[9].push_back(53);
        beginBoard->mainBoard[53] = 9;
        beginBoard->pieces[9].push_back(54);
        beginBoard->mainBoard[54] = 9;
        beginBoard->pieces[9].push_back(55);
        beginBoard->mainBoard[55] = 9;
        beginBoard->pieces[12].push_back(56);
        beginBoard->mainBoard[56] = 12;
        beginBoard->pieces[10].push_back(57);
        beginBoard->mainBoard[57] = 10;
        beginBoard->pieces[11].push_back(58);
        beginBoard->mainBoard[58] = 11;
        beginBoard->pieces[13].push_back(59);
        beginBoard->mainBoard[59] = 13;
        beginBoard->pieces[14].push_back(60);
        beginBoard->mainBoard[60] = 14;
        beginBoard->pieces[11].push_back(61);
        beginBoard->mainBoard[61] = 11;
        beginBoard->pieces[10].push_back(62);
        beginBoard->mainBoard[62] = 10;
        beginBoard->pieces[12].push_back(63);
        beginBoard->mainBoard[63] = 12;
        // Side To Move
        beginBoard->sideToMove = false;
        // White King Castle
        beginBoard->whiteSmallCastle = true;
        beginBoard->ZobristHashCode ^= BoardInitializer::ZCodeFlag[6];
        // White Queen Castle
        beginBoard->whiteBigCastle = true;
        beginBoard->ZobristHashCode ^= BoardInitializer::ZCodeFlag[5];
        // Black King Castle
        beginBoard->blackSmallCastle = true;
        beginBoard->ZobristHashCode ^= BoardInitializer::ZCodeFlag[4];
        // Black Queen Castle
        beginBoard->blackBigCastle = true;
        beginBoard->ZobristHashCode ^= BoardInitializer::ZCodeFlag[3];
        // Un passent Place
        beginBoard->unpassentPlace = 0;
        // 50 Move Rule
        beginBoard->fiftyMoveRule = 0;
        for (int counter = 0; counter < 16; counter++)
        {
            beginBoard->whitePieces += Option::PowerTwo[counter];
        }
        for (int counter = 48; counter < 64; counter++)
        {
            beginBoard->blackPieces += Option::PowerTwo[counter];
        }
        for (int counter = 8; counter < 16; counter++)
        {
            beginBoard->whitePawns += Option::PowerTwo[counter];
        }
        for (int counter = 48; counter < 56; counter++)
        {
            beginBoard->blackPawns += Option::PowerTwo[counter];
        }
        beginBoard->ZobristHashCode = 0 ^ ZCode[4][0] ^ ZCode[2][1] ^ ZCode[3][2] ^ ZCode[5][3] ^ ZCode[6][4] ^ ZCode[3][5] ^ ZCode[2][6] ^ ZCode[4][7] ^ ZCode[1][8] ^ ZCode[1][9] ^ ZCode[1][10] ^ ZCode[1][11] ^ ZCode[1][12] ^ ZCode[1][13] ^ ZCode[1][14] ^ ZCode[1][15] ^ ZCode[9][48] ^ ZCode[9][49] ^ ZCode[9][50] ^ ZCode[9][51] ^ ZCode[9][52] ^ ZCode[9][53] ^ ZCode[9][54] ^ ZCode[9][55] ^ ZCode[12][56] ^ ZCode[10][57] ^ ZCode[11][58] ^ ZCode[13][59] ^ ZCode[14][60] ^ ZCode[11][61] ^ ZCode[10][62] ^ ZCode[12][63];
        initialized = true;
    }
}

void BoardInitializer::Cleanup()
{
    if (beginBoard)
    {
        delete beginBoard;
        beginBoard = nullptr;
    }
    initialized = false;
}
