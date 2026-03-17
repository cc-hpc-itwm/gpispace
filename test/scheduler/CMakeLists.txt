gspc_add_library (NAME SchedulerTestNetworkStrategy
  TYPE STATIC
  NAMESPACE GPISpace
  SOURCES "NetworkStrategy.cpp"
  LIBRARIES PRIVATE gspc_scheduler
            PRIVATE fhgcom
)

gspc_add_test (NAME drts_should_handle_multiple_workflows_being_submitted_during_lifetime
  SOURCES drts_should_handle_multiple_workflows_being_submitted_during_lifetime.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/selftest"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            gspc_testing
            gspc_util
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

gspc_add_test (NAME id_generator
  SOURCES id_generator.cpp
  USE_BOOST
  LIBRARIES gspc_scheduler gspc_testing
)
