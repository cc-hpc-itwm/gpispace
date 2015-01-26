add_subdirectory(sdpa)
add_subdirectory(bin)

configure_file (
  re_register_after_master_connection_lost.sh.in
  re_register_after_master_connection_lost.sh
  @ONLY
)
add_test (NAME re_register_after_master_connection_lost
  COMMAND bash re_register_after_master_connection_lost.sh
)
set_tests_properties (re_register_after_master_connection_lost
  PROPERTIES REQUIRED_FILES "${FILES_REQUIRED_IN_INSTALLATION}"
             LABELS "requires_installation"
             RESOURCE_LOCK starts_drts_environment
)

configure_file (
  properly_find_master_when_started_after_agent.sh.in
  properly_find_master_when_started_after_agent.sh
  @ONLY
)
add_test (NAME properly_find_master_when_started_after_agent
  COMMAND bash properly_find_master_when_started_after_agent.sh
)
set_tests_properties (properly_find_master_when_started_after_agent
  PROPERTIES REQUIRED_FILES "${FILES_REQUIRED_IN_INSTALLATION}"
             LABELS "requires_installation"
             RESOURCE_LOCK starts_drts_environment
)

include_directories (${UTIL_INCLUDE_DIR})
include_directories (${CMAKE_SOURCE_DIR})

fhg_add_test (drts_should_handle_multiple_workflows_being_submitted_during_lifetime.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_SOURCE_DIR}/share/selftest/xml"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc
                 test-utilities
                 ${UTIL_LIBRARIES}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
  RESOURCE_LOCK starts_drts_environment
  REQUIRES_INSTALLATION
)

fhg_add_test (id_generator
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
)
