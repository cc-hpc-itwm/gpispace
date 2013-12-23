# mirko.rahn@itwm.fraunhofer.de

fhg_add_application_test (example_n_of_m_we-exec SCRIPT test.we-exec)
fhg_add_application_test (example_n_of_m_submit SCRIPT test.submit
  RESOURCE_LOCK starts_drts_environment
)
