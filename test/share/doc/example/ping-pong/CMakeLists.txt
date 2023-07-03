fhg_add_test (NAME doc_example_ping-pong
  SOURCES test.cpp
  USE_BOOST
  PERFORMANCE_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/ping-pong"
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
