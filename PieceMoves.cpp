#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "PieceMoves.h"
#include "Option.h"

Move *PieceMoves::WhitePawnMoves[64][18] = {nullptr};
Move *PieceMoves::BlackPawnMoves[64][18] = {nullptr};
Move *PieceMoves::WhiteKingMoves[64][18] = {nullptr};
Move *PieceMoves::BlackKingMoves[64][18] = {nullptr};
Move *PieceMoves::KnightMoves[64][16] = {nullptr};
std::vector<Move *> PieceMoves::BishopMoves[64][8] = {};
std::vector<Move *> PieceMoves::RookMoves[64][8] = {};
std::vector<Move *> PieceMoves::QueenMoves[64][16] = {};
long long PieceMoves::pawnTwoMove[64] = {};
bool PieceMoves::initialized = false;

void PieceMoves::Initialize()
{
    if (!initialized)
    {
        for (int counter = 8; counter < 16; counter++)
        {
            pawnTwoMove[counter] = Option::PowerTwo[counter + 8] + Option::PowerTwo[counter + 16];
        }
        for (int counter = 48; counter < 56; counter++)
        {
            pawnTwoMove[counter] = Option::PowerTwo[counter - 8] + Option::PowerTwo[counter - 16];
        }

        // WhitePawnMoves
        // 0
        for (int counter = 8; counter < 16; counter++)
        {
            Move *newMove = new Move();
            newMove->beginPlace = counter;
            newMove->endPlace = counter + 16;
            newMove->unpassentPlace = counter + 8;
            WhitePawnMoves[counter][0] = newMove;
        }
        // 1
        for (int counter = 8; counter < 48; counter++)
        {
            Move *newMove = new Move();
            newMove->beginPlace = counter;
            newMove->endPlace = counter + 8;
            WhitePawnMoves[counter][1] = newMove;
        }
        // 2, 3, 4, 5
        for (int counter = 48; counter < 56; counter++)
        {
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter + 8;
                newMove->promotionPiece = 5;
                WhitePawnMoves[counter][2] = newMove;
            }
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter + 8;
                newMove->promotionPiece = 4;
                WhitePawnMoves[counter][3] = newMove;
            }
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter + 8;
                newMove->promotionPiece = 3;
                WhitePawnMoves[counter][4] = newMove;
            }
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter + 8;
                newMove->promotionPiece = 2;
                WhitePawnMoves[counter][5] = newMove;
            }
        }
        // 6, 7
        for (int counter = 32; counter < 40; counter++)
        {
            if (counter % 8 > 0)
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter + 7;
                newMove->PublicFlag = (char)64;
                WhitePawnMoves[counter][6] = newMove;
            }
            if (counter % 8 < 7)
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter + 9;
                newMove->PublicFlag = (char)64;
                WhitePawnMoves[counter][7] = newMove;
            }
        }

        // 8
        for (int counter = 8; counter < 48; counter++)
        {
            if (counter % 8 > 0)
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter + 7;
                newMove->PublicFlag = (char)128;
                WhitePawnMoves[counter][8] = newMove;
            }
        }
        // 9, 10, 11, 12
        for (int counter = 48; counter < 56; counter++)
        {
            if (counter % 8 > 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 7;
                    newMove->promotionPiece = 5;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag += (char)16;
                    }
                    WhitePawnMoves[counter][9] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 7;
                    newMove->promotionPiece = 4;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag += (char)16;
                    }
                    WhitePawnMoves[counter][10] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 7;
                    newMove->promotionPiece = 3;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag += (char)16;
                    }
                    WhitePawnMoves[counter][11] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 7;
                    newMove->promotionPiece = 2;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag += (char)16;
                    }
                    WhitePawnMoves[counter][12] = newMove;
                }
            }
        }
        // 13
        for (int counter = 8; counter < 48; counter++)
        {
            if (counter % 8 < 7)
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter + 9;
                newMove->PublicFlag = (char)128;
                WhitePawnMoves[counter][13] = newMove;
            }
        }

        // 14, 15, 16, 17
        for (int counter = 48; counter < 56; counter++)
        {
            if (counter % 8 < 7)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 9;
                    newMove->promotionPiece = 5;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag += (char)32;
                    }
                    WhitePawnMoves[counter][14] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 9;
                    newMove->promotionPiece = 4;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag += (char)32;
                    }
                    WhitePawnMoves[counter][15] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 9;
                    newMove->promotionPiece = 3;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag += (char)32;
                    }
                    WhitePawnMoves[counter][16] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 9;
                    newMove->promotionPiece = 2;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag += (char)32;
                    }
                    WhitePawnMoves[counter][17] = newMove;
                }
            }
        }
        // BlackPawnMoves
        // 0
        for (int counter = 48; counter < 56; counter++)
        {
            Move *newMove = new Move();
            newMove->beginPlace = counter;
            newMove->endPlace = counter - 16;
            newMove->unpassentPlace = counter - 8;
            BlackPawnMoves[counter][0] = newMove;
        }

        // 1
        for (int counter = 16; counter < 56; counter++)
        {
            Move *newMove = new Move();
            newMove->beginPlace = counter;
            newMove->endPlace = counter - 8;
            BlackPawnMoves[counter][1] = newMove;
        }

        // 2, 3, 4, 5
        for (int counter = 8; counter < 16; counter++)
        {
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter - 8;
                newMove->promotionPiece = 13;
                BlackPawnMoves[counter][2] = newMove;
            }
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter - 8;
                newMove->promotionPiece = 12;
                BlackPawnMoves[counter][3] = newMove;
            }
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter - 8;
                newMove->promotionPiece = 11;
                BlackPawnMoves[counter][4] = newMove;
            }
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter - 8;
                newMove->promotionPiece = 10;
                BlackPawnMoves[counter][5] = newMove;
            }
        }
        // 6, 7
        for (int counter = 24; counter < 32; counter++)
        {
            if (counter % 8 < 7)
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter - 7;
                newMove->PublicFlag = (char)64;
                BlackPawnMoves[counter][6] = newMove;
            }
            if (counter % 8 > 0)
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter - 9;
                newMove->PublicFlag = (char)64;
                BlackPawnMoves[counter][7] = newMove;
            }
        }

        // 8
        for (int counter = 16; counter < 56; counter++)
        {
            if (counter % 8 < 7)
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter - 7;
                newMove->PublicFlag = (char)128;
                BlackPawnMoves[counter][8] = newMove;
            }
        }

        // 9, 10, 11, 12
        for (int counter = 8; counter < 16; counter++)
        {
            if (counter % 8 < 7)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 7;
                    newMove->promotionPiece = 13;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag += (char)128;
                    }
                    BlackPawnMoves[counter][9] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 7;
                    newMove->promotionPiece = 12;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag += (char)128;
                    }
                    BlackPawnMoves[counter][10] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 7;
                    newMove->promotionPiece = 11;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag += (char)128;
                    }
                    BlackPawnMoves[counter][11] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 7;
                    newMove->promotionPiece = 10;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag += (char)128;
                    }
                    BlackPawnMoves[counter][12] = newMove;
                }
            }
        }
        // 13
        for (int counter = 16; counter < 56; counter++)
        {
            if (counter % 8 > 0)
            {
                Move *newMove = new Move();
                newMove->beginPlace = counter;
                newMove->endPlace = counter - 9;
                newMove->PublicFlag = (char)128;
                BlackPawnMoves[counter][13] = newMove;
            }
        }

        // 14, 15, 16, 17
        for (int counter = 8; counter < 16; counter++)
        {
            if (counter % 8 > 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 9;
                    newMove->promotionPiece = 13;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag += (char)64;
                    }
                    BlackPawnMoves[counter][14] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 9;
                    newMove->promotionPiece = 12;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag += (char)64;
                    }
                    BlackPawnMoves[counter][15] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 9;
                    newMove->promotionPiece = 11;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag += (char)64;
                    }
                    BlackPawnMoves[counter][16] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 9;
                    newMove->promotionPiece = 10;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag += (char)64;
                    }
                    BlackPawnMoves[counter][17] = newMove;
                }
            }
        }

        // Knight Moves
        for (int counter = 0; counter < 64; counter++)
        {
            if (counter + 17 < 64)
            {
                if (counter % 8 < 7)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 17;
                        KnightMoves[counter][0] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 17;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 63)
                        {
                            newMove->CastleFlag = (char)32;
                        }
                        KnightMoves[counter][1] = newMove;
                    }
                }
            }
            if (counter + 10 < 64)
            {
                if (counter % 8 < 6)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 10;
                        KnightMoves[counter][2] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 10;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 63)
                        {
                            newMove->CastleFlag = (char)32;
                        }
                        KnightMoves[counter][3] = newMove;
                    }
                }
            }
            if (counter + 15 < 64)
            {
                if (counter % 8 > 0)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 15;
                        KnightMoves[counter][4] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 15;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 56)
                        {
                            newMove->CastleFlag = (char)16;
                        }
                        KnightMoves[counter][5] = newMove;
                    }
                }
            }
            if (counter + 6 < 64)
            {
                if (counter % 8 > 1)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 6;
                        KnightMoves[counter][6] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 6;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 56)
                        {
                            newMove->CastleFlag = (char)16;
                        }
                        KnightMoves[counter][7] = newMove;
                    }
                }
            }
            if (counter - 10 >= 0)
            {
                if (counter % 8 > 1)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 10;
                        KnightMoves[counter][8] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 10;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 0)
                        {
                            newMove->CastleFlag = (char)64;
                        }
                        KnightMoves[counter][9] = newMove;
                    }
                }
            }
            if (counter - 17 >= 0)
            {
                if (counter % 8 > 0)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 17;
                        KnightMoves[counter][10] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 17;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 0)
                        {
                            newMove->CastleFlag = (char)64;
                        }
                        KnightMoves[counter][11] = newMove;
                    }
                }
            }
            if (counter - 15 >= 0)
            {
                if (counter % 8 < 7)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 15;
                        KnightMoves[counter][12] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 15;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 7)
                        {
                            newMove->CastleFlag = (char)128;
                        }
                        KnightMoves[counter][13] = newMove;
                    }
                }
            }
            if (counter - 6 >= 0)
            {
                if (counter % 8 < 6)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 6;
                        KnightMoves[counter][14] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 6;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 7)
                        {
                            newMove->CastleFlag = (char)128;
                        }
                        KnightMoves[counter][15] = newMove;
                    }
                }
            }
        }

        // Bishop Moves
        for (int counter = 0; counter < 64; counter++)
        {
            BishopMoves[counter][0] = {};
            BishopMoves[counter][1] = {};
            BishopMoves[counter][2] = {};
            BishopMoves[counter][3] = {};
            BishopMoves[counter][4] = {};
            BishopMoves[counter][5] = {};
            BishopMoves[counter][6] = {};
            BishopMoves[counter][7] = {};
            int increment = 7;
            int endPos = counter + increment;
            while (endPos < 64 && endPos % 8 != 7)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    BishopMoves[counter][0].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag = (char)16;
                    }
                    BishopMoves[counter][1].push_back(newMove);
                }
                endPos += increment;
            }

            increment = 9;
            endPos = counter + increment;

            while (endPos < 64 && endPos % 8 != 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    BishopMoves[counter][2].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag = (char)32;
                    }
                    BishopMoves[counter][3].push_back(newMove);
                }
                endPos += increment;
            }
            increment = -9;
            endPos = counter + increment;
            while (endPos >= 0 && endPos % 8 != 7)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    BishopMoves[counter][4].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag = (char)64;
                    }
                    BishopMoves[counter][5].push_back(newMove);
                }
                endPos += increment;
            }

            increment = -7;
            endPos = counter + increment;
            while (endPos >= 0 && endPos % 8 != 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    BishopMoves[counter][6].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag = (char)128;
                    }
                    BishopMoves[counter][7].push_back(newMove);
                }
                endPos += increment;
            }
        }

        // Rook Moves
        for (int counter = 0; counter < 64; counter++)
        {
            RookMoves[counter][0] = {};
            RookMoves[counter][1] = {};
            RookMoves[counter][2] = {};
            RookMoves[counter][3] = {};
            RookMoves[counter][4] = {};
            RookMoves[counter][5] = {};
            RookMoves[counter][6] = {};
            RookMoves[counter][7] = {};
            int increment = 8;
            int endPos = counter + increment;
            while (endPos < 64)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    if (counter == 0)
                    {
                        newMove->CastleFlag = (char)64;
                    }
                    if (counter == 7)
                    {
                        newMove->CastleFlag = (char)128;
                    }
                    RookMoves[counter][0].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag = (char)16;
                    }
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag = (char)32;
                    }
                    if (counter == 0)
                    {
                        newMove->CastleFlag += (char)64;
                    }
                    if (counter == 7)
                    {
                        newMove->CastleFlag += (char)128;
                    }
                    RookMoves[counter][1].push_back(newMove);
                }
                endPos += increment;
            }
            increment = 1;
            endPos = counter + increment;
            while (endPos % 8 != 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    if (counter == 0)
                    {
                        newMove->CastleFlag = (char)64;
                    }
                    if (counter == 56)
                    {
                        newMove->CastleFlag = (char)16;
                    }
                    RookMoves[counter][2].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag = (char)32;
                    }
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag = (char)128;
                    }
                    if (counter == 0)
                    {
                        newMove->CastleFlag += (char)64;
                    }
                    if (counter == 56)
                    {
                        newMove->CastleFlag += (char)16;
                    }
                    RookMoves[counter][3].push_back(newMove);
                }
                endPos += increment;
            }
            increment = -1;
            endPos = counter + increment;
            while (endPos % 8 != 7 && endPos >= 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    if (counter == 7)
                    {
                        newMove->CastleFlag = (char)128;
                    }
                    if (counter == 63)
                    {
                        newMove->CastleFlag = (char)32;
                    }
                    RookMoves[counter][4].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag = (char)16;
                    }
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag = (char)64;
                    }
                    if (counter == 63)
                    {
                        newMove->CastleFlag += (char)32;
                    }
                    if (counter == 7)
                    {
                        newMove->CastleFlag += (char)128;
                    }
                    RookMoves[counter][5].push_back(newMove);
                }
                endPos += increment;
            }
            increment = -8;
            endPos = counter + increment;
            while (endPos >= 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    if (counter == 56)
                    {
                        newMove->CastleFlag = (char)16;
                    }
                    if (counter == 63)
                    {
                        newMove->CastleFlag = (char)32;
                    }
                    RookMoves[counter][6].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag = (char)128;
                    }
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag = (char)64;
                    }
                    if (counter == 63)
                    {
                        newMove->CastleFlag += (char)32;
                    }
                    if (counter == 56)
                    {
                        newMove->CastleFlag += (char)16;
                    }
                    RookMoves[counter][7].push_back(newMove);
                }
                endPos += increment;
            }
        }
        // Queen Moves
        for (int counter = 0; counter < 64; counter++)
        {
            QueenMoves[counter][0] = {};
            QueenMoves[counter][1] = {};
            QueenMoves[counter][2] = {};
            QueenMoves[counter][3] = {};
            QueenMoves[counter][4] = {};
            QueenMoves[counter][5] = {};
            QueenMoves[counter][6] = {};
            QueenMoves[counter][7] = {};
            QueenMoves[counter][8] = {};
            QueenMoves[counter][9] = {};
            QueenMoves[counter][10] = {};
            QueenMoves[counter][11] = {};
            QueenMoves[counter][12] = {};
            QueenMoves[counter][13] = {};
            QueenMoves[counter][14] = {};
            QueenMoves[counter][15] = {};
            int increment = 7;
            int endPos = counter + increment;
            while (endPos < 64 && endPos % 8 != 7)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    QueenMoves[counter][0].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag = (char)16;
                    }
                    QueenMoves[counter][1].push_back(newMove);
                }
                endPos += increment;
            }
            increment = 9;
            endPos = counter + increment;
            while (endPos < 64 && endPos % 8 != 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    QueenMoves[counter][2].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag = (char)32;
                    }
                    QueenMoves[counter][3].push_back(newMove);
                }
                endPos += increment;
            }
            increment = -9;
            endPos = counter + increment;
            while (endPos >= 0 && endPos % 8 != 7)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    QueenMoves[counter][4].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag = (char)64;
                    }
                    QueenMoves[counter][5].push_back(newMove);
                }
                endPos += increment;
            }

            increment = -7;
            endPos = counter + increment;
            while (endPos > 0 && endPos % 8 != 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    QueenMoves[counter][6].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag = (char)128;
                    }
                    QueenMoves[counter][7].push_back(newMove);
                }
                endPos += increment;
            }
            increment = 8;
            endPos = counter + increment;
            while (endPos < 64)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    QueenMoves[counter][8].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag = (char)16;
                    }
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag = (char)32;
                    }
                    QueenMoves[counter][9].push_back(newMove);
                }
                endPos += increment;
            }

            increment = 1;
            endPos = counter + increment;
            while (endPos % 8 != 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    QueenMoves[counter][10].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    if (QueenMoves[counter][11].empty())
                    {
                        QueenMoves[counter][11] = std::vector<Move *>();
                    }
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag = (char)32;
                    }
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag = (char)128;
                    }
                    QueenMoves[counter][11].push_back(newMove);
                }
                endPos += increment;
            }
            increment = -1;
            endPos = counter + increment;
            while (endPos % 8 != 7 && endPos >= 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    QueenMoves[counter][12].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag = (char)16;
                    }
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag = (char)64;
                    }
                    QueenMoves[counter][13].push_back(newMove);
                }
                endPos += increment;
            }

            increment = -8;
            endPos = counter + increment;
            while (endPos >= 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    QueenMoves[counter][14].push_back(newMove);
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = endPos;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag = (char)128;
                    }
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag = (char)64;
                    }
                    QueenMoves[counter][15].push_back(newMove);
                }
                endPos += increment;
            }
        }

        // WhiteKingMoves
        for (int counter = 0; counter < 64; counter++)
        {
            if (counter + 7 < 64)
            {
                if (counter % 8 > 0)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 7;
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][0] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 7;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 56)
                        {
                            newMove->CastleFlag = (char)16;
                        }
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][1] = newMove;
                    }
                }
            }

            if (counter + 8 < 64)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 8;
                    newMove->CastleFlag += (char)128;
                    newMove->CastleFlag += (char)64;
                    WhiteKingMoves[counter][2] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 8;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag = (char)32;
                    }
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag = (char)16;
                    }
                    newMove->CastleFlag += (char)128;
                    newMove->CastleFlag += (char)64;
                    WhiteKingMoves[counter][3] = newMove;
                }
            }
            if (counter + 9 < 64)
            {
                if (counter % 8 < 7)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 9;
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][4] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 9;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 63)
                        {
                            newMove->CastleFlag = (char)32;
                        }
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][5] = newMove;
                    }
                }
            }

            if (counter + 1 < 64)
            {
                if (counter % 8 < 7)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 1;
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][6] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 1;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 63)
                        {
                            newMove->CastleFlag = (char)32;
                        }
                        if (newMove->endPlace == 7)
                        {
                            newMove->CastleFlag = (char)128;
                        }
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][7] = newMove;
                    }
                }
            }
            if (counter - 7 >= 0)
            {
                if (counter % 8 < 7)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 7;
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][8] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 7;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 7)
                        {
                            newMove->CastleFlag = (char)128;
                        }
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][9] = newMove;
                    }
                }
            }

            if (counter - 8 >= 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 8;
                    newMove->CastleFlag += (char)128;
                    newMove->CastleFlag += (char)64;
                    WhiteKingMoves[counter][10] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 8;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag = (char)64;
                    }
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag = (char)128;
                    }
                    newMove->CastleFlag += (char)128;
                    newMove->CastleFlag += (char)64;
                    WhiteKingMoves[counter][11] = newMove;
                }
            }
            if (counter - 9 >= 0)
            {
                if (counter % 8 > 0)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 9;
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][12] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 9;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 0)
                        {
                            newMove->CastleFlag = (char)64;
                        }
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][13] = newMove;
                    }
                }
            }

            if (counter - 1 >= 0)
            {
                if (counter % 8 > 0)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 1;
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][14] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 1;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 0)
                        {
                            newMove->CastleFlag = (char)64;
                        }
                        if (newMove->endPlace == 56)
                        {
                            newMove->CastleFlag = (char)16;
                        }
                        newMove->CastleFlag += (char)128;
                        newMove->CastleFlag += (char)64;
                        WhiteKingMoves[counter][15] = newMove;
                    }
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = 4;
                    newMove->endPlace = 6;
                    newMove->CastleFlag = (char)(128 + 64 + 8);
                    WhiteKingMoves[4][16] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = 4;
                    newMove->endPlace = 2;
                    newMove->CastleFlag = (char)(128 + 64 + 4);
                    WhiteKingMoves[4][17] = newMove;
                }
            }
        }
        // BlackKingMoves
        for (int counter = 0; counter < 64; counter++)
        {
            if (counter + 7 < 64)
            {
                if (counter % 8 > 0)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 7;
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][0] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 7;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 56)
                        {
                            newMove->CastleFlag = (char)16;
                        }
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][1] = newMove;
                    }
                }
            }

            if (counter + 8 < 64)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 8;
                    newMove->CastleFlag += (char)32;
                    newMove->CastleFlag += (char)16;
                    BlackKingMoves[counter][2] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter + 8;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 63)
                    {
                        newMove->CastleFlag = (char)32;
                    }
                    if (newMove->endPlace == 56)
                    {
                        newMove->CastleFlag = (char)16;
                    }
                    newMove->CastleFlag += (char)32;
                    newMove->CastleFlag += (char)16;
                    BlackKingMoves[counter][3] = newMove;
                }
            }
            if (counter + 9 < 64)
            {
                if (counter % 8 < 7)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 9;
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][4] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 9;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 63)
                        {
                            newMove->CastleFlag = (char)32;
                        }
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][5] = newMove;
                    }
                }
            }

            if (counter + 1 < 64)
            {
                if (counter % 8 < 7)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 1;
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][6] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter + 1;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 63)
                        {
                            newMove->CastleFlag = (char)32;
                        }
                        if (newMove->endPlace == 7)
                        {
                            newMove->CastleFlag = (char)128;
                        }
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][7] = newMove;
                    }
                }
            }
            if (counter - 7 >= 0)
            {
                if (counter % 8 < 7)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 7;
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][8] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 7;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 7)
                        {
                            newMove->CastleFlag = (char)128;
                        }
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][9] = newMove;
                    }
                }
            }

            if (counter - 8 >= 0)
            {
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 8;
                    newMove->CastleFlag += (char)32;
                    newMove->CastleFlag += (char)16;
                    BlackKingMoves[counter][10] = newMove;
                }
                {
                    Move *newMove = new Move();
                    newMove->beginPlace = counter;
                    newMove->endPlace = counter - 8;
                    newMove->PublicFlag = (char)128;
                    if (newMove->endPlace == 0)
                    {
                        newMove->CastleFlag = (char)64;
                    }
                    if (newMove->endPlace == 7)
                    {
                        newMove->CastleFlag = (char)128;
                    }
                    newMove->CastleFlag += (char)32;
                    newMove->CastleFlag += (char)16;
                    BlackKingMoves[counter][11] = newMove;
                }
            }
            if (counter - 9 >= 0)
            {
                if (counter % 8 > 0)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 9;
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][12] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 9;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 0)
                        {
                            newMove->CastleFlag = (char)64;
                        }
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][13] = newMove;
                    }
                }
            }

            if (counter - 1 >= 0)
            {
                if (counter % 8 > 0)
                {
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 1;
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][14] = newMove;
                    }
                    {
                        Move *newMove = new Move();
                        newMove->beginPlace = counter;
                        newMove->endPlace = counter - 1;
                        newMove->PublicFlag = (char)128;
                        if (newMove->endPlace == 0)
                        {
                            newMove->CastleFlag = (char)64;
                        }
                        if (newMove->endPlace == 56)
                        {
                            newMove->CastleFlag = (char)16;
                        }
                        newMove->CastleFlag += (char)32;
                        newMove->CastleFlag += (char)16;
                        BlackKingMoves[counter][15] = newMove;
                    }
                }
            }

            {
                Move *newMove = new Move();
                newMove->beginPlace = 60;
                newMove->endPlace = 62;
                newMove->CastleFlag = (char)(32 + 16 + 2);
                BlackKingMoves[60][16] = newMove;
            }

            {
                Move *newMove = new Move();
                newMove->beginPlace = 60;
                newMove->endPlace = 58;
                newMove->CastleFlag = (char)(32 + 16 + 1);
                BlackKingMoves[60][17] = newMove;
            }
        }
        initialized = true;
    }
}

