#ifndef REPETITIONHISTORY_H
#define REPETITIONHISTORY_H

#include <vector>
#include <cstddef>

class RepetitionHistory
{
public:
    static void Reset();
    static void ResetWithRoot(long long rootHash);
    static void Push(long long hash);
    static void Pop();
    static bool IsRepetition(long long hash);
    static std::size_t Size();
    static long long Get(std::size_t index);
    static void SetHistory(const std::vector<long long>& hashes);
    static const std::vector<long long>& GetHistory();

private:
    static std::vector<long long> history;
};

#endif
