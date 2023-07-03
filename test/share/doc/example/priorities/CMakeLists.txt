fhg_add_test (NAME doc_example_priorities_sum
  SOURCES sum.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/priorities"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

fhg_add_test (NAME share_example_priorities_sum_and_conditional_product
  SOURCES sum_and_conditional_product.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/priorities"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
