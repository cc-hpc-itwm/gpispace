set (_iml_testing_port_counter 11520
  CACHE INTERNAL
  "Counter for unique --iml-vmem-port arguments."
)

set (_iml_testing_port_per_test 100)

set (_iml_testing_beegfs_tests)

## Unit tests
add_unit_test (NAME iml-handle_generator
  USE_BOOST
  SOURCES "vmem/gaspi/handle_generator.cpp"
  LIBRARIES gspc_iml_server
            gspc_testing
)

add_unit_test (NAME iml-memory_area
  USE_BOOST
  SOURCES "vmem/gaspi/memory_area.cpp"
  LIBRARIES gspc_iml_server
            gspc_testing
)

add_unit_test (NAME iml-segment
  USE_BOOST
  SOURCES "vmem/gaspi/segment.cpp"
  LIBRARIES gspc_iml_server
            gspc_testing
)

add_unit_test (NAME iml-api
  USE_BOOST
  SOURCES "Client.cpp"
  LIBRARIES gspc_iml_client
            gspc_util
            gspc_testing
)

add_unit_test (NAME iml-api_types_match_real_types
  USE_BOOST
  SOURCES "vmem/gaspi/api_types_match_real_types.cpp"
  LIBRARIES gspc_iml_client
            GASPI::static
            gspc_util
            gspc_testing
)

add_unit_test (NAME iml-gaspi-NetdevID
  USE_BOOST
  SOURCES "gaspi/NetdevID.cpp"
  LIBRARIES gspc_iml_client
            gspc_util
            GASPI::static
            gspc_testing
)

add_unit_test (NAME iml-dtmmgr
  USE_BOOST
  SOURCES "vmem/dtmmgr.cpp"
  LIBRARIES gspc_iml_server
            gspc_testing
)

add_unit_test (NAME iml-tmmgr
  USE_BOOST
  SOURCES "vmem/tmmgr.cpp"
  LIBRARIES gspc_iml_server
            gspc_testing
)

add_unit_test (NAME iml-perftmmgr
  USE_BOOST
  PERFORMANCE_TEST
  SOURCES "vmem/perftmmgr.cpp"
  LIBRARIES gspc_iml_server
            gspc_testing
)

add_unit_test (NAME iml-beegfs_area
  USE_BOOST
  SOURCES "vmem/gaspi/beegfs_area.cpp"
  LIBRARIES gspc_iml_server
            gspc_testing
  ARGS --beegfs-directory "${GSPC_IML_TESTING_BEEGFS_DIRECTORY}"
)
list (APPEND _iml_testing_beegfs_tests "iml-beegfs_area")

## System tests
list (TRANSFORM iml_files_in_installation
  PREPEND "${CMAKE_INSTALL_PREFIX}/"
  OUTPUT_VARIABLE _iml_required_files
)

add_unit_test (NAME iml-Rifs
  USE_BOOST
  SOURCES "Rifs.cpp"
  LIBRARIES gspc_iml_client
            gspc_testing
            Boost::program_options
  ARGS --iml-rif-strategy ${GSPC_TESTING_RIF_STRATEGY}
  LABELS requires_installation
  REQUIRED_FILES ${_iml_required_files}
)
set_tests_properties (iml-Rifs PROPERTIES ENVIRONMENT
  "IML_TESTING_OVERRIDE_INSTALLATION_PREFIX=${CMAKE_INSTALL_PREFIX}"
)

add_unit_test (NAME iml-RuntimeSystem
  USE_BOOST
  SOURCES "RuntimeSystem.cpp"
  LIBRARIES gspc_iml_client
            gspc_testing
            Boost::program_options
  ARGS --iml-vmem-port ${_iml_testing_port_counter}
       --iml-vmem-startup-timeout 60
       --iml-rif-strategy ${GSPC_TESTING_RIF_STRATEGY}
  LABELS requires_installation requires_vmem
  REQUIRED_FILES ${_iml_required_files}
  RUN_SERIAL
)
math (EXPR _iml_testing_port_counter
  "${_iml_testing_port_counter} + ${_iml_testing_port_per_test}"
)
set (_iml_testing_port_counter ${_iml_testing_port_counter}
  CACHE INTERNAL
  "Counter for unique --iml-vmem-port arguments."
)
set_tests_properties (iml-RuntimeSystem PROPERTIES ENVIRONMENT
  "IML_TESTING_OVERRIDE_INSTALLATION_PREFIX=${CMAKE_INSTALL_PREFIX}"
)

add_unit_test (NAME iml-standalone_put_get
  USE_BOOST
  SOURCES "standalone_put_get.cpp"
  LIBRARIES gspc_iml_client
            gspc_testing
            gspc_util
            Boost::program_options
  ARGS --iml-vmem-port ${_iml_testing_port_counter}
       --iml-vmem-startup-timeout 60
       --iml-rif-strategy ${GSPC_TESTING_RIF_STRATEGY}
  LABELS requires_installation requires_vmem
  REQUIRED_FILES ${_iml_required_files}
  RUN_SERIAL
)
math (EXPR _iml_testing_port_counter
  "${_iml_testing_port_counter} + ${_iml_testing_port_per_test}"
)
set (_iml_testing_port_counter ${_iml_testing_port_counter}
  CACHE INTERNAL
  "Counter for unique --iml-vmem-port arguments."
)
set_tests_properties (iml-standalone_put_get PROPERTIES ENVIRONMENT
  "IML_TESTING_OVERRIDE_INSTALLATION_PREFIX=${CMAKE_INSTALL_PREFIX}"
)

## Only run BeeGFS tests if mountpoint qualifies
extended_add_executable (
  NAME iml-check_beegfs_segment_requirements
  DONT_APPEND_EXE_SUFFIX
  NO_RDYNAMIC
  SOURCES "vmem/gaspi/check_beegfs_segment_requirements.cpp"
  LIBRARIES gspc_iml_server
            gspc_util
            Boost::program_options
)

file (GENERATE OUTPUT "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
  CONTENT "execute_process (COMMAND \"$<TARGET_FILE:iml-check_beegfs_segment_requirements>\"
                                    \"${GSPC_IML_TESTING_BEEGFS_DIRECTORY}\"
              WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\"
              RESULT_VARIABLE iml_check_beegfs_segment_requirements_result
            )

            if (NOT \${iml_check_beegfs_segment_requirements_result} EQUAL 0)
              list (APPEND CTEST_CUSTOM_TESTS_IGNORE ${_iml_testing_beegfs_tests})
            endif()"
)
