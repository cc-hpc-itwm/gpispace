project(libfhglog)

option(DEBUG_FHGLOG "Additional debug output from fhglog self" OFF)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${UTIL_INCLUDE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/fhglog/fhglog-config.hpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fhglog/fhglog-config.hpp
  DESTINATION include/fhglog
  COMPONENT runtime-headers)

if (DEBUG_FHGLOG)
  add_definitions("-DFHGLOG_DEBUG_CONFIG")
else (DEBUG_FHGLOG)
endif (DEBUG_FHGLOG)

add_subdirectory(fhglog)
add_subdirectory(apps)

add_subdirectory(tests)
