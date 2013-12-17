project(libfhglog)

set(FHGLOG_EXIT_ON_ERROR Yes CACHE BOOL "Exit on error or not")
set(FHGLOG_EXIT_ON_ERROR_CODE "42" CACHE STRING "The exit code with which fhglog will exit on FATAL messages")

option(DEBUG_FHGLOG "Additional debug output from fhglog self" OFF)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${UTIL_INCLUDE_DIR})

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(syslog.h HAVE_SYSLOG_H)

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
