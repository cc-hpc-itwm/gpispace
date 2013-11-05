# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(libfhgutil)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

# don't forget to add the configuration option inc libfhglogConfig.cmake.in

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# threads
find_package(Threads REQUIRED QUIET)
if(CMAKE_USE_PTHREADS_INIT)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_USE_PTHREADS_INIT)

include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(FhgLog REQUIRED QUIET)
include_directories(${FhgLog_INCLUDE_DIR})

find_package(UTIL REQUIRED QUIET)
include_directories(${UTIL_INCLUDE_DIR})

find_package(HWLOC REQUIRED QUIET)
include_directories(SYSTEM ${HWLOC_INCLUDE_DIR})

find_package (NACL QUIET)
if (NACL_FOUND)
  if (ENABLE_LICENSE_CHECK)
    include_directories (${NACL_INCLUDE_DIR})
  endif()
else()
  set(ENABLE_LICENSE_CHECK CACHE BOOL OFF FORCE)
endif()

# Reflect the package structure
add_subdirectory(fhg)
add_subdirectory(bin)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)
