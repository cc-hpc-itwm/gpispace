# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(sdpa-gui)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

set(PROJECT_DESCRIPTION "SDPA Monitor GUI")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")

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

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.37.0" "1.37" "1.36.0" "1.36")
set(Boost_FIND_QUIETLY true)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system serialization program_options)
include_directories(${Boost_INCLUDE_DIRS})

## check for fhglog
find_package(FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

## check for SDPA headers
find_package(SDPA REQUIRED)
include_directories(${SDPA_INCLUDE_DIR})

## check for qt
find_package(Qt4 4.4.3 COMPONENTS QtCore QtGui QtXml REQUIRED)
find_package(TIFF REQUIRED)
find_package(JPEG REQUIRED)
#find_package(PNG REQUIRED)
find_package(MNG REQUIRED)

add_subdirectory(sdpa-gui)

# CTest
#include(CTest)
#if (BUILD_TESTING)
#  add_subdirectory(tests)
#endif (BUILD_TESTING)
