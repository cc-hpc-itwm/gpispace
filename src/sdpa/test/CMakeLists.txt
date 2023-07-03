if (BUILD_TESTING)
  gspc_add_library (NAME SDPATestNetworkStrategy
    TYPE STATIC
    NAMESPACE GPISpace
    SOURCES "NetworkStrategy.cpp"
    LIBRARIES PRIVATE fhgcom
  )
endif()

fhg_add_test (NAME drts_should_handle_multiple_workflows_being_submitted_during_lifetime
  SOURCES drts_should_handle_multiple_workflows_being_submitted_during_lifetime.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${PROJECT_SOURCE_DIR}/share/selftest"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

fhg_add_test (NAME id_generator
  SOURCES id_generator.cpp
  USE_BOOST
  LIBRARIES sdpa
)
