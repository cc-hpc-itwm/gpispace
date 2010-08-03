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

message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

include(CheckCompiler)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

message(STATUS "CXXFLAGS: ${CMAKE_CXX_FLAGS}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build type: default (not-set)")
else ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
  string(TOUPPER ${CMAKE_BUILD_TYPE} tmp)
  message(STATUS "additional CXXFLAGS: ${CMAKE_CXX_FLAGS_${tmp}}")
endif ("${CMAKE_BUILD_TYPE}" STREQUAL "")

# Reflect the package structure
add_subdirectory(fhg)

# cpack configuration
include(CPack)
