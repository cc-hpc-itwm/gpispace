# mirko.rahn@itwm.fraunhofer.de

set (versions
  split_join
  split_join.parallel_split_join
)

set (PUT_PORT "I=10000L")
set (EXPECTED_OUTPUT "333383335000L")

foreach (SPLIT_JOIN_MAIN ${versions})
  fhg_add_application_test (example_${SPLIT_JOIN_MAIN}_we-exec
    SCRIPT test.we-exec
    SCRIPT_OUT test.${SPLIT_JOIN_MAIN}.we-exec
  )

  fhg_add_application_test (example_${SPLIT_JOIN_MAIN}_submit
    SCRIPT test.submit
    SCRIPT_OUT test.${SPLIT_JOIN_MAIN}.submit
    RESOURCE_LOCK starts_drts_environment
  )
endforeach()
