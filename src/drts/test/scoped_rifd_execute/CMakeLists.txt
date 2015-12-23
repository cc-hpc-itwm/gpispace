
extended_add_executable (NAME test_binary
  SOURCES "test_binary.cpp"
  LIBRARIES rif-started_process_promise
            Util::Generic
            Boost::program_options
)
_create_bundle (test_binary.exe BUNDLE_PATH)

fhg_add_test (scoped_rifd_execute.cpp
  PROJECT drts
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --test-binary "$<TARGET_FILE:test_binary.exe>"
       --bundle-path "${BUNDLE_PATH}"
  LINK_LIBRARIES gspc
                 test-utilities
                 Boost::program_options
 REQUIRES_INSTALLATION
 START_SCOPED_RIF
)
