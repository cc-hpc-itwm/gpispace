gspc_add_test (NAME drts_multiple_module_implementations
  SOURCES test.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --fmt-directory "${FMT_DIR}"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
