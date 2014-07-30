# petry@itwm.fraunhofer.de

include_directories (${UTIL_INCLUDE_DIR})
include_directories (${CMAKE_SOURCE_DIR})

fhg_add_test (test.cpp
  PROJECT share_selftest
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../xml/"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc
                 test-utilities
                 ${UTIL_LIBRARIES}
                 ${Boost_PROGRAM_OPTIONS_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARIES}
  RESOURCE_LOCK starts_drts_environment
  REQUIRES_INSTALLATION
)
