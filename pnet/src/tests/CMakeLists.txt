find_package (LibReadline REQUIRED QUIET)
include_directories (SYSTEM ${LibReadline_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(tests
    test_activity.cpp
    test_adjacency.cpp
    test_bitset.cpp
    test_bitsetofint.cpp
    test_execute_activity.cpp
    test_expr_parser.cpp
    test_expr_rename.cpp
    test_expr_get_names.cpp
    test_expr_simplify.cpp
    test_layer_atomics.cpp
    test_layer_cancel.cpp
    test_load_module.cpp
    test_port.cpp
    test_property.cpp
    test_queue.cpp
    test_signal.cpp
    test_trans.cpp
    test_unordered_map.cpp
    test_sig_cpp.cpp
    test_try_gen.cpp
    test_gen.cpp
    test_context.cpp
    test_literal_show.cpp
   )

foreach (test ${tests})
  fhg_add_test (${test}
    PROJECT we
    LINK_LIBRARIES pnet
                   we2
                   ${FhgLog_LIBRARY}
                   ${UTIL_LIBRARIES}
                   ${Boost_UNIT_TEST_LIBRARIES}
                   ${Boost_LIBRARIES}
                   ${LibReadline_LIBRARY}
                   ${CMAKE_DL_LIBS}
  )
endforeach()

fhg_add_test (evaluate_net.cpp
  PROJECT test_we
  LINK_LIBRARIES pnet ${FhgLog_LIBRARY} ${CMAKE_DL_LIBS}
  DESCRIPTION "evaluate various nets and compare the results with given reference nets"
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/pnets
)

set (cross_hpp ${CMAKE_CURRENT_BINARY_DIR}/cross.hpp)
add_custom_command (
  OUTPUT ${cross_hpp}
  COMMAND we_test_sig_cpp > ${cross_hpp}
  DEPENDS we_test_sig_cpp
)

fhg_add_test (test_sig_cpp_result.cpp ${cross_hpp}
  PROJECT we
  LINK_LIBRARIES pnet ${Boost_LIBRARIES}
  DESCRIPTION "signature to cpp"
)

fhg_add_test (test_loader.cpp
  PROJECT we
  LINK_LIBRARIES pnet we-dev ${FhgLog_LIBRARY} ${CMAKE_DL_LIBS}
  DEPENDS question answer
)

add_library (answer SHARED answer.cpp)
add_library (question SHARED question.cpp)
target_link_libraries (question answer)
target_link_libraries (answer pnet)
