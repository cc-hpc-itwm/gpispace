# -*- mode: cmake; -*-

include (fhg_test_macros)

# search for libreadline
find_package (LibReadline REQUIRED)
include_directories (${LibReadline_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(tc_srcs
    test_activity.cpp
    test_adjacency.cpp
    test_bitset.cpp
    test_bitsetofint.cpp
    test_cross.cpp
    test_execute_activity.cpp
    test_expr_parser.cpp
    test_expr_parser_ite.cpp
    test_expr_rename.cpp
    test_expr_get_names.cpp
    test_expr_simplify.cpp
    test_layer_atomics.cpp
    test_layer_cancel.cpp
    test_loader.cpp
    test_load_module.cpp
    test_port.cpp
#    test_requirements.cpp
    test_property.cpp
    test_queue.cpp
    test_signal.cpp
    test_trans.cpp
    test_unordered_map.cpp
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
    string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_we_\\2" tc_name ${tc_src})
    message(STATUS "adding test case ${tc_name} (using ${tc_src})")

    add_executable(${tc_name} ${tc_src})
    target_link_libraries(${tc_name} we)
    target_link_libraries(${tc_name} ${FhgLog_LIBRARY})
    target_link_libraries(${tc_name} ${UTIL_LIBRARIES})
    target_link_libraries(${tc_name} ${Boost_UNIT_TEST_LIBRARIES})
    target_link_libraries(${tc_name} ${Boost_LIBRARIES})
    target_link_libraries(${tc_name} ${LibReadline_LIBRARY})
    target_link_libraries(${tc_name} ${CMAKE_DL_LIBS})
    get_target_property(LOC ${tc_name} LOCATION_)
    add_test(${tc_name} ${LOC})
  endforeach(tc_src ${tc_srcs})
endfunction(construct_tests)

list(SORT tc_srcs)
construct_tests(${tc_srcs})

fhg_add_test (evaluate_net.cpp
  LINK_LIBRARIES we ${FhgLog_LIBRARY} ${CMAKE_DL_LIBS}
  DESCRIPTION "evaluate various nets and compare the results with given reference nets"
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/pnets
)

set(cross_hpp
    ${CMAKE_CURRENT_BINARY_DIR}/cross.hpp
   )
set(cross_gen test_we_sig_cpp)
add_custom_command(OUTPUT ${cross_hpp}
                   COMMAND ${cross_gen} > ${cross_hpp}
                   DEPENDS ${cross_gen}
                  )

set(tc_src
    test_sig_cpp_result.cpp
   )
string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_we_\\2" tc_name ${tc_src})
add_executable(${tc_name} ${tc_src} ${cross_hpp})
target_link_libraries(${tc_name} ${Boost_LIBRARIES})
get_target_property(LOC ${tc_name} LOCATION_)
add_test(${tc_name} ${LOC})

string(REGEX REPLACE "(.*/)?test_(.*)\\.c.*" "test_we_\\2" tc_name test_loader.cpp)
add_dependencies(${tc_name} question answer)
