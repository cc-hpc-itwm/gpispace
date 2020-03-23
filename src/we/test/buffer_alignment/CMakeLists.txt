fhg_add_test (NAME we_buffer_alignment
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
            net_using_buffers
  DEPENDS net_using_buffers
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)

extended_add_library (NAME net_using_buffers 
  TYPE STATIC 
  SOURCES "nets_using_buffers.cpp"
  INCLUDE_DIRECTORIES PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} 
  LIBRARIES Util::Generic 
)
