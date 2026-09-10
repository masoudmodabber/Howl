#ifndef HOWL_PASSED_PAWN_V2_H
#define HOWL_PASSED_PAWN_V2_H

#include <algorithm>
#include <iterator>

namespace PassedPawnV2
{

inline int RoundDivide(int numerator, int denominator)
{
    return numerator >= 0
        ? (numerator + denominator / 2) / denominator
        : -((-numerator + denominator / 2) / denominator);
}

inline void DecodeRanks(const int parameters[6], int ranks[6])
{
    ranks[0] = parameters[0];
    for (int i = 1; i < 6; ++i)
        ranks[i] = ranks[i - 1] + std::max(0, parameters[i]);
}

inline void Generate(const int middleGameParameters[6], int middleGameFileAmplitude,
                     const int endGameParameters[6], int middleGame[64], int endGame[64])
{
    std::fill(middleGame, middleGame + 64, 0);
    std::fill(endGame, endGame + 64, 0);

    int middleGameRanks[6];
    int endGameRanks[6];
    DecodeRanks(middleGameParameters, middleGameRanks);
    DecodeRanks(endGameParameters, endGameRanks);

    static const int fileWeights[8] = {-3, -1, 1, 3, 3, 1, -1, -3};
    for (int rank = 0; rank < 6; ++rank)
    {
        for (int file = 0; file < 8; ++file)
        {
            const int square = (rank + 1) * 8 + file;
            middleGame[square] = middleGameRanks[rank] +
                RoundDivide(middleGameFileAmplitude * fileWeights[file], 6);
            endGame[square] = endGameRanks[rank];
        }
    }
}

} // namespace PassedPawnV2

#endif // HOWL_PASSED_PAWN_V2_H
