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

fhg_add_test (NAME we_type_stencil_cache_has_correct_destruction_order
  SOURCES "stencil_cache_has_correct_destruction_order.cpp"
  ARGS --neighbors $<TARGET_FILE:stencil_cache_workflow_neighbors_callback>
  USE_BOOST
  LIBRARIES pnet
            stencil_cache_workflow_neighbors_callback
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
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

extended_add_executable (NAME mk_cpp
  SOURCES "mk_cpp.cpp"
  LIBRARIES pnet Util::Generic
)

add_custom_command (
  OUTPUT sig_struct.hpp sig_op.hpp sig_op.cpp
  COMMAND "$<TARGET_FILE:mk_cpp.exe>" -H sig_struct.hpp -O sig_op.hpp -I sig_op.cpp
)

extended_add_library (NAME sig_op
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/sig_op.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LIBRARIES Boost::base
)

fhg_add_test (NAME we_type_sig
  SOURCES sig.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
  LIBRARIES sig_op pnet
)

fhg_add_test (NAME we_type_get_schedule_data
  SOURCES get_schedule_data.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_dynamic_requirements
  SOURCES dynamic_requirements.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_transition_with_preferences
  SOURCES transition_with_preferences.cpp
  USE_BOOST
  LIBRARIES pnet
)
