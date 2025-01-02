set (_iml_testing_port_counter 11520
  CACHE INTERNAL
  "Counter for unique --iml-vmem-port arguments."
)

set (_iml_testing_port_per_test 100)

set (_iml_testing_beegfs_tests)

iml_add_library (
  NAME Testing
  NAMESPACE IMLPrivate
  SOURCES "testing/beegfs_directory.cpp"
          "testing/parse_command_line.cpp"
          "testing/random/AllocationHandle.cpp"
          "testing/random/MemoryLocation.cpp"
          "testing/random/MemoryRegion.cpp"
          "testing/random/SegmentHandle.cpp"
          "testing/set_nodefile_from_environment.cpp"
          "testing/virtual_memory_socket_name_for_localhost.cpp"
  INCLUDE_DIRECTORIES
    PUBLIC ${PROJECT_SOURCE_DIR}/test
  LIBRARIES
    PUBLIC IML::Client
    PUBLIC Util::Generic
    PUBLIC Boost::program_options
    PUBLIC Boost::filesystem
)

## Unit tests
iml_add_unit_test (NAME handle_generator
  SOURCES "vmem/gaspi/handle_generator.cpp"
  LIBRARIES IMLPrivate::Server
)

iml_add_unit_test (NAME memory_area
  SOURCES "vmem/gaspi/memory_area.cpp"
  LIBRARIES IMLPrivate::Server
            IMLPrivate::Testing
)

iml_add_unit_test (NAME segment
  SOURCES "vmem/gaspi/segment.cpp"
  LIBRARIES IMLPrivate::Server
)

iml_add_unit_test (NAME api
  SOURCES "Client.cpp"
  LIBRARIES IML::Client
            Util::Generic
)

iml_add_unit_test (NAME api_types_match_real_types
  SOURCES "vmem/gaspi/api_types_match_real_types.cpp"
  LIBRARIES IMLPrivate::Headers
            IML::Client
            GASPI::static
            Util::Generic
)

iml_add_unit_test (NAME gaspi-NetdevID
  SOURCES "gaspi/NetdevID.cpp"
  LIBRARIES IML::Client
            Util::Generic
            GASPI::static
)

iml_add_unit_test (NAME dtmmgr
  SOURCES "vmem/dtmmgr.cpp"
  LIBRARIES IMLPrivate::Server
            IMLPrivate::Testing
)

iml_add_unit_test (NAME tmmgr
  SOURCES "vmem/tmmgr.cpp"
  LIBRARIES IMLPrivate::Server
            IMLPrivate::Testing
)

iml_add_unit_test (NAME perftmmgr
  SOURCES "vmem/perftmmgr.cpp"
  PERFORMANCE_TEST
  LIBRARIES IMLPrivate::Server
            IMLPrivate::Testing
)

iml_add_unit_test (NAME beegfs_area
  SOURCES "vmem/gaspi/beegfs_area.cpp"
  LIBRARIES IMLPrivate::Server
            IMLPrivate::Testing
            Boost::filesystem
  NEEDS_BEEGFS
)

iml_add_unit_test (NAME rif-started_process_promise
  SOURCES "rif/started_process_promise.cpp"
  DESCRIPTION "pipe used for communicating with a iml-rif-started process during startup"
  LIBRARIES IMLPrivate::rif-started_process_promise
            Util::Generic
            PRIVATE fmt
)

## System tests
iml_add_system_test (NAME Rifs
  SOURCES "Rifs.cpp"
  LIBRARIES IML::Client
            IMLPrivate::Testing
            Boost::program_options
  STARTS_RIFD
)

iml_add_system_test (NAME RuntimeSystem
  SOURCES "RuntimeSystem.cpp"
  LIBRARIES IML::Client
            IMLPrivate::Testing
            Boost::program_options
  STARTS_SERVER
  STARTS_RIFD
)

iml_add_system_test (NAME standalone_put_get
  SOURCES "standalone_put_get.cpp"
  LIBRARIES IML::Client
            IMLPrivate::Testing
            Util::Generic
            Boost::program_options
  STARTS_SERVER
  STARTS_RIFD
)

## Only run BeeGFS tests if mountpoint qualifies
iml_add_executable (NAME iml-check_beegfs_segment_requirements
  SOURCES "vmem/gaspi/check_beegfs_segment_requirements.cpp"
  LIBRARIES IMLPrivate::Server
            Util::Generic
            Boost::program_options
)

file (GENERATE OUTPUT "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
  CONTENT "execute_process (COMMAND \"$<TARGET_FILE:iml-check_beegfs_segment_requirements>\"
                                    \"${IML_TESTING_BEEGFS_DIRECTORY}\"
              WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\"
              RESULT_VARIABLE iml_check_beegfs_segment_requirements_result
            )

            if (NOT \${iml_check_beegfs_segment_requirements_result} EQUAL 0)
              list (APPEND CTEST_CUSTOM_TESTS_IGNORE ${_iml_testing_beegfs_tests})
            endif()"
)
