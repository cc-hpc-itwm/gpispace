fhg_add_test (NAME doc_example_n_of_m_with_credit
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/n_of_m_with_credit"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
