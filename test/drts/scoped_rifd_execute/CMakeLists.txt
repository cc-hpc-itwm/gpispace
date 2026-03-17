# This target is not installed but needs to be treated like one. To
# avoid manually bundling it, ensure that regardless of deployment
# settings it ends up having all paths set.
set (CMAKE_CXX_USE_IMPLICIT_LINK_DIRECTORIES_IN_RUNTIME_PATH true)
gspc_add_executable (NAME test_binary
  SOURCES "test_binary.cpp"
  LIBRARIES gspc_rif_started_process_promise
            gspc_util
            Boost::program_options
)

gspc_add_test (NAME drts_scoped_rifd_execute
  SOURCES scoped_rifd_execute.cpp
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --shared-directory "${GSPC_SHARED_DIRECTORY_FOR_TESTS}"
       --test-binary "$<TARGET_FILE:test_binary.exe>"
  LIBRARIES gspc
            gspc_testing
            Boost::program_options
 REQUIRES_INSTALLATION
 START_SCOPED_RIF
)
