if (BUILD_TESTING)
  gspc_add_library (NAME we_test_eureka_Fixture
    NAMESPACE GSPC
    TYPE STATIC
    SOURCES "Fixture.cpp"
    LIBRARIES
      PUBLIC GSPC::we_test_eureka_jobserver_provider
      PUBLIC gspc
      PUBLIC test-utilities
      PUBLIC Util::Generic
      PUBLIC Boost::filesystem
      PUBLIC Boost::program_options
  )
  gspc_add_library (NAME we_test_eureka_jobserver_client
    NAMESPACE GSPC
    TYPE STATIC
    SOURCES "jobserver/client.cpp"
    LIBRARIES
      PRIVATE Util::RPC
      PRIVATE Util::Generic
  )
  gspc_add_library (NAME we_test_eureka_jobserver_provider
    TYPE STATIC
    NAMESPACE GSPC
    SOURCES "jobserver/provider.cpp"
    LIBRARIES
      PRIVATE Util::RPC
      PRIVATE Util::Generic
  )
endif()

macro (add_we_eureka_test what)
  fhg_add_test (NAME "we_eureka_${what}"
    SOURCES "${what}.cpp"
    USE_BOOST
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
         --source-directory "${PROJECT_SOURCE_DIR}/src"
         --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
         --jobserver-client "$<TARGET_FILE:GSPC-we_test_eureka_jobserver_client>"
    LIBRARIES PRIVATE GSPC::we_test_eureka_Fixture
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endmacro()

add_we_eureka_test ("eureka_group_can_be_reused_after_eureka")
add_we_eureka_test ("finite_tasks_exit_or_get_cancelled")
add_we_eureka_test ("infinite_tasks_get_cancelled")
add_we_eureka_test ("multi_mod")
add_we_eureka_test ("multiple_eurekas_at_once")
add_we_eureka_test ("multiple_functions_in_the_same_group")
add_we_eureka_test ("multiple_functions_multiple_groups")
add_we_eureka_test ("multiple_groups")
add_we_eureka_test ("pending_tasks_are_removed")
