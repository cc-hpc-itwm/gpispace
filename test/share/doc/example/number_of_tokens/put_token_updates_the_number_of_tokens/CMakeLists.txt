gspc_add_test (NAME doc_example_number_of_tokens_put_token_updates_the_number_of_tokens
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/number_of_tokens/put_token_updates_the_number_of_tokens"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
