fhg_add_test (NAME we_type_bytearray
  SOURCES bytearray.cpp
  USE_BOOST
  LIBRARIES pnet fhg-util
)

fhg_add_test (NAME we_type_value
  SOURCES value.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_signature
  SOURCES signature.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_bitsetofint
  SOURCES bitsetofint.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_module_call
  SOURCES module_call.cpp
  USE_BOOST
  LIBRARIES pnet
)

add_executable (mk_cpp mk_cpp.cpp)
target_link_libraries (mk_cpp pnet Boost::program_options Util::Generic)

add_custom_command (
  OUTPUT sig_struct.hpp sig_op.hpp sig_op.cpp
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mk_cpp -H sig_struct.hpp -O sig_op.hpp -I sig_op.cpp
  DEPENDS mk_cpp
)

extended_add_library (NAME sig_op
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/sig_op.cpp"
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}"
)

fhg_add_test (NAME we_type_sig
  SOURCES sig.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LIBRARIES sig_op pnet
)

fhg_add_test (NAME we_type_get_schedule_data
  SOURCES get_schedule_data.cpp
  USE_BOOST
  LIBRARIES pnet
)
