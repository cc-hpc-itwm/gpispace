# -*- mode: cmake; -*-

# search for libreadline
find_package (LibReadline REQUIRED)
include_directories (${LibReadline_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(tc_srcs
    test_activity.cpp
    test_adjacency.cpp
    test_bignet.cpp
    test_bitset.cpp
    test_bitsetofint.cpp
    test_bitset_token.cpp
    test_choice_cond_loop.cpp
    test_choice_mapreduce.cpp
    test_cond.cpp
    test_control_loop_exp.cpp
    test_control_loop_with_choice_and_read.cpp
    test_control_loop_with_choice.cpp
    test_control_loop_with_choice_no_max_place.cpp
    test_count_number_of_copies.cpp
    test_cross.cpp
    test_execute_activity.cpp
    test_expr_parser.cpp
    test_expr_parser_ite.cpp
    test_expr_rename.cpp
    test_expr_get_names.cpp
    test_expr_simplify.cpp
    test_generic_permute.cpp
    test_independent_loops.cpp
    test_kirchhoff.cpp
    test_kirchhoff_v2.cpp
    test_kirchhoff_v3.cpp
    test_kirchhoff_v4.cpp
    test_layer_atomics.cpp
    test_layer_cancel.cpp
    test_loader.cpp
    test_load_module.cpp
    test_multirel.cpp
    test_net_basics.cpp
    test_parallel_execution_factored_out.cpp
    test_parallel_execution_with_extractor_and_injector.cpp
    test_port.cpp
    test_preference.cpp
    test_property.cpp
    test_queue.cpp
    test_signal.cpp
    test_structured_token.cpp
    test_swap.cpp
    test_timed_parallel_execution.cpp
    test_trans.cpp
    test_unordered_map.cpp
    test_user_defined_transition_function.cpp
    test_sig_cpp.cpp
    test_try_gen.cpp
    test_gen.cpp
   )

# answer module
add_library(answer SHARED answer.cpp)
add_library(question SHARED question.cpp)
target_link_libraries(question answer)

function(construct_tests )
  foreach (tc_src ${ARGN})
    # get the filename without extension
    string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_${PROJECT_NAME}_\\2" tc_name ${tc_src})
    message(STATUS "adding test case ${tc_name} (using ${tc_src})")

    add_executable(${tc_name} ${tc_src})
    target_link_libraries(${tc_name} ${WE_PRECOMPILE_LIBRARIES})
    target_link_libraries(${tc_name} ${FhgLog_LIBRARY})
    target_link_libraries(${tc_name} ${Boost_UNIT_TEST_LIBRARIES})
    target_link_libraries(${tc_name} ${Boost_LIBRARIES})
    target_link_libraries(${tc_name} ${LibReadline_LIBRARY})
    target_link_libraries(${tc_name} dl)
    get_target_property(LOC ${tc_name} LOCATION_)
    add_test(${tc_name} ${LOC})
  endforeach(tc_src ${tc_srcs})
endfunction(construct_tests)

list(SORT tc_srcs)
construct_tests(${tc_srcs})

set(cross_hpp
    ${CMAKE_CURRENT_BINARY_DIR}/cross.hpp
   )
set(cross_gen
    test_${PROJECT_NAME}_sig_cpp
   )
add_custom_command(OUTPUT ${cross_hpp}
                   COMMAND ${cross_gen} > ${cross_hpp}
                   DEPENDS ${cross_gen}
                  )

set(tc_src
    test_sig_cpp_result.cpp
   )
string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_${PROJECT_NAME}_\\2" tc_name ${tc_src})
add_executable(${tc_name} ${tc_src} ${cross_hpp})
target_link_libraries(${tc_name} ${Boost_LIBRARIES})
get_target_property(LOC ${tc_name} LOCATION_)
add_test(${tc_name} ${LOC})

set (gen_srcs
     generate_map_reduce.cpp
    )
list(SORT gen_srcs)
construct_tests(${gen_srcs})

string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_${PROJECT_NAME}_\\2" tc_name test_loader.cpp)
add_dependencies(${tc_name} question answer)
