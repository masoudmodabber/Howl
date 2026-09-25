#ifndef HOWL_PIECE_SQUARE_MODEL_H
#define HOWL_PIECE_SQUARE_MODEL_H

#include <algorithm>

namespace PieceSquareModel
{
constexpr int PawnParameterCount = 7;
constexpr int MinorParameterCount = 11;
constexpr int MajorParameterCount = 6;
constexpr int KingParameterCount = 7;
constexpr int TotalParameterCount = 2 * (PawnParameterCount + 2 * MinorParameterCount
                                        + 2 * MajorParameterCount + KingParameterCount);

// Candidate 2 (Conservative Quadratic & Dead Dimension Cleaned Model: 86 parameters)
constexpr int CandPawnParameterCount = 5;
constexpr int CandKnightParameterCount = 10;
constexpr int CandBishopParameterCount = 11;
constexpr int CandMajorParameterCount = 5;
constexpr int CandKingParameterCount = 7;
constexpr int CandTotalParameterCount = 2 * (CandPawnParameterCount + CandKnightParameterCount
                                            + CandBishopParameterCount + 2 * CandMajorParameterCount + CandKingParameterCount);

inline void GeneratePawn(const int* p, int* table)
{
    for (int sq = 0; sq < 64; ++sq)
    {
        const int rank = sq / 8;
        const int file = sq % 8;
        const bool edge = file == 0 || file == 7;
        table[sq] = p[0] + p[1] * rank + p[2] * rank * rank
                  + p[3] * edge + p[4] * (edge && rank >= 1 && rank <= 6)
                  + p[5] * (rank == 1) + p[6] * (rank == 7);
    }
}

inline void GenerateMinor(const int* p, int* table)
{
    for (int sq = 0; sq < 64; ++sq)
    {
        const int rank = sq / 8;
        const int fileCentrality = std::min(sq % 8, 7 - (sq % 8));
        int value = p[0] + p[1] * rank + p[2] * fileCentrality
                  + p[3] * fileCentrality * fileCentrality
                  + p[4] * (rank == 0) + p[5] * (rank == 6);
        if (rank >= 2 && rank <= 6)
            value += p[6 + rank - 2] * fileCentrality;
        table[sq] = value;
    }
}

inline void GenerateMajor(const int* p, int* table)
{
    for (int sq = 0; sq < 64; ++sq)
    {
        const int rank = sq / 8;
        const int fileCentrality = std::min(sq % 8, 7 - (sq % 8));
        table[sq] = p[0] + p[1] * rank + p[2] * rank * rank
                  + p[3] * fileCentrality + p[4] * (rank == 0)
                  + p[5] * (rank == 6);
    }
}

inline void GenerateKing(const int* p, int* table)
{
    for (int sq = 0; sq < 64; ++sq)
    {
        const int rank = sq / 8;
        const int fileCentrality = std::min(sq % 8, 7 - (sq % 8));
        const int rankCentrality = std::min(rank, 7 - rank);
        table[sq] = p[0]
                  + (fileCentrality == 1 ? p[1] : 0)
                  + (fileCentrality == 2 ? p[2] : 0)
                  + (fileCentrality == 3 ? p[3] : 0)
                  + (rankCentrality == 1 ? p[4] : 0)
                  + (rankCentrality == 2 ? p[5] : 0)
                  + (rankCentrality == 3 ? p[6] : 0);
    }
}

// Candidate 1 Generator Functions (eliminates rank^2, fc^2, and unreachable rank 7)
inline void GenerateCandPawn(const int* p, int* table)
{
    for (int sq = 0; sq < 64; ++sq)
    {
        const int rank = sq / 8;
        const int file = sq % 8;
        const bool edge = file == 0 || file == 7;
        table[sq] = p[0] + p[1] * rank
                  + p[2] * edge + p[3] * (edge && rank >= 1 && rank <= 6)
                  + p[4] * (rank == 1);
    }
}

inline void GenerateCandMinor(const int* p, int* table)
{
    for (int sq = 0; sq < 64; ++sq)
    {
        const int rank = sq / 8;
        const int fileCentrality = std::min(sq % 8, 7 - (sq % 8));
        int value = p[0] + p[1] * rank + p[2] * fileCentrality
                  + p[3] * (rank == 0) + p[4] * (rank == 6);
        if (rank >= 2 && rank <= 6)
            value += p[5 + rank - 2] * fileCentrality;
        table[sq] = value;
    }
}

inline void GenerateCandMajor(const int* p, int* table)
{
    for (int sq = 0; sq < 64; ++sq)
    {
        const int rank = sq / 8;
        const int fileCentrality = std::min(sq % 8, 7 - (sq % 8));
        table[sq] = p[0] + p[1] * rank
                  + p[2] * fileCentrality + p[3] * (rank == 0)
                  + p[4] * (rank == 6);
    }
}
}

#endif
