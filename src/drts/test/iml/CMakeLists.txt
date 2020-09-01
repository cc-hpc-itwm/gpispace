fhg_add_test (NAME iml_vmem_allocation
  SOURCES "vmem_allocation.cpp"
  USE_BOOST
  ARGS --iml-home "${CMAKE_INSTALL_PREFIX}"
       --iml-shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --iml-vmem-startup-timeout 60
  LIBRARIES IML::client
            IML::test-utilities
            pnet
            gspc
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  REQUIRES_IML
  START_IML_SCOPED_RIF
)

