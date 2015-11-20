
fhg_add_test (installation.cpp
  PROJECT drts
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  LINK_LIBRARIES gspc
                 Boost::program_options
  REQUIRES_INSTALLATION
)
fhg_add_test (installation_from_command_line.cpp
  PROJECT drts
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
  LINK_LIBRARIES gspc
                 Boost::program_options
 REQUIRES_INSTALLATION
)

fhg_add_test (context.cpp
  PROJECT drts
  BOOST_UNIT_TEST
  LINK_LIBRARIES gspc
)

fhg_add_test (scoped_rifd.cpp
  PROJECT drts
  BOOST_UNIT_TEST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LINK_LIBRARIES gspc
                 test-utilities
                 Boost::program_options
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
                 Boost::program_options
 REQUIRES_INSTALLATION
 START_SCOPED_RIF
)

add_subdirectory (client_implementation_with_ostream_logger)
add_subdirectory (scoped_rifd_execute)
