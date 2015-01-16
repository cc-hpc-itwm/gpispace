# mirko.rahn@itwm.fraunhofer.de

include_directories (${CMAKE_SOURCE_DIR})

fhg_add_test (test.cpp
  PROJECT we_transition_cannot_fire_when_input_port_is_not_connected
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc
                 test-utilities
                 ${UTIL_LIBRARIES}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
  REQUIRES_INSTALLATION
  REQUIRES_RIF
)
