# {mirko.rahn,bernd.loerwald}@itwm.fraunhofer.de

# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (test_bytearray.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet
)

fhg_add_test (value.cpp
  BOOST_UNIT_TEST
  PROJECT test_we_type
  LINK_LIBRARIES pnet
)

fhg_add_test (signature.cpp
  BOOST_UNIT_TEST
  PROJECT test_we_type
  LINK_LIBRARIES pnet
)

fhg_add_test (bitsetofint.cpp
  PROJECT test_we_type
  LINK_LIBRARIES pnet
)

fhg_add_test (property.cpp
  BOOST_UNIT_TEST
  PROJECT test_we_type
  LINK_LIBRARIES pnet
)

fhg_add_test (trans.cpp
  PROJECT test_we_type
  LINK_LIBRARIES pnet
)

add_executable (mk_cpp mk_cpp.cpp)
target_link_libraries (mk_cpp pnet ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_custom_command (
  OUTPUT sig_struct.hpp sig_op.hpp sig_op.cpp
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mk_cpp -H sig_struct.hpp -O sig_op.hpp -I sig_op.cpp
  DEPENDS mk_cpp
)

include_directories (${CMAKE_CURRENT_BINARY_DIR})
add_library (sig_op ${CMAKE_CURRENT_BINARY_DIR}/sig_op.cpp)

fhg_add_test (sig.cpp
  BOOST_UNIT_TEST
  PROJECT test_we
  LINK_LIBRARIES sig_op pnet
)
