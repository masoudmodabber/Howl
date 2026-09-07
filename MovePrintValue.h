#ifndef MOVEPRINTVALUE_H
#define MOVEPRINTVALUE_H

#include <string>

enum class SearchBound
{
    Exact,
    Lower,
    Upper
};

class MovePrintValue {
public:

    int value = 0;
    std::string printString;
    SearchBound bound = SearchBound::Exact;
    bool selective = false;
    int depth = 0;
    int64_t elapsed_ms = 0;
    int64_t nodes = 0;
    int64_t nps = 0;
    std::string scoreText;
    std::string pv;
};

#endif
