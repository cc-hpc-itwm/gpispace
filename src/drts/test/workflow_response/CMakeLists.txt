# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (test.cpp
  PROJECT drts_workflow_response
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --rpc-lib "$<TARGET_FILE:RPC>"
  LINK_LIBRARIES gspc
                 test-utilities
                 fhg-util
                 Boost::program_options
                 Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
