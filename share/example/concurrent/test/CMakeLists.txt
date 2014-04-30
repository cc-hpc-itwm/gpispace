# mirko.rahn@itwm.fraunhofer.de

fhg_add_application_test (example_concurrent_we-exec SCRIPT test.we-exec)
fhg_add_application_test (example_concurrent_submit SCRIPT test.submit
  RESOURCE_LOCK starts_drts_environment
)
