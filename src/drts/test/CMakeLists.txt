
fhg_add_test (NAME drts_installation
  SOURCES installation.cpp
  USE_BOOST
  LIBRARIES gspc
            Boost::program_options
  REQUIRES_INSTALLATION
)
fhg_add_test (NAME drts_installation_from_command_line
  SOURCES installation_from_command_line.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
  LIBRARIES gspc
            Boost::program_options
  REQUIRES_INSTALLATION
)

fhg_add_test (NAME drts_context
  SOURCES context.cpp
  USE_BOOST
  LIBRARIES gspc
)

fhg_add_test (NAME scoped_rifd
  SOURCES scoped_rifd.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

fhg_add_test (NAME scoped_drts
  SOURCES scoped_drts.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

add_subdirectory (client_implementation_with_ostream_logger)
add_subdirectory (scoped_rifd_execute)

file (COPY ${CMAKE_SOURCE_DIR}/src/fhgcom/tests/certs DESTINATION ${CMAKE_CURRENT_BINARY_DIR})