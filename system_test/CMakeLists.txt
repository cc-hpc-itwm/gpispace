include (ProcessorCount)

ProcessorCount (gspc_system_test_nproc)
if (gspc_system_test_nproc EQUAL 0)
  set (gspc_system_test_nproc 1)
endif()

function (gspc_add_system_test _name)
  set (_test_name "gspc-system-test_${_name}")

  # the notation $ENV\{<var-name>} is necessary to delay the
  # variable evaluation until the system test is run
  add_test (
    NAME ${_test_name}
    COMMAND
      ${CMAKE_COMMAND} -P
        "${CMAKE_CURRENT_SOURCE_DIR}/run_system_test.cmake"
        NAME ${_name}
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${_name}"
        BUILD_DIR "${PROJECT_BINARY_DIR}/system_test/${_name}/build"
        INSTALL_DIR "${PROJECT_BINARY_DIR}/system_test/${_name}/install"
        CMAKE_DEFINITIONS
          -D GPISpace_ROOT=${CMAKE_INSTALL_PREFIX}
        NPROC ${gspc_system_test_nproc}
        ARGS
          "--gspc-home=${CMAKE_INSTALL_PREFIX}"
          "--nodefile=$ENV\{GSPC_NODEFILE_FOR_TESTS}"
          "--rif-strategy=${GSPC_TESTING_RIF_STRATEGY}"
          ${ARGN}
  )

  set_tests_properties (${_test_name}
    PROPERTIES
      FAIL_REGULAR_EXPRESSION "FAILURE.*${_name} :"
  )

  set_property (TEST ${_test_name}
    PROPERTY LABELS "system-test"
  )
endfunction()


gspc_add_system_test (aggregate_sum
    --topology=worker:2
    --N=100
)

gspc_add_system_test (preferences_and_multimodules
    --num-tasks=5
    --num-workers-per-target 5 4 3
)
