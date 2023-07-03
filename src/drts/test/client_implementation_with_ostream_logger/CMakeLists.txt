if (BUILD_TESTING)
  gspc_add_library (NAME client_implementation_with_ostream_logger
    SOURCES "client_implementation_with_ostream_logger.cpp"
    TYPE MODULE
    LIBRARIES GPISpace::header-only
  )
endif()

fhg_add_test (NAME drts_client_implementation_with_ostream_logger
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --implementation "$<TARGET_FILE:client_implementation_with_ostream_logger>"
  LIBRARIES gspc
            test-utilities
            fhg-util
            gspc::logging
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
