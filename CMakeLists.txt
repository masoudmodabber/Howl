cmake_minimum_required(VERSION 3.10)
project(Howl C CXX)
set(CMAKE_CXX_STANDARD 17)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -flto")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
endif()

add_library(fathom STATIC
    third_party/fathom/src/tbprobe.c
)
target_include_directories(fathom PUBLIC third_party/fathom/src)
file(GLOB SOURCES "*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tools_scratch_test_node_counts.cpp")
add_executable(howl ${SOURCES})
target_link_libraries(howl PRIVATE fathom)

include(CTest)

if(BUILD_TESTING)
    set(TEST_ENGINE_SOURCES ${SOURCES})
    list(REMOVE_ITEM TEST_ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    add_executable(howl_structured_nnue_weights_test
        tests/StructuredNNUEWeightsTest.cpp StructuredNNUEWeights.cpp
    )
    target_include_directories(howl_structured_nnue_weights_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    add_executable(howl_structured_nnue_rebuild_test
        tests/StructuredNNUERebuildTest.cpp StructuredNNUEEvaluator.cpp StructuredNNUEWeights.cpp Board.cpp PositionCore.cpp BoardInitializer.cpp Option.cpp
    )
    target_include_directories(howl_structured_nnue_rebuild_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    add_executable(howl_structured_nnue_incremental_validator
        tests/StructuredNNUEIncrementalValidator.cpp
        ${TEST_ENGINE_SOURCES}
    )
    target_include_directories(howl_structured_nnue_incremental_validator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(howl_structured_nnue_incremental_validator PRIVATE fathom)
    add_executable(howl_correctness_tests
        tests/CorrectnessTests.cpp
        ${TEST_ENGINE_SOURCES}
    )
    target_include_directories(howl_correctness_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_definitions(howl_correctness_tests PRIVATE HOWL_CORRECTNESS_TESTING=1)
    target_compile_definitions(howl_correctness_tests PRIVATE HOWL_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(howl_correctness_tests PRIVATE fathom)

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

    foreach(SEARCH_CASE mate_in_one mate_in_two forced_evasion illegal_king_capture promotion_mate returned_move_legality lmr_research_accounting lmr_promotion_exemption lmr_equal_winning_capture_exemption repetition_real_knight repetition_false_a1 repetition_castling_difference repetition_ep_difference repetition_search_line repetition_undo_restoration mate_in_1_2_3_progression mate_search_ply_propagation mate_winning_and_losing_scores mate_illegal_sentinel_distinction null_move_ep_restoration null_move_pointer_lifetime nmp_zugzwang_pawn_endgame nmp_tactical_integrity nmp_sparse_minor_endgame_guard tt_entry_layout_and_packed_move interrupted_iteration_bestmove single_pv_iteration_reporting aspiration_policy)
        add_test(
            NAME search_${SEARCH_CASE}
            COMMAND howl_correctness_tests search ${SEARCH_CASE}
        )
    endforeach()

    foreach(QSEARCH_CASE in_check_stand_pat quiet_king_evasion quiet_block_evasion mate stalemate pseudo_legal_illegal quiet_promotion promotion_capture underpromotion_knight underpromotion_rook promotion_check discovered_check ordinary_delta pawn_capture_recapture pawn_capture_free pawn_capture_forcing quiet_check checking_capture delta_boundary movegen_correctness)
        add_test(
            NAME qsearch_${QSEARCH_CASE}
            COMMAND howl_correctness_tests qsearch ${QSEARCH_CASE}
        )
    endforeach()

    foreach(CACHE_CASE exchange exchange_variants exchange_promotion_key exchange_wrong_lookup exchange_field_collision exchange_attacker_bounds exchange_power_two_bounds exchange_legacy_equivalence exchange_fixed_table)
        add_test(
            NAME cache_${CACHE_CASE}
            COMMAND howl_correctness_tests cache ${CACHE_CASE}
        )
    endforeach()

    add_test(
        NAME lifecycle_fen_replacement
        COMMAND howl_correctness_tests lifecycle fen_replacement
    )
    foreach(UCI_CASE movetime depth nodes stop quit time_control)
        add_test(
            NAME uci_${UCI_CASE}
            COMMAND howl_correctness_tests uci ${UCI_CASE}
        )
    endforeach()

    foreach(TABLEBASE_CASE disabled invalid_path above_limit unsupported_state missing_king win draw loss search_result)
        add_test(
            NAME tablebase_${TABLEBASE_CASE}
            COMMAND howl_correctness_tests tablebase ${TABLEBASE_CASE}
        )
    endforeach()

    add_test(
        NAME search_multipv_correctness
        COMMAND howl_correctness_tests search multipv_correctness
    )
endif()
