# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (test.cpp
  PROJECT drts_add_worker
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc
                 test-utilities
                 fhg-util
                 Network
                 Boost::program_options
                 Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)