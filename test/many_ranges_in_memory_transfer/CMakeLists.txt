fhg_add_test (NAME many_ranges_in_memory_transfer
  SOURCES "driver.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --size-of-range 1000
       --distance-between-ranges 24
       --number-of-ranges 16384
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)

fhg_add_test (NAME many_ranges_in_memory_transfer_minimal
  SOURCES "driver.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --size-of-range 1
       --distance-between-ranges 0
       --number-of-ranges 16384
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
