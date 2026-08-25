cmake_minimum_required(VERSION 3.10)
project(Howl)
set(CMAKE_CXX_STANDARD 17)
file(GLOB SOURCES "*.cpp")
add_executable(howl ${SOURCES})

include(CTest)

if(BUILD_TESTING)
    set(TEST_ENGINE_SOURCES ${SOURCES})
    list(REMOVE_ITEM TEST_ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

    add_executable(howl_correctness_tests
        tests/CorrectnessTests.cpp
        ${TEST_ENGINE_SOURCES}
    )
    target_include_directories(howl_correctness_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

    foreach(POSITION start kiwipete position3 position4 position5 position6)
        add_test(
            NAME perft_${POSITION}
            COMMAND howl_correctness_tests perft ${POSITION}
        )
        add_test(
            NAME restoration_${POSITION}
            COMMAND howl_correctness_tests restoration ${POSITION}
        )
    endforeach()

    foreach(ZOBRIST_CASE start_parity move_order fen_vs_moves side_to_move castling_rights en_passant_rank3 en_passant_rank6)
        add_test(
            NAME zobrist_${ZOBRIST_CASE}
            COMMAND howl_correctness_tests zobrist ${ZOBRIST_CASE}
        )
    endforeach()

    foreach(SEARCH_CASE mate_in_one mate_in_two forced_evasion illegal_king_capture promotion_mate returned_move_legality)
        add_test(
            NAME search_${SEARCH_CASE}
            COMMAND howl_correctness_tests search ${SEARCH_CASE}
        )
    endforeach()
endif()
