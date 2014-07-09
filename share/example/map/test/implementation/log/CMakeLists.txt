# mirko.rahn@itwm.fraunhofer.de

include_directories (${CMAKE_SOURCE_DIR}/share/example)
include_directories (${FhgLog_INCLUDE_DIR})

add_library (log_map-impl SHARED
  implementation.cpp
)
set_target_properties (log_map-impl PROPERTIES OUTPUT_NAME map-impl)
