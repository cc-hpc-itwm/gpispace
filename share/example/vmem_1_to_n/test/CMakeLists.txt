# mirko.rahn@itwm.fraunhofer.de

math (EXPR num_bytes "192 << 20")

fhg_add_test (NAME share_example_vmem_1_to_n
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/.."
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --num-bytes ${num_bytes}
  LIBRARIES gspc
            test-utilities
            transform_file_type
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
