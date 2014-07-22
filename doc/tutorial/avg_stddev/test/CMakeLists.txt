# mirko.rahn@itwm.fraunhofer.de

fhg_add_application_test (tutorial_avg_stddev_we-exec SCRIPT test.we-exec)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_executable (gen.bin gen.cpp)
target_link_libraries (gen.bin
  ${Boost_PROGRAM_OPTIONS_LIBRARIES}
  ${Boost_THREAD_LIBRARIES}
  ${Boost_SYSTEM_LIBRARIES}
  pthread
)

include_directories (${UTIL_INCLUDE_DIR})
include_directories (${CMAKE_SOURCE_DIR})

fhg_add_test (test.cpp
  PROJECT doc_tutorial_avg_stddev
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --generator "${CMAKE_CURRENT_BINARY_DIR}/gen.bin"
  LINK_LIBRARIES gspc
                 test-utilities
                 ${UTIL_LIBRARIES}
                 ${Boost_PROGRAM_OPTIONS_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARIES}
  RESOURCE_LOCK starts_drts_environment
)
