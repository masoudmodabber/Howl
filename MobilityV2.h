#ifndef HOWL_MOBILITY_V2_H
#define HOWL_MOBILITY_V2_H

#include <algorithm>

namespace MobilityV2
{

inline int RoundDivide(int numerator, int denominator)
{
    return numerator >= 0
        ? (numerator + denominator / 2) / denominator
        : -((-numerator + denominator / 2) / denominator);
}

inline void DecodeAnchors(const int* parameters, int count, int* anchors)
{
    anchors[0] = parameters[0];
    for (int i = 1; i < count; ++i)
        anchors[i] = anchors[i - 1] + std::max(0, parameters[i]);
}

inline void Interpolate(const int* buckets, const int* anchors, int anchorCount, int* output)
{
    for (int a = 0; a < anchorCount - 1; ++a)
    {
        const int firstBucket = buckets[a];
        const int span = buckets[a + 1] - firstBucket;
        const int firstValue = anchors[a];
        const int delta = anchors[a + 1] - firstValue;
        for (int offset = 0; offset < span; ++offset)
            output[firstBucket + offset] = firstValue + RoundDivide(delta * offset, span);
    }
    output[buckets[anchorCount - 1]] = anchors[anchorCount - 1];
}

inline void GenerateKnight(const int parameters[4], int output[9])
{
    static const int buckets[4] = {1, 3, 5, 7};
    int anchors[4];
    DecodeAnchors(parameters, 4, anchors);
    Interpolate(buckets, anchors, 4, output);
    output[0] = anchors[0] - RoundDivide(anchors[1] - anchors[0], 2);
    output[8] = anchors[3];
}

inline void GenerateBishop(const int parameters[5], int output[14])
{
    static const int buckets[5] = {0, 3, 5, 8, 11};
    int anchors[5];
    DecodeAnchors(parameters, 5, anchors);
    Interpolate(buckets, anchors, 5, output);

    const int terminalDelta = anchors[4] - anchors[3];
    output[12] = anchors[4] + RoundDivide(2 * terminalDelta, 9);
    output[13] = anchors[4] + RoundDivide(terminalDelta, 3);
}

inline void GenerateRook(const int parameters[5], int output[15])
{
    static const int buckets[5] = {0, 4, 8, 11, 14};
    int anchors[5];
    DecodeAnchors(parameters, 5, anchors);
    Interpolate(buckets, anchors, 5, output);
}

inline void GenerateQueenMiddleGame(const int parameters[5], int output[28])
{
    static const int buckets[5] = {0, 4, 8, 12, 15};
    int anchors[5];
    DecodeAnchors(parameters, 5, anchors);
    Interpolate(buckets, anchors, 5, output);
    std::fill(output + 16, output + 28, anchors[4]);
}

inline void GenerateQueenEndGame(const int parameters[5], int output[28])
{
    static const int buckets[5] = {0, 3, 6, 9, 12};
    int anchors[5];
    DecodeAnchors(parameters, 5, anchors);
    Interpolate(buckets, anchors, 5, output);
    std::fill(output + 13, output + 28, anchors[4]);
}

} // namespace MobilityV2

#endif // HOWL_MOBILITY_V2_H
