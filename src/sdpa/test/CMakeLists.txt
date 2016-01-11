fhg_add_test (drts_should_handle_multiple_workflows_being_submitted_during_lifetime.cpp
  PROJECT sdpa_test
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_SOURCE_DIR}/share/selftest"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc
                 test-utilities
                 fhg-util
                 Boost::program_options
                 Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

fhg_add_test (id_generator
  BOOST_UNIT_TEST
  LINK_LIBRARIES sdpa
)
