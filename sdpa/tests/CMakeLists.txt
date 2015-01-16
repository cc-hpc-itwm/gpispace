add_subdirectory(sdpa)
add_subdirectory(bin)

fhg_add_test (drts_should_handle_multiple_workflows_being_submitted_during_lifetime.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_SOURCE_DIR}/share/selftest"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc
                 test-utilities
                 ${UTIL_LIBRARIES}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
  REQUIRES_INSTALLATION
  REQUIRES_RIF
)

fhg_add_test (id_generator
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
)
