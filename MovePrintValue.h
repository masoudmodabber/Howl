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
};

#endif
