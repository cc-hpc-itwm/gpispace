# petry@itwm.fhg.de

configure_file (die.sh.in die.sh @ONLY)
configure_file (dir_build.sh.in dir_build.sh @ONLY)
configure_file (env.sh.in env.sh @ONLY)
configure_file (error.sh.in error.sh @ONLY)
configure_file (measured.sh.in measured.sh @ONLY)
configure_file (measurement.sh.in measurement.sh @ONLY)

macro (fhg_add_application_test)
  parse_arguments (TEST "SCRIPT;ARGS;SCRIPT_OUT;RESOURCE_LOCK" "" ${ARGN})

  car (TEST_NAME ${TEST_DEFAULT_ARGS})

  if (TEST_SCRIPT_OUT)
    set (SCRIPT_OUT ${TEST_SCRIPT_OUT})
  else()
    set (SCRIPT_OUT ${CMAKE_CURRENT_BINARY_DIR}/${TEST_SCRIPT}.sh)
  endif()

    configure_file (
      ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SCRIPT}.sh.in
      ${SCRIPT_OUT}
      @ONLY
    )

    add_test (NAME ${TEST_NAME} COMMAND bash ${SCRIPT_OUT} ${TEST_ARGS})

    set (REQUIRED_FILES
      ${CMAKE_CURRENT_BINARY_DIR}/die.sh
      ${CMAKE_CURRENT_BINARY_DIR}/dir_build.sh
      ${CMAKE_CURRENT_BINARY_DIR}/env.sh
      ${CMAKE_CURRENT_BINARY_DIR}/error.sh
      ${CMAKE_CURRENT_BINARY_DIR}/measured.sh
      ${CMAKE_CURRENT_BINARY_DIR}/measurement.sh
      ${FILES_REQUIRED_IN_INSTALLATION}
    )
    set (LABELS requires_installation)
    set_tests_properties (${TEST_NAME}
      PROPERTIES REQUIRED_FILES "${REQUIRED_FILES}" LABELS "${LABELS}"
    )

    if (TEST_RESOURCE_LOCK)
      set_tests_properties (${TEST_NAME}
        PROPERTIES RESOURCE_LOCK ${TEST_RESOURCE_LOCK}
        )
    endif()
endmacro()

fhg_add_application_test (example_compiler-wrapper_we-exec SCRIPT test.we-exec)
fhg_add_application_test (example_compiler-wrapper_submit SCRIPT test.sdpacc
  RESOURCE_LOCK starts_drts_environment
)
