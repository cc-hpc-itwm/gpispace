# mirko.rahn@itwm.fraunhofer.de

add_executable (gen.bin gen.cpp)
target_include_directories (gen.bin
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_link_libraries (gen.bin
  Util::Generic
  Boost::program_options
  Boost::thread
  Boost::system
  pthread
)

fhg_add_test (NAME doc_tutorial_avg_stddev
  SOURCES test.cpp
  USE_BOOST
  INCLUDE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --generator "${CMAKE_CURRENT_BINARY_DIR}/gen.bin"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
