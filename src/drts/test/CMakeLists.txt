
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

set (ISSUE_986_DATA "${CMAKE_CURRENT_SOURCE_DIR}/issue_986.data")
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/worker_description.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/worker_description.cpp"
  @ONLY
)
fhg_add_test (NAME drts_worker_description
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/worker_description.cpp"
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

if (BUILD_TESTING)
  gspc_add_library (NAME module_nounload_detection-empty
    SOURCES "module_nounload_detection/empty.cpp"
    TYPE SHARED
  )
endif()

fhg_add_test (NAME module_nounload_detection
  SOURCES "module_nounload_detection.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Util::Generic
            Boost::filesystem
            Boost::program_options
  COMPILE_DEFINITIONS
    PRIVATE "LIBRARY_TO_LOAD=\"$<TARGET_FILE:module_nounload_detection-empty>\""
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  DEPENDS module_nounload_detection-empty
)

if (_PC_HWLOC_VERSION VERSION_LESS "2.5")
  fhg_add_test (NAME worker_env
    SOURCES "worker_env.cpp"
    USE_BOOST
    ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
        --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
        --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
    LIBRARIES gspc
              test-utilities
              Util::Generic
              Boost::filesystem
              Boost::program_options
    REQUIRES_INSTALLATION
    START_SCOPED_RIF
  )
endif()

add_subdirectory (client_implementation_with_ostream_logger)
add_subdirectory (scoped_rifd_execute)

if (GSPC_WITH_IML)
  fhg_add_test (NAME iml_to_pnet_value_wrappers
    SOURCES "iml_to_pnet_value_wrappers.cpp"
    USE_BOOST
    LIBRARIES gspc
              Util::Generic
              # \todo Is this fine? There is no public API to generate
              # random handles... Remove randomness?
              IMLPrivate::Testing
  )
endif()
