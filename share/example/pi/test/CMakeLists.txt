# mirko.rahn@itwm.fraunhofer.de

fhg_add_application_test (example_pi_we-exec SCRIPT test.we-exec)
fhg_add_application_test (example_pi_submit SCRIPT test.submit
  RESOURCE_LOCK starts_drts_environment
)
