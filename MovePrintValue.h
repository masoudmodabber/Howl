#ifndef MOVEPRINTVALUE_H
#define MOVEPRINTVALUE_H

#include <string>
#include <cstdint>

enum class SearchBound
{
    Exact,
    Lower,
    Upper
};

// Provenance describes how the score was obtained. Proof is directional: an
// omitted sibling invalidates an upper bound, but need not invalidate a lower
// bound established by a different, fully searched move. Quiescence is the
// engine's defined frontier, not proof of a game-theoretic score or exact mate.
enum class SearchProvenance : uint16_t
{
    FullSearch = 0,
    Quiescence = 1,
    ForwardPruning = 2,
    ReducedSearch = 4,
    NullMove = 8,
    ProbCut = 16,
    Repetition = 32,
    Aborted = 64,
    InvalidMove = 128,
    NullVerification = 256,
    ExtendedFrontier = 512
};

enum SearchProof : uint8_t
{
    NoProof = 0,
    LowerProof = 1,
    UpperProof = 2,
    ExactProof = LowerProof | UpperProof
};

inline uint8_t InvertProof(uint8_t proof)
{
    return static_cast<uint8_t>(((proof & LowerProof) ? UpperProof : NoProof) |
                                ((proof & UpperProof) ? LowerProof : NoProof));
}

class MovePrintValue {
public:

    int value = 0;
    std::string printString;
    SearchBound bound = SearchBound::Exact;
    uint8_t proof = ExactProof;
    uint16_t provenance = 0;
    // Retained for exact-mate reporting. Score reuse uses proof, not this bit.
    bool selective = false;

    bool HasLowerProof() const { return (proof & LowerProof) != 0; }
    bool HasUpperProof() const { return (proof & UpperProof) != 0; }
    bool ProvesLower(int threshold) const { return HasLowerProof() && value >= threshold; }
    bool ProvesUpper(int threshold) const { return HasUpperProof() && value <= threshold; }
    void AddProvenance(SearchProvenance source)
    {
        provenance |= static_cast<uint16_t>(source);
    }
    void SetProof(bool lower, bool upper)
    {
        proof = static_cast<uint8_t>((lower ? LowerProof : NoProof) |
                                     (upper ? UpperProof : NoProof));
    }
    void MarkSpeculative(SearchProvenance source)
    {
        AddProvenance(source);
        proof = NoProof;
        selective = true;
    }
    int depth = 0;
    int64_t elapsed_ms = 0;
    int64_t nodes = 0;
    int64_t nps = 0;
    std::string scoreText;
    std::string pv;
};

#endif
