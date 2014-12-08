# mirko.rahn@itwm.fraunhofer.de

include_directories (${UTIL_INCLUDE_DIR})
include_directories (${CMAKE_SOURCE_DIR})

math (EXPR size_communication_buffer "8 * 4 << 20")

math (EXPR num_bytes "192 << 20")
math (EXPR size_virtual_memory "${num_bytes} + ${size_communication_buffer}")

fhg_add_test (test.cpp
  PROJECT share_example_vmem_1_to_n
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/.."
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-per-node ${size_virtual_memory}
       --virtual-memory-startup-timeout 60
       --virtual-memory-port 10820
       --num-bytes ${num_bytes}
  LINK_LIBRARIES gspc
                 test-utilities
                 transform_file_type
                 ${UTIL_LIBRARIES}
                 ${Boost_PROGRAM_OPTIONS_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARIES}
  RESOURCE_LOCK requires_virtual_memory
  REQUIRES_INSTALLATION
)
