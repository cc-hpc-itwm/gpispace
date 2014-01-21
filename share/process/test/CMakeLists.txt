
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_executable (interleave interleave.c)
add_executable (deinterleave deinterleave.c)

add_executable (rotate rotate.c)
set_target_properties (rotate PROPERTIES COMPILE_FLAGS -std=c99)

fhg_add_test (file_input.cpp
  PROJECT share_process
  BOOST_UNIT_TEST
  ARGS ${CMAKE_CURRENT_BINARY_DIR}/interleave
  LINK_LIBRARIES process fhg-util
  DEPENDS interleave
)

fhg_add_test (file_output.cpp
  PROJECT share_process
  BOOST_UNIT_TEST
  ARGS ${CMAKE_CURRENT_BINARY_DIR}/deinterleave
  LINK_LIBRARIES process fhg-util
  DEPENDS deinterleave
)

fhg_add_test (all.cpp
  PROJECT share_process
  BOOST_UNIT_TEST
  ARGS ${CMAKE_CURRENT_BINARY_DIR}/rotate
  LINK_LIBRARIES process
  DEPENDS rotate
)

fhg_add_test (redefine.cpp
  PROJECT share_process
  BOOST_UNIT_TEST
  LINK_LIBRARIES process
)

fhg_add_test (big.cpp
  PROJECT share_process
  BOOST_UNIT_TEST
  ARGS 1048576
  LINK_LIBRARIES process
)

fhg_add_test (basics.cpp
  PROJECT share_process
  BOOST_UNIT_TEST
  LINK_LIBRARIES process
)
