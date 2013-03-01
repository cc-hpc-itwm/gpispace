# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(libfhglog)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

include (./version.cmake)

option(ENABLE_REMOTE_LOGGING "Build fhglog with remote logging support" YES)
option(USE_STL_TR1
 "Use tr1 from stl"
 0)
set(FHGLOG_EXIT_ON_ERROR Yes CACHE BOOL "Exit on error or not")
set(FHGLOG_EXIT_ON_ERROR_CODE "42" CACHE STRING "The exit code with which fhglog will exit on FATAL messages")

option(DEBUG_FHGLOG "Additional debug output from fhglog self" OFF)

# don't forget to add the configuration option inc libfhglogConfig.cmake.in

set(PROJECT_DESCRIPTION "FHG Logging Framework")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2009 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")
set(FHGLOG_VERSION     "${FHGLOG_MAJOR}.${FHGLOG_MINOR}.${FHGLOG_PATCH}")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# threads
message(STATUS "checking for thread package")
find_package(Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_USE_PTHREADS_INIT)

include(CheckIncludeFiles)
# usage: CHECK_INCLUDE_FILES (<header> <RESULT_VARIABLE> )
CHECK_INCLUDE_FILES(syslog.h HAVE_SYSLOG_H)

if (ENABLE_REMOTE_LOGGING)
  set(FHGLOG_WITH_REMOTE_LOGGING 1)
endif (ENABLE_REMOTE_LOGGING)

## check for boost
include(SDPAFindBoost)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})
# on osx, the dylib needs to be linked against boost-thread.

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

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include(TargetDoc OPTIONAL)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

message(STATUS "CXXFLAGS: ${CMAKE_CXX_FLAGS}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build type: default (not-set)")
else ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
  string(TOUPPER ${CMAKE_BUILD_TYPE} tmp)
  message(STATUS "additional CXXFLAGS: ${CMAKE_CXX_FLAGS_${tmp}}")
endif ("${CMAKE_BUILD_TYPE}" STREQUAL "")

if (DEBUG_FHGLOG)
  add_definitions("-DFHGLOG_DEBUG_CONFIG")
else (DEBUG_FHGLOG)
endif (DEBUG_FHGLOG)

#install(EXPORT ${PROJECT_NAME}Exports
#  DESTINATION lib/shared
#  FILE ${PROJECT_NAME}Targets.cmake
#  COMPONENT libraries)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  DESTINATION lib/shared
  COMPONENT libraries)

# Reflect the package structure
add_subdirectory(fhglog)
add_subdirectory(apps)
# CTest
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)
