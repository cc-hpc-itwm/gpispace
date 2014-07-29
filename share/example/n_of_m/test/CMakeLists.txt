# mirko.rahn@itwm.fraunhofer.de

fhg_add_application_test (example_n_of_m_we-exec SCRIPT test.we-exec)

include_directories (${UTIL_INCLUDE_DIR})
include_directories (${CMAKE_SOURCE_DIR})

fhg_add_test (test.cpp
  PROJECT share_example_n_of_m
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../"
       --command "${CMAKE_CURRENT_BINARY_DIR}/demo.bin"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc
                 test-utilities
                 ${UTIL_LIBRARIES}
                 ${Boost_PROGRAM_OPTIONS_LIBRARIES}
                 ${Boost_FILESYSTEM_LIBRARIES}
  RESOURCE_LOCK starts_drts_environment
  DEPENDS demo.bin
  REQUIRES_INSTALLATION
)


add_executable (demo.bin demo.c)
