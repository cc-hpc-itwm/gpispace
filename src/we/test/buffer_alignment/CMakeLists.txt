fhg_add_test (NAME we_buffer_alignment
  SOURCES "test.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
            net_using_buffers
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)

if (BUILD_TESTING)
  gspc_add_library (NAME net_using_buffers
    TYPE STATIC
    SOURCES "net_description.cpp"
            "nets_using_buffers.cpp"
    LIBRARIES we-dev
              Util::Generic
  )
endif()
