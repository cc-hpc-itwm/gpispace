# mirko.rahn@itwm.fraunhofer.de

extended_add_executable (NAME gen
  SOURCES "gen.cpp"
  LIBRARIES Util::Generic
            Boost::program_options
            Boost::thread
            Boost::system
)
target_include_directories (gen.exe
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)

fhg_add_test (NAME doc_tutorial_avg_stddev
  SOURCES test.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../"
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