void PieceMoves::Cleanup()
{
    if (initialized)
    {
        // Clean up WhitePawnMoves
        for (int i = 0; i < 64; i++)
        {
            for (int j = 0; j < 18; j++)
            {
                if (WhitePawnMoves[i][j] != nullptr)
                {
                    delete WhitePawnMoves[i][j];
                    WhitePawnMoves[i][j] = nullptr;
                }
            }
        }

        // Clean up BlackPawnMoves
        for (int i = 0; i < 64; i++)
        {
            for (int j = 0; j < 18; j++)
            {
                if (BlackPawnMoves[i][j] != nullptr)
                {
                    delete BlackPawnMoves[i][j];
                    BlackPawnMoves[i][j] = nullptr;
                }
            }
        }

        // Clean up WhiteKingMoves
        for (int i = 0; i < 64; i++)
        {
            for (int j = 0; j < 18; j++)
            {
                if (WhiteKingMoves[i][j] != nullptr)
                {
                    delete WhiteKingMoves[i][j];
                    WhiteKingMoves[i][j] = nullptr;
                }
            }
        }

        // Clean up BlackKingMoves
        for (int i = 0; i < 64; i++)
        {
            for (int j = 0; j < 18; j++)
            {
                if (BlackKingMoves[i][j] != nullptr)
                {
                    delete BlackKingMoves[i][j];
                    BlackKingMoves[i][j] = nullptr;
                }
            }
        }

        // Clean up KnightMoves
        for (int i = 0; i < 64; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                if (KnightMoves[i][j] != nullptr)
                {
                    delete KnightMoves[i][j];
                    KnightMoves[i][j] = nullptr;
                }
            }
        }

        // Clean up BishopMoves
        for (int i = 0; i < 64; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                for (Move *move : BishopMoves[i][j])
                {
                    delete move;
                }
                BishopMoves[i][j].clear();
            }
        }

        // Clean up RookMoves
        for (int i = 0; i < 64; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                for (Move *move : RookMoves[i][j])
                {
                    delete move;
                }
                RookMoves[i][j].clear();
            }
        }

        // Clean up QueenMoves
        for (int i = 0; i < 64; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                for (Move *move : QueenMoves[i][j])
                {
                    delete move;
                }
                QueenMoves[i][j].clear();
            }
        }

        // Reset pawnTwoMove array
        for (int i = 0; i < 64; i++)
        {
            pawnTwoMove[i] = 0;
        }

        initialized = false;
    }
}