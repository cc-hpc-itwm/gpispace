fhg_add_test (NAME doc_example_pi
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/pi"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
