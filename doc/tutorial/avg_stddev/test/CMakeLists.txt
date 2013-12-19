# mirko.rahn@itwm.fraunhofer.de

fhg_add_application_test (tutorial_avg_stddev_we-exec SCRIPT test.we-exec)
fhg_add_application_test (tutorial_avg_stddev_submit SCRIPT test.submit
  RESOURCE_LOCK starts_drts_environment
)
