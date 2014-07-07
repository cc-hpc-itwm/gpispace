# mirko.rahn@itwm.fraunhofer.de

# \todo is a resource lock necessary?
fhg_add_application_test (example_map_log_we_exec
  SCRIPT test.we-exec
  ARGS "${CMAKE_CURRENT_BINARY_DIR}/implementation/log/libmap-impl.so"
)

fhg_add_application_test (example_map_log_submit
  SCRIPT test.submit
  ARGS "${CMAKE_CURRENT_BINARY_DIR}/implementation/log/libmap-impl.so"
  RESOURCE_LOCK starts_drts_environment
)

add_subdirectory (implementation/log)
