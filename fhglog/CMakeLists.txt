project(libfhglog)

include (./version.cmake)

option(ENABLE_REMOTE_LOGGING "Build fhglog with remote logging support" YES)
set(FHGLOG_EXIT_ON_ERROR Yes CACHE BOOL "Exit on error or not")
set(FHGLOG_EXIT_ON_ERROR_CODE "42" CACHE STRING "The exit code with which fhglog will exit on FATAL messages")

option(DEBUG_FHGLOG "Additional debug output from fhglog self" OFF)

# don't forget to add the configuration option inc libfhglogConfig.cmake.in

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif()

find_package(Threads REQUIRED QUIET)
if(CMAKE_USE_PTHREADS_INIT)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif()

find_package(UTIL REQUIRED QUIET)
include_directories(${UTIL_INCLUDE_DIR})

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(syslog.h HAVE_SYSLOG_H)

if (ENABLE_REMOTE_LOGGING)
  set(FHGLOG_WITH_REMOTE_LOGGING 1)
endif (ENABLE_REMOTE_LOGGING)

include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/fhglog/fhglog-config.hpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/fhglog/fhglog-config.hpp
  DESTINATION include/fhglog
  COMPONENT runtime-headers)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FindFhgLog.cmake.in    ${CMAKE_CURRENT_BINARY_DIR}/FindFhgLog.cmake)
add_definitions(-DHAVE_CONFIG_H)

# this file is installed and contains all necessary information about the installed libseda,
# it also loads the file with the exported targets
configure_file(${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY)

link_directories(${CMAKE_LIBRARY_PATH})
link_directories(${${PROJECT_NAME}_BINARY_DIR})

if (DEBUG_FHGLOG)
  add_definitions("-DFHGLOG_DEBUG_CONFIG")
else (DEBUG_FHGLOG)
endif (DEBUG_FHGLOG)

add_subdirectory(fhglog)
add_subdirectory(apps)

add_subdirectory(tests)
