#ifndef MATESCORE_H
#define MATESCORE_H

namespace MateScore
{
    constexpr int Mate = 159999;
    constexpr int Sentinel = 160000;
    constexpr int Threshold = 159800;

    inline bool IsMate(int score)
    {
        return score != Sentinel && score != -Sentinel &&
            (score > Threshold || score < -Threshold);
    }

    inline int MatedAtPly(int ply)
    {
        return -Mate + ply;
    }

    inline int MateAtPly(int ply)
    {
        return Mate - ply;
    }

    inline int ToTranspositionTable(int score, int ply)
    {
        if (score > Threshold)
            return score + ply;
        if (score < -Threshold)
            return score - ply;
        return score;
    }

    inline int FromTranspositionTable(int score, int ply)
    {
        if (score > Threshold)
            return score - ply;
        if (score < -Threshold)
            return score + ply;
        return score;
    }

    inline int MovesFromRootScore(int score)
    {
        const int plies = score > 0 ? Mate - score : score + Mate;
        return (plies + 1) / 2;
    }
}

#endif
