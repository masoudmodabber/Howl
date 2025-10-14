#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "Board.h"
#include <algorithm>
#include <stdexcept> 
#include <string>
#include <sstream> 
#include <stdexcept> 

Board* Board::MakeCopy()
{
    Board* copiedBoard = new Board();
    for (int i = 0; i < 64; ++i) {
        copiedBoard->mainBoard[i] = mainBoard[i];
    }
    copiedBoard->moveNumber = moveNumber;
    copiedBoard->fiftyMoveRule = fiftyMoveRule;
    for (int counter = 0; counter < 15; counter++)
    {
        copiedBoard->pieces[counter] = pieces[counter];
    }
    copiedBoard->whitePieces = whitePieces;
    copiedBoard->blackPieces = blackPieces;
    copiedBoard->whitePawns = whitePawns;
    copiedBoard->blackPawns = blackPawns;
    copiedBoard->sideToMove = sideToMove;
    copiedBoard->whiteSmallCastle = whiteSmallCastle;
    copiedBoard->whiteBigCastle = whiteBigCastle;
    copiedBoard->blackSmallCastle = blackSmallCastle;
    copiedBoard->blackBigCastle = blackBigCastle;
    copiedBoard->unpassentPlace = unpassentPlace;
    copiedBoard->ZobristHashCode = ZobristHashCode;
    return copiedBoard;
}

bool Board::AreBoardsEqual(Board& board1, Board& board2)
{
    if (board1.whitePieces != board2.whitePieces)
        throw std::runtime_error("White pieces are not equal");

    if (board1.whitePawns != board2.whitePawns)
        throw std::runtime_error("White pawns are not equal");

    if (board1.blackPieces != board2.blackPieces)
        throw std::runtime_error("Black pieces are not equal");

    if (board1.blackPawns != board2.blackPawns)
        throw std::runtime_error("Black pawns are not equal");

    if (board1.fiftyMoveRule != board2.fiftyMoveRule)
        throw std::runtime_error("Fifty move rules are not equal");

    if (board1.sideToMove != board2.sideToMove)
        throw std::runtime_error("Sides to move are not equal");

    if (board1.whiteSmallCastle != board2.whiteSmallCastle)
        throw std::runtime_error("White small castles are not equal");

    if (board1.whiteBigCastle != board2.whiteBigCastle)
        throw std::runtime_error("White big castles are not equal");

    if (board1.blackSmallCastle != board2.blackSmallCastle)
        throw std::runtime_error("Black small castles are not equal");

    if (board1.blackBigCastle != board2.blackBigCastle)
        throw std::runtime_error("Black big castles are not equal");

    if (board1.unpassentPlace != board2.unpassentPlace)
        throw std::runtime_error("Unpassent places are not equal");


    for (int i = 0; i < 64; ++i) {
        if (board1.mainBoard[i] != board2.mainBoard[i]) {
            std::ostringstream oss;
            oss << "Main boards are not equal at index " << i << ": "
                << "board1[" << i << "] = " << board1.mainBoard[i] << ", "
                << "board2[" << i << "] = " << board2.mainBoard[i];
            throw std::runtime_error(oss.str());
        }
    }

    for (int i = 0; i < 15; ++i) {
        if (board1.pieces[i].size() != board2.pieces[i].size() ||
            !std::is_permutation(board1.pieces[i].begin(), board1.pieces[i].end(), board2.pieces[i].begin())) {
            throw std::runtime_error("Pieces are not equal: " + std::to_string(i));
        }
    }

    if (board1.ZobristHashCode != board2.ZobristHashCode)
        throw std::runtime_error("Zobrist hash codes are not equal");

    return true;
}
