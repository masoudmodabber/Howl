#include "RepetitionHistory.h"

std::vector<long long> RepetitionHistory::history;

void RepetitionHistory::Reset()
{
    history.clear();
}

void RepetitionHistory::ResetWithRoot(long long rootHash)
{
    history.clear();
    history.push_back(rootHash);
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
    // Check all previous positions in history before the current position (history.back())
    for (std::size_t i = history.size() - 1; i > 0; --i)
    {
        if (history[i - 1] == hash)
        {
            return true;
        }
    }
    return false;
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
