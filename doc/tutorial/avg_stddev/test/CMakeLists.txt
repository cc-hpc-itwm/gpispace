# mirko.rahn@itwm.fraunhofer.de

add_executable (gen.bin gen.cpp)
target_include_directories (gen.bin
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
target_link_libraries (gen.bin
  Util::Generic
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  pthread
)

fhg_add_test (test.cpp
  PROJECT doc_tutorial_avg_stddev
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
                      PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --generator "${CMAKE_CURRENT_BINARY_DIR}/gen.bin"
  LINK_LIBRARIES gspc
                 test-utilities
                 fhg-util
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
