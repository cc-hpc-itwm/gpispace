# mirko.rahn@itwm.fraunhofer.de

# \todo is a resource lock necessary?
fhg_add_application_test (example_map_log_we_exec
  SCRIPT test.we-exec
  ARGS "${CMAKE_CURRENT_BINARY_DIR}/implementation/log/libmap-impl.so"
       "${CMAKE_CURRENT_SOURCE_DIR}/implementation/log/verify.sh"
       "${CMAKE_CURRENT_SOURCE_DIR}/implementation/log/setup.sh"
)

fhg_add_application_test (example_map_log_submit
  SCRIPT test.submit
  ARGS "${CMAKE_CURRENT_BINARY_DIR}/implementation/log/libmap-impl.so"
       "${CMAKE_CURRENT_SOURCE_DIR}/implementation/log/verify.sh"
       "${CMAKE_CURRENT_SOURCE_DIR}/implementation/log/setup.sh"
  RESOURCE_LOCK starts_drts_environment
)

add_subdirectory (implementation/log)
add_subdirectory (implementation/transform_file)
