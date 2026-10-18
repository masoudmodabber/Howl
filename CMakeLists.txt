cmake_minimum_required(VERSION 3.10)
project(Howl C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
endif()

add_library(fathom STATIC
    third_party/fathom/src/tbprobe.c
)
target_include_directories(fathom PUBLIC third_party/fathom/src)

set(HOWL_SOURCES
    AttackPlaces.cpp
    Board.cpp
    BoardInitializer.cpp
    BoardLogic.cpp
    BoardMaker.cpp
    ChessCache.cpp
    ChessStringManipulation.cpp
    ExchangeChessCache.cpp
    GameLogic.cpp
    HashMemoryBudget.cpp
    KingSetup.cpp
    LastFourMoves.cpp
    MissingInfoAboutPrevStateFromMove.cpp
    MoveLogic.cpp
    NNUEHistory.cpp
    Option.cpp
    PVSSearch.cpp
    PassedPawnSetup.cpp
    PieceMoves.cpp
    PositionCore.cpp
    QSearcher.cpp
    RepetitionHistory.cpp
    Search.cpp
    SearchParameters.cpp
    StructuredNNUEEvaluator.cpp
    StructuredNNUEWeights.cpp
    Tablebase.cpp
    TranspositionTable.cpp
    UCI.cpp
    main.cpp
)

add_executable(howl ${HOWL_SOURCES})
target_link_libraries(howl PRIVATE fathom)
