gspc_add_test (NAME doc_example_number_of_tokens_co_generate_expression
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/number_of_tokens/co_generate"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --net "expression"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
            fmt
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

gspc_add_test (NAME doc_example_number_of_tokens_co_generate_module
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/number_of_tokens/co_generate"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --net "module"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
            fmt
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
