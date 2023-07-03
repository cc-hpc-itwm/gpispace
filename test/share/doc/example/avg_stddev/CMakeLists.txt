if (BUILD_TESTING)
  gspc_add_executable (NAME gen
    SOURCES "gen.cpp"
    LIBRARIES Util::Generic
              Boost::program_options
              Boost::thread
              Boost::system
    INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/share/doc/example/avg_stddev/include"
  )
endif()

fhg_add_test (NAME doc_example_avg_stddev
  SOURCES test.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/share/doc/example/avg_stddev/include"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/doc/example/avg_stddev"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --generator "$<TARGET_FILE:gen.exe>"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
