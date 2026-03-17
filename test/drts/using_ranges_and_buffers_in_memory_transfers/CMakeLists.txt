gspc_add_test (NAME using_ranges_and_buffers_in_memory_transfers
  SOURCES "test.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
            Test::build_net_description
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)

gspc_add_library (NAME build_net_description
  NAMESPACE Test
  TYPE STATIC
  SOURCES "net_description.cpp"
  LIBRARIES gspc_util fmt gspc_testing
)
