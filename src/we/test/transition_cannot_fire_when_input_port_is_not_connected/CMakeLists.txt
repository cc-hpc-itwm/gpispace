fhg_add_test (NAME we_transition_cannot_fire_when_input_port_is_not_connected
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
