# petry@itwm.fhg.de

fhg_add_application_test (example_compiler-wrapper_we-exec SCRIPT test.we-exec)
fhg_add_application_test (example_compiler-wrapper_submit SCRIPT test.sdpacc
  RESOURCE_LOCK starts_drts_environment
)
