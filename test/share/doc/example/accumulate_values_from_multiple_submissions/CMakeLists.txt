fhg_add_test (NAME doc_example_accumulate_values_from_multiple_submissions
  SOURCES driver.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/accumulate_values_from_multiple_submissions"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
