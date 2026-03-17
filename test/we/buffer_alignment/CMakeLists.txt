gspc_add_test (NAME we_buffer_alignment
  SOURCES "test.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
            net_using_buffers
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)

gspc_add_library (NAME net_using_buffers
  TYPE STATIC
  SOURCES "net_description.cpp"
          "nets_using_buffers.cpp"
  LIBRARIES gspc_we
            gspc_util
            gspc_testing
)
