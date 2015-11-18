# mirko.rahn@itwm.fraunhofer.de

math (EXPR num_bytes "192 << 20")
math (EXPR size_virtual_memory "${num_bytes}")

fhg_add_test (test.cpp
  PROJECT share_example_vmem_1_to_n
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/.."
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-per-node ${size_virtual_memory}
       --virtual-memory-startup-timeout 60
       --num-bytes ${num_bytes}
  LINK_LIBRARIES gspc
                 test-utilities
                 transform_file_type
                 fhg-util
                 Boost::program_options
                 Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)
