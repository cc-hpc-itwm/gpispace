fhg_add_test (NAME drts_scheduler.performance
  SOURCES network_description.cpp
          test.cpp
  USE_BOOST
  PERFORMANCE_TEST
  RUN_SERIAL
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --num-workers 100
       --num-tasks 20000
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
