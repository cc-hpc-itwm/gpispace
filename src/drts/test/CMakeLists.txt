
fhg_add_test (installation.cpp
  PROJECT drts
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  LINK_LIBRARIES gspc
                 ${Boost_PROGRAM_OPTIONS_LIBRARIES}
  REQUIRES_INSTALLATION
)
fhg_add_test (installation_from_command_line.cpp
  PROJECT drts
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
  LINK_LIBRARIES gspc
                 ${Boost_PROGRAM_OPTIONS_LIBRARIES}
 REQUIRES_INSTALLATION
)

fhg_add_test (scoped_rifd.cpp
  PROJECT drts
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc
                 test-utilities
                 ${Boost_PROGRAM_OPTIONS_LIBRARIES}
 REQUIRES_INSTALLATION
 START_SCOPED_RIF
)

fhg_add_test (scoped_drts.cpp
  PROJECT drts
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc
                 test-utilities
                 ${Boost_PROGRAM_OPTIONS_LIBRARIES}
 REQUIRES_INSTALLATION
 START_SCOPED_RIF
)