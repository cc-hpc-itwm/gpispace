if (BUILD_TESTING)
  gspc_add_library (NAME NumberOfTokens
    NAMESPACE Plugin
    SOURCES "${PROJECT_SOURCE_DIR}/share/doc/example/number_of_tokens/plugin_put_token_updates_the_number_of_tokens/Plugin.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
endif()

fhg_add_test (NAME doc_example_number_plugin_put_token_updates_the_number_of_tokens
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/number_of_tokens/plugin_put_token_updates_the_number_of_tokens"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --plugin-path "$<TARGET_FILE:Plugin::NumberOfTokens>"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
