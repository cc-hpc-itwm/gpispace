fhg_add_test (NAME share_example_ping-pong
  SOURCES test.cpp
  USE_BOOST
  PERFORMANCE_TEST
  ARGS --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/.."
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  RUN_SERIAL # actually a performance test but a fundamental one we
             # don't want to ignore
)
