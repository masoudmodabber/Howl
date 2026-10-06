#include "RepetitionHistory.h"
#include "Board.h"
#include "BoardInitializer.h"
#include "Option.h"
#include <array>
#include <cassert>
#include <cstdlib>
#include <mutex>
#include <stdexcept>

std::vector<long long> RepetitionHistory::history;
std::size_t RepetitionHistory::searchRootIndex = 0;
static bool strictThreefold = false;

namespace
{
    constexpr std::size_t CuckooSize = 8192;
    struct Transition
    {
        std::uint64_t key = 0;
        std::uint8_t from = 0;
        std::uint8_t to = 0;
        std::uint8_t piece = 0;
    };
    std::array<Transition, CuckooSize> cuckoo{};
    std::once_flag cuckooOnce;
    std::size_t transitionCount = 0;

    int H1(std::uint64_t key) { return static_cast<int>(key & (CuckooSize - 1)); }
    int H2(std::uint64_t key) { return static_cast<int>((key >> 16) & (CuckooSize - 1)); }

    bool GeometricMove(int piece, int from, int to)
    {
        const int fromFile = from & 7, fromRank = from >> 3;
        const int toFile = to & 7, toRank = to >> 3;
        const int df = std::abs(toFile - fromFile), dr = std::abs(toRank - fromRank);
        switch (piece % 8)
        {
        case 2: return (df == 1 && dr == 2) || (df == 2 && dr == 1);
        case 3: return df == dr && df != 0;
        case 4: return (df == 0) != (dr == 0);
        case 5: return (df == dr && df != 0) || ((df == 0) != (dr == 0));
        case 6: return df <= 1 && dr <= 1 && (df != 0 || dr != 0);
        default: return false;
        }
    }

    bool ClearPath(const Board& board, int piece, int from, int to)
    {
        if (piece % 8 == 2 || piece % 8 == 6)
            return true;
        const int stepFile = (to & 7) == (from & 7) ? 0 : ((to & 7) > (from & 7) ? 1 : -1);
        const int stepRank = (to >> 3) == (from >> 3) ? 0 : ((to >> 3) > (from >> 3) ? 1 : -1);
        int square = from + stepFile + 8 * stepRank;
        while (square != to)
        {
            if (board.positionCore.pieceAt[square] != 0)
                return false;
            square += stepFile + 8 * stepRank;
        }
        return true;
    }

    void InsertTransition(Transition transition)
    {
        int slot = H1(transition.key);
        for (int attempt = 0; attempt < 64; ++attempt)
        {
            if (cuckoo[slot].key == 0)
            {
                cuckoo[slot] = transition;
                ++transitionCount;
                return;
            }
            std::swap(cuckoo[slot], transition);
            const int first = H1(transition.key);
            slot = slot == first ? H2(transition.key) : first;
        }
        throw std::runtime_error("repetition cuckoo table insertion failed");
    }

    void InitializeCuckoo()
    {
        for (int colour = 0; colour < 2; ++colour)
            for (int type = 2; type <= 6; ++type)
                for (int from = 0; from < 64; ++from)
                    for (int to = from + 1; to < 64; ++to)
                        if (GeometricMove(type, from, to))
                        {
                            const int piece = type + (colour ? 8 : 0);
                            const auto key = static_cast<std::uint64_t>(
                                BoardInitializer::ZCode[piece][from] ^
                                BoardInitializer::ZCode[piece][to] ^
                                BoardInitializer::ZCodeFlag[7]);
                            InsertTransition({key, static_cast<std::uint8_t>(from),
                                              static_cast<std::uint8_t>(to),
                                              static_cast<std::uint8_t>(piece)});
                        }
        assert(transitionCount > 0);
    }

    bool MatchesCurrentPosition(const Board& board, const Transition& transition)
    {
        const int side = board.sideToMove ? 1 : 0;
        const int moverColour = side;
        const int piece = transition.piece;
        const auto occupied = [&](int square) { return board.positionCore.pieceAt[square] != 0; };
        const auto valid = [&](int from, int to)
        {
            if (board.positionCore.pieceAt[from] != piece || occupied(to))
                return false;
            if ((piece > 8) != (moverColour != 0))
                return false;
            return GeometricMove(piece, from, to) && ClearPath(board, piece, from, to);
        };
        return valid(transition.from, transition.to) || valid(transition.to, transition.from);
    }
}

void RepetitionHistory::Reset()
{
    history.clear();
    searchRootIndex = 0;
}

void RepetitionHistory::ResetWithRoot(long long rootHash)
{
    history.clear();
    history.push_back(rootHash);
    searchRootIndex = 0;
}

void RepetitionHistory::Push(long long hash)
{
    history.push_back(hash);
}

void RepetitionHistory::Pop()
{
    if (!history.empty())
    {
        history.pop_back();
    }
}

bool RepetitionHistory::IsRepetition(long long hash)
{
    if (history.size() <= 1)
    {
        return false;
    }
    std::size_t matches = 0;
    bool searchCycle = false;
    for (std::size_t i = history.size() - 1; i > 0; --i)
    {
        if (history[i - 1] == hash)
        {
            ++matches;
            if (i - 1 > searchRootIndex)
                searchCycle = true;
        }
    }
    if (strictThreefold)
        return matches >= 2;
    return searchCycle || matches >= 2;
}

bool RepetitionHistory::IsSearchTreeRepetition(long long hash)
{
    if (history.size() <= 1)
        return false;
    for (std::size_t i = history.size() - 1; i > 0; --i)
        if (history[i - 1] == hash && i - 1 > searchRootIndex)
            return true;
    return false;
}

bool RepetitionHistory::HasGameCycle(const ::Board& board)
{
    InitializeGameCycleTable();
    if (history.size() < 4)
        return false;

    const std::uint64_t current = static_cast<std::uint64_t>(board.ZobristHashCode);
    std::size_t matches = 0;
    const std::size_t currentIndex = history.size() - 1;
    for (std::size_t index = currentIndex - 3; ; )
    {
        const auto transitionKey = current ^ static_cast<std::uint64_t>(history[index]);
        const int slots[2] = {H1(transitionKey), H2(transitionKey)};
        bool transitionFound = false;
        for (int slot : slots)
        {
            const Transition& transition = cuckoo[slot];
            if (transition.key == transitionKey)
            {
                if (MatchesCurrentPosition(board, transition))
                {
                    transitionFound = true;
                    break;
                }
            }
        }
        if (transitionFound)
        {
            ++matches;
            if (index > searchRootIndex)
                return true;
        }
        if (index < 2)
            break;
        index -= 2;
    }
    return matches >= 2;
}

void RepetitionHistory::InitializeGameCycleTable()
{
    std::call_once(cuckooOnce, InitializeCuckoo);
}

void RepetitionHistory::BeginSearchRoot()
{
    if (!history.empty())
        searchRootIndex = history.size() - 1;
}

void RepetitionHistory::SetStrictThreefold(bool enabled)
{
    strictThreefold = enabled;
}

std::size_t RepetitionHistory::Size()
{
    return history.size();
}

long long RepetitionHistory::Get(std::size_t index)
{
    return history[index];
}

void RepetitionHistory::SetHistory(const std::vector<long long>& hashes)
{
    history = hashes;
}

const std::vector<long long>& RepetitionHistory::GetHistory()
{
    return history;
}
