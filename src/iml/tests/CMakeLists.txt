fhg_add_test (NAME iml_installation
  SOURCES installation.cpp
  USE_BOOST
  LIBRARIES IML::client
            Boost::program_options
  REQUIRES_INSTALLATION
)

fhg_add_test (NAME iml_installation_from_command_line
  SOURCES installation_from_command_line.cpp
  USE_BOOST
  ARGS --iml-home "${CMAKE_INSTALL_PREFIX}"
  LIBRARIES IML::client
            Boost::program_options
  REQUIRES_INSTALLATION
)

fhg_add_test (NAME iml_scoped_rifd
  SOURCES "scoped_rifd.cpp"
  USE_BOOST
  ARGS --iml-home "${CMAKE_INSTALL_PREFIX}"
       --iml-shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES IML::client
            IML::test-utilities
            Boost::program_options
  REQUIRES_INSTALLATION
  START_IML_SCOPED_RIF
)

fhg_add_test (NAME iml_scoped_drts
  SOURCES "scoped_drts.cpp"
  USE_BOOST
  ARGS --iml-home "${CMAKE_INSTALL_PREFIX}"
       --iml-shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --iml-vmem-startup-timeout 60
  LIBRARIES IML::client
            IML::test-utilities
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_IML
  START_IML_SCOPED_RIF
)

fhg_add_test (NAME iml_standalone_put_get
  SOURCES "standalone_put_get.cpp"
  USE_BOOST
  ARGS --iml-home "${CMAKE_INSTALL_PREFIX}"
       --iml-shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --iml-vmem-startup-timeout 60
  LIBRARIES IML::client
            IML::test-utilities
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_IML
  START_IML_SCOPED_RIF
)
