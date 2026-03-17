gspc_add_executable (NAME gen
  SOURCES "gen.cpp"
  LIBRARIES gspc_util
            Boost::program_options
            Boost::thread
            Boost::system
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/share/doc/example/avg_stddev/include"
)

gspc_add_test (NAME doc_example_avg_stddev
  SOURCES test.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/share/doc/example/avg_stddev/include"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/avg_stddev"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --generator "$<TARGET_FILE:gen.exe>"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
