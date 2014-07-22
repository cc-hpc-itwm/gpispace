# mirko.rahn@itwm.fraunhofer.de

include_directories (${CMAKE_SOURCE_DIR}/share/example)
include_directories (${FhgLog_INCLUDE_DIR})

add_library (transform_file_type
  type.cpp
)
target_link_libraries (transform_file_type
  we-dev
  ${Boost_FILESYSTEM_LIBRARY}
)

add_library (transform_file_map-impl SHARED
  implementation.cpp
)
target_link_libraries (transform_file_map-impl
  transform_file_type
)
set_target_properties (transform_file_map-impl PROPERTIES OUTPUT_NAME map-impl)

add_executable (gen gen.cpp)
target_link_libraries (gen
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

add_executable (mk_user_parameter mk_user_parameter.cpp)
target_link_libraries (mk_user_parameter
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  transform_file_type
)

fhg_add_application_test (example_map_transform_file_submit
  SCRIPT test.submit
  ARGS "${CMAKE_CURRENT_BINARY_DIR}/libmap-impl.so"
  RESOURCE_LOCK starts_drts_environment
)
