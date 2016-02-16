extended_add_library (NAME client_implementation_with_ostream_logger
  SOURCES "client_implementation_with_ostream_logger.cpp"
  TYPE MODULE
)

fhg_add_test (test.cpp
  PROJECT drts_client_implementation_with_ostream_logger
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --implementation "$<TARGET_FILE:client_implementation_with_ostream_logger>"
  LINK_LIBRARIES gspc
                 test-utilities
                 fhg-util
                 Boost::program_options
                 Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
