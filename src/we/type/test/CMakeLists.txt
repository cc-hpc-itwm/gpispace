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
  SOURCES "${CMAKE_BINARY_DIR}/we/type/test/signature.cpp"
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_bitsetofint
  SOURCES bitsetofint.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_ModuleCall
  SOURCES ModuleCall.cpp
  USE_BOOST
  LIBRARIES pnet
)

gspc_add_executable (NAME mk_cpp
  SOURCES "mk_cpp.cpp"
  LIBRARIES pnet Util::Generic
)

fhg_add_test (NAME we_type_pnetc_compare
  SOURCES "pnetc_compare.cpp"
  USE_BOOST
  LIBRARIES pnet
)

if (BUILD_TESTING)
  add_custom_command (
    OUTPUT sig_struct.hpp sig_op.hpp sig_op.cpp
    COMMAND "$<TARGET_FILE:mk_cpp.exe>"
  )

  gspc_add_library (NAME sig_op
    TYPE STATIC
    SOURCES "${CMAKE_CURRENT_BINARY_DIR}/sig_op.cpp"
    INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
    LIBRARIES pnet
              Boost::base
  )

  fhg_add_test (NAME we_type_sig
    SOURCES sig.cpp
    USE_BOOST
    INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
    LIBRARIES sig_op pnet
  )
endif()

fhg_add_test (NAME we_type_get_schedule_data
  SOURCES get_schedule_data.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_type_dynamic_requirements
  SOURCES dynamic_requirements.cpp
  USE_BOOST
  LIBRARIES pnet we::test_equality
)

fhg_add_test (NAME we_type_transition_with_preferences
  SOURCES transition_with_preferences.cpp
  USE_BOOST
  LIBRARIES pnet we::test_equality
)

fhg_add_test (NAME we_type_MemoryBufferInfo
  SOURCES MemoryBufferInfo.cpp
  USE_BOOST
  LIBRARIES pnet we_expr_type_testing
)

fhg_add_test (NAME we_type_memory_transfer
  SOURCES "memory_transfer.cpp"
  USE_BOOST
  LIBRARIES pnet we_expr_type_testing
)

fhg_add_test (NAME we_type_Transition
  SOURCES "Transition.cpp"
  USE_BOOST
  LIBRARIES pnet we_expr_type_testing
)
