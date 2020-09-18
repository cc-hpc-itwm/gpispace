
extended_add_executable (NAME test_binary
  SOURCES "test_binary.cpp"
  LIBRARIES rif-started_process_promise
            Util::Generic
            Boost::program_options
)
_create_bundle (test_binary.exe BUNDLE_PATH)

fhg_add_test (NAME drts_scoped_rifd_execute
  SOURCES scoped_rifd_execute.cpp
  USE_BOOST
  ARGS --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --test-binary "$<TARGET_FILE:test_binary.exe>"
       --bundle-path "${BUNDLE_PATH}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
 REQUIRES_INSTALLATION
 START_SCOPED_RIF
)
