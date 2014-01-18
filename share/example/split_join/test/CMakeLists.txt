# mirko.rahn@itwm.fraunhofer.de

set (PUT_PORT "I=10000L")
set (EXPECTED_OUTPUT "333383335000L")
set (REENTRANT_VERSION not_reentrant)

foreach (SPLIT_VERSION ${SPLIT_JOIN_SPLIT_VERSIONS})
  set (SPLIT_JOIN_MAIN split_join.${SPLIT_VERSION}.${REENTRANT_VERSION})

  fhg_add_application_test (example_${SPLIT_JOIN_MAIN}_we-exec
    SCRIPT test.we-exec
    SCRIPT_OUT test.${SPLIT_JOIN_MAIN}.we-exec.sh
  )

  fhg_add_application_test (example_${SPLIT_JOIN_MAIN}_submit
    SCRIPT test.submit
    SCRIPT_OUT test.${SPLIT_JOIN_MAIN}.submit.sh
    RESOURCE_LOCK starts_drts_environment
  )
endforeach()

set (PUT_PORT "\"I=10000L\\ I=20000L\"")
set (EXPECTED_OUTPUT "333383335000L\\n2666866670000L")
set (REENTRANT_VERSION reentrant.sequential)

foreach (SPLIT_VERSION ${SPLIT_JOIN_SPLIT_VERSIONS})
  set (SPLIT_JOIN_MAIN split_join.${SPLIT_VERSION}.${REENTRANT_VERSION})

  fhg_add_application_test (example_${SPLIT_JOIN_MAIN}_we-exec
    SCRIPT test.we-exec
    SCRIPT_OUT test.${SPLIT_JOIN_MAIN}.we-exec.sh
  )

  fhg_add_application_test (example_${SPLIT_JOIN_MAIN}_submit
    SCRIPT test.submit
    SCRIPT_OUT test.${SPLIT_JOIN_MAIN}.submit.sh
    RESOURCE_LOCK starts_drts_environment
  )
endforeach()
