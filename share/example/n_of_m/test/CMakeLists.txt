# mirko.rahn@itwm.fraunhofer.de

extended_add_executable (NAME demo SOURCES "demo.cpp")

fhg_add_test (test.cpp
  PROJECT share_example_n_of_m
  BOOST_UNIT_TEST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../"
       --command "$<TARGET_FILE:demo.exe>"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc
                 test-utilities
                 fhg-util
                 Boost::program_options
                 Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)
