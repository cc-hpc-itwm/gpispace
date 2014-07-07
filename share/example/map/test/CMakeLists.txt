# mirko.rahn@itwm.fraunhofer.de

# \todo is a resource lock necessary?
fhg_add_application_test (example_map_we_exec SCRIPT test.we-exec)

fhg_add_application_test (example_map_submit SCRIPT test.submit
  RESOURCE_LOCK starts_drts_environment
)

add_subdirectory (implementation/log)
