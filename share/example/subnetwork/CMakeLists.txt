math (EXPR block_size "1 << 18")
math (EXPR number_of_blocks "1 << 8")
math (EXPR memory_total "1 << 20")

fhg_add_test (NAME share_example_subnetwork
  SOURCES driver.cpp
  USE_BOOST
  ARGS --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --block-size ${block_size}
       --number-of-blocks ${number_of_blocks}
       --memory-total ${memory_total}
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
