# mirko.rahn@itwm.fraunhofer.de

include_directories (${CMAKE_SOURCE_DIR}/share/example/map)
include_directories (${FhgLog_INCLUDE_DIR})

add_library (map-impl SHARED
  implementation.cpp
)
