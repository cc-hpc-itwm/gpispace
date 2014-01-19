
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_executable (interleave interleave.c)
add_executable (deinterleave deinterleave.c)

add_executable (rotate rotate.c)
set_target_properties (rotate PROPERTIES COMPILE_FLAGS -std=c99)

fhg_add_test (test_process_file_input.cpp
  BOOST_UNIT_TEST
  ARGS ${CMAKE_CURRENT_BINARY_DIR}/interleave
  LINK_LIBRARIES process
  DEPENDS interleave
)

fhg_add_test (test_process_file_output.cpp
  BOOST_UNIT_TEST
  ARGS ${CMAKE_CURRENT_BINARY_DIR}/deinterleave
  LINK_LIBRARIES process
  DEPENDS deinterleave
)

fhg_add_test (test_process_all.cpp
  BOOST_UNIT_TEST
  ARGS ${CMAKE_CURRENT_BINARY_DIR}/rotate
  LINK_LIBRARIES process
  DEPENDS rotate
)

fhg_add_test (test_process_redefine.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES process
)

fhg_add_test (test_process_big.cpp
  BOOST_UNIT_TEST
  ARGS 1048576
  LINK_LIBRARIES process
)

fhg_add_test (test_basics.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES process
)
