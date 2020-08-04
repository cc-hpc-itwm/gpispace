fhg_add_test (NAME drts_work_stealing_single_module
  SOURCES test.cpp
  USE_BOOST
  ARGS --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --test-net "net_with_single_module_implementation"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

fhg_add_test (NAME drts_work_stealing_multiple_module
  SOURCES test.cpp
  USE_BOOST
  ARGS --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --test-net "net_with_multiple_module_implementations"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  RUN_SERIAL # actually a performance test but a fundamental one we
             # don't want to ignore
)
