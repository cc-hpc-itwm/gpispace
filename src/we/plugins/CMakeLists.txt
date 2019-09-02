extended_add_library (NAME Log
  NAMESPACE Plugin
  SOURCES "Log.cpp"
  POSITION_INDEPENDENT
  TYPE SHARED
  LIBRARIES pnet
  INSTALL
  INSTALL_DESTINATION "plugin"
)
extended_add_library (NAME StencilCache
  NAMESPACE Plugin
  SOURCES "StencilCache.cpp"
  POSITION_INDEPENDENT
  TYPE SHARED
  LIBRARIES pnet
  INSTALL
  INSTALL_DESTINATION "plugin"
)

function (test_Log case)
  fhg_add_test (NAME plugin_Log_${case}
    SOURCES test/Log/${case}/${case}.cpp
    USE_BOOST
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/test/Log/${case}"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --plugin-path "${CMAKE_INSTALL_PREFIX}/plugin/$<TARGET_FILE_NAME:Plugin::Log>"
    LIBRARIES gspc
              test-utilities
              fhg-util
              Boost::program_options
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endfunction()

test_Log ("echo")
test_Log ("create_and_call_after_in_one_transition")
test_Log ("call_before_and_destroy_in_one_transition")
