fhg_add_test (bytearray.cpp
  BOOST_UNIT_TEST
  LINK_LIBRARIES pnet fhg-util
)

fhg_add_test (value.cpp
  BOOST_UNIT_TEST
  PROJECT we_type
  LINK_LIBRARIES pnet
)

fhg_add_test (signature.cpp
  BOOST_UNIT_TEST
  PROJECT we_type
  LINK_LIBRARIES pnet
)

fhg_add_test (bitsetofint.cpp
  BOOST_UNIT_TEST
  PROJECT we_type
  LINK_LIBRARIES pnet
)

fhg_add_test (module_call.cpp
  BOOST_UNIT_TEST
  PROJECT we_type
  LINK_LIBRARIES pnet
)

extended_add_executable (NAME mk_cpp
  SOURCES "mk_cpp.cpp"
  LIBRARIES pnet Boost::program_options Util::Generic
)

add_custom_command (
  OUTPUT sig_struct.hpp sig_op.hpp sig_op.cpp
  COMMAND "$<TARGET_FILE:mk_cpp.exe>" -H sig_struct.hpp -O sig_op.hpp -I sig_op.cpp
)

extended_add_library (NAME sig_op
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/sig_op.cpp"
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}"
)

fhg_add_test (sig.cpp
  BOOST_UNIT_TEST
  PROJECT we
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LINK_LIBRARIES sig_op pnet
)

fhg_add_test (get_schedule_data.cpp
  BOOST_UNIT_TEST
  PROJECT we_type
  LINK_LIBRARIES pnet
)
