# -*- mode: cmake; -*-
cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(pnete)
string(TOUPPER ${PROJECT_NAME} UPPER_PROJECT_NAME)

set(PROJECT_DESCRIPTION "Petri Net Editor")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2011 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Bernd Lörwald <bernd.loerwald@itwm.fraunhofer.de>")

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

#include(SDPAFindBoost)
#include_directories(${Boost_INCLUDE_DIRS})

## check for fhglog
#find_package(FhgLog REQUIRED)
#include_directories(${FhgLog_INCLUDE_DIR})

## check for we
#find_package(WE REQUIRED)
#include_directories(${WE_INCLUDE_DIR})

## check for util
#find_package(UTIL REQUIRED)
#include_directories(${UTIL_INCLUDE_DIR})

# seda library
#find_package(SEDA REQUIRED)
#include_directories(${SEDA_INCLUDE_DIR})

## check for SDPA headers
#find_package(SDPA REQUIRED)
#include_directories(${SDPA_INCLUDE_DIR})

## check for qt
find_package(Qt4 4.4.3 COMPONENTS QtCore QtGui QtXml REQUIRED)
#find_package(TIFF REQUIRED)
#find_package(JPEG REQUIRED)
#find_package(PNG REQUIRED)
#find_package(MNG REQUIRED)

#find_package(WE REQUIRED)
#include_directories(${WE_INCLUDE_DIR})

add_subdirectory(pnete)

install(DIRECTORY demo DESTINATION share/pnete
  USE_SOURCE_PERMISSIONS
  PATTERN ".svn" EXCLUDE
 )

# CTest
#include(CTest)
#if (BUILD_TESTING)
#  add_subdirectory(tests)
#endif (BUILD_TESTING)
