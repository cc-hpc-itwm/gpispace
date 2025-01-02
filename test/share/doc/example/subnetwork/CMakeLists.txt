math (EXPR block_size "1 << 18")
math (EXPR number_of_blocks "1 << 8")
math (EXPR memory_total "1 << 20")

fhg_add_test (NAME doc_example_subnetwork
  SOURCES driver.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/subnetwork"
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
            fmt
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
