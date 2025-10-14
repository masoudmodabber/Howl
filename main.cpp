//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
//#include <malloc.h>

#include <iostream>
#include "UCI.h"
#include "AttackPlaces.h"
#include "BoardInitializer.h"
#include "PieceMoves.h"
#include "MoveLogic.h"
#include "KingSetup.h"
#include "Option.h"
#include "PassedPawnSetup.h"

int main(int argc, char* argv[])
{
#ifdef _WIN32
    // Enable detailed memory leak detection with file/line info
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(82017); 
    //_CrtSetBreakAlloc(17127);

    // This should enable file/line tracking
#endif

    Option::Initialize();
    AttackPlaces::Initialize();
    BoardInitializer::Initialize();
    PieceMoves::Initialize();
    MoveLogic::Initialize();
    KingSetup::Initialize();
    PassedPawnSetup::Initialize();

    const char* processorsStr = std::getenv("NUMBER_OF_PROCESSORS");
    if (processorsStr != nullptr)
    {
        int processors = std::stoi(processorsStr);
    }
    std::cout << "id name Howl 5\n";
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.setf(std::ios::unitbuf);      // auto-flush after each <<

    // belt-and-braces (some libcs ignore unitbuf when piped):
    setvbuf(stdout, nullptr, _IONBF, 0);    // fully unbuffer stdout
    UCI uci;
    uci.MainAsync();

    return 0;
}
