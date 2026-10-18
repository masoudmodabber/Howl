cmake_minimum_required(VERSION 3.10)
project(Howl C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(HOWL_EMBED_NNUE "Embed NNUE weights directly into the executable" OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(HOWL_EMBED_NNUE)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto")
    else()
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")
    endif()
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
endif()

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
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
    ChessStringManipulation.cpp
    ExchangeChessCache.cpp
    GameLogic.cpp
    HashMemoryBudget.cpp
    KingSetup.cpp
    LastFourMoves.cpp
    MissingInfoAboutPrevStateFromMove.cpp
    MoveLogic.cpp
    NNUEHistory.cpp
    EngineInitializer.cpp
    PVSSearch.cpp
    PassedPawnSetup.cpp
    PieceMoves.cpp
    PositionCore.cpp
    QSearcher.cpp
    RepetitionHistory.cpp
    Search.cpp
    NNUEEvaluator.cpp
    NNUEWeights.cpp
    Tablebase.cpp
    TranspositionTable.cpp
    UCI.cpp
    main.cpp
)

if(HOWL_EMBED_NNUE)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(EMBEDDED_NNUE_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/EmbeddedNNUEWeights.cpp")
    add_custom_command(
        OUTPUT "${EMBEDDED_NNUE_SOURCE}"
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/embed_nnue.py"
                "${CMAKE_CURRENT_SOURCE_DIR}/nnue.weights"
                "${EMBEDDED_NNUE_SOURCE}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/nnue.weights"
                "${CMAKE_CURRENT_SOURCE_DIR}/scripts/embed_nnue.py"
        COMMENT "Generating embedded NNUE weights source..."
        VERBATIM
    )
    list(APPEND HOWL_SOURCES "${EMBEDDED_NNUE_SOURCE}")
    add_compile_definitions(HOWL_EMBED_NNUE=1)
endif()

add_executable(howl ${HOWL_SOURCES})
target_link_libraries(howl PRIVATE fathom)

if(HOWL_EMBED_NNUE)
    if(WIN32)
        set_target_properties(howl PROPERTIES OUTPUT_NAME "howl-v11.0-windows-x86_64")
    else()
        set_target_properties(howl PROPERTIES OUTPUT_NAME "howl-v11.0-linux-x86_64")
    endif()
endif()
