# mirko.rahn@itwm.fraunhofer.de

fhg_add_application_test (example_map_transform_file_submit
  SCRIPT test.submit
  ARGS "${CMAKE_CURRENT_BINARY_DIR}/transform_file/libmap-impl.so"
       "${CMAKE_CURRENT_SOURCE_DIR}/transform_file/verify.sh"
       "${CMAKE_CURRENT_BINARY_DIR}/transform_file/setup.sh"
  RESOURCE_LOCK starts_drts_environment
)

add_subdirectory (log)
add_subdirectory (transform_file)
