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
    target_compile_definitions(howl_correctness_tests PRIVATE HOWL_CORRECTNESS_TESTING=1)

    add_executable(howl_benchmark
        tests/Benchmark.cpp
        ${TEST_ENGINE_SOURCES}
    )
    target_include_directories(howl_benchmark PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_definitions(howl_benchmark PRIVATE
        HOWL_BENCHMARK_RELEASE=$<IF:$<CONFIG:Release>,1,0>
        HOWL_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )

    add_executable(howl_memory_stress
        tests/MemoryStress.cpp
        ${TEST_ENGINE_SOURCES}
    )
    target_include_directories(howl_memory_stress PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    target_compile_definitions(howl_memory_stress PRIVATE
        HOWL_MEMORY_STRESS_RELEASE=$<IF:$<CONFIG:Release>,1,0>
        HOWL_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )

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

    foreach(SEARCH_CASE mate_in_one mate_in_two forced_evasion illegal_king_capture promotion_mate returned_move_legality lmr_research_accounting lmr_promotion_exemption lmr_equal_winning_capture_exemption repetition_real_knight repetition_false_a1 repetition_castling_difference repetition_ep_difference repetition_search_line repetition_undo_restoration mate_in_1_2_3_progression mate_search_ply_propagation mate_winning_and_losing_scores mate_illegal_sentinel_distinction null_move_ep_restoration null_move_pointer_lifetime nmp_zugzwang_pawn_endgame nmp_tactical_integrity futility_pruning_quiet_move_skip tt_entry_layout_and_packed_move)
        add_test(
            NAME search_${SEARCH_CASE}
            COMMAND howl_correctness_tests search ${SEARCH_CASE}
        )
    endforeach()

    foreach(QSEARCH_CASE in_check_stand_pat quiet_king_evasion quiet_block_evasion mate stalemate pseudo_legal_illegal research_accounting quiet_promotion promotion_capture underpromotion_knight underpromotion_rook promotion_check discovered_check ordinary_delta pawn_capture_recapture pawn_capture_free pawn_capture_forcing quiet_check checking_capture delta_boundary)
        add_test(
            NAME qsearch_${QSEARCH_CASE}
            COMMAND howl_correctness_tests qsearch ${QSEARCH_CASE}
        )
    endforeach()

    add_test(
        NAME cache_igg_depth_mapping
        COMMAND howl_correctness_tests search igg_depth_mapping
    )

    foreach(CACHE_CASE evaluation evaluation_rook_ordering evaluation_rook_connection evaluation_fixed_table exchange exchange_variants pawn_key exchange_promotion_key exchange_wrong_lookup exchange_field_collision exchange_attacker_bounds exchange_power_two_bounds exchange_legacy_equivalence exchange_fixed_table)
        add_test(
            NAME cache_${CACHE_CASE}
            COMMAND howl_correctness_tests cache ${CACHE_CASE}
        )
    endforeach()

    add_test(
        NAME lifecycle_fen_replacement
        COMMAND howl_correctness_tests lifecycle fen_replacement
    )
    add_test(
        NAME memory_hash_budget
        COMMAND howl_correctness_tests memory hash_budget
    )

    foreach(UCI_CASE movetime depth nodes stop quit time_control)
        add_test(
            NAME uci_${UCI_CASE}
            COMMAND howl_correctness_tests uci ${UCI_CASE}
        )
    endforeach()

    foreach(EVAL_CASE black_en_passant black_knight_attack bishop_pair_color king_danger_sign passed_pawn_table_symmetry king_safety_table_symmetry pawn_move_center_symmetry)
        add_test(
            NAME eval_${EVAL_CASE}
            COMMAND howl_correctness_tests eval ${EVAL_CASE}
        )
    endforeach()
    add_test(
        NAME search_tt_lmr_reduced_depth_provenance
        COMMAND howl_correctness_tests search tt_lmr_reduced_depth_provenance
    )
endif()

