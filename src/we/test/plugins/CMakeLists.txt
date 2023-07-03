if (BUILD_TESTING)
  gspc_add_library (NAME Log
    NAMESPACE Plugin
    SOURCES "Log.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
  gspc_add_library (NAME Tunnel
    NAMESPACE Plugin
    SOURCES "Tunnel.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
endif()

function (test_Log case)
  fhg_add_test (NAME plugin_Log_${case}
    SOURCES test/Log/${case}/${case}.cpp
    USE_BOOST
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/test/Log/${case}"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --plugin-path "$<TARGET_FILE:Plugin::Log>"
    LIBRARIES test-utilities
              fhg-util
              Boost::program_options
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endfunction()

test_Log ("echo")
test_Log ("create_and_call_after_in_one_transition")
test_Log ("call_before_and_destroy_in_one_transition")

function (test_Tunnel case)
  fhg_add_test (NAME plugin_Tunnel_${case}
    SOURCES test/Tunnel/${case}/${case}.cpp
    USE_BOOST
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/test/Tunnel/${case}"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --plugin-path "$<TARGET_FILE:Plugin::Tunnel>"
    LIBRARIES test-utilities
              fhg-util
              Boost::program_options
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endfunction()

test_Tunnel (subnetwork)
