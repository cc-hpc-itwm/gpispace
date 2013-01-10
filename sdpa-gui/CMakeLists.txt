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

include(SDPAFindBoost)
include_directories(${Boost_INCLUDE_DIRS})

## check for fhglog
find_package(FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/we)

## check for util
find_package(UTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

# seda library
find_package(SEDA REQUIRED)
include_directories(${SEDA_INCLUDE_DIR})

## check for SDPA headers
find_package(SDPA REQUIRED)
include_directories(${SDPA_INCLUDE_DIR})

find_package(FhgCom REQUIRED QUIET)
include_directories(${FhgCom_INCLUDE_DIR})


# state machine compiler (only when required)
include(FindSMC)
if (SMC_FOUND)
    message(STATUS "Found StateMachineCompiler: ${SMC_JAR}")
    include_directories(${SMC_INCLUDE_DIR})
    include (SMCMacros)
else (SMC_FOUND)
    message(STATUS "WARNING: SMC could not be found!")
endif (SMC_FOUND)

## check for qt
find_package(Qt4 4.4.3 COMPONENTS QtCore QtGui QtXml REQUIRED)
#find_package(TIFF REQUIRED)
#find_package(JPEG REQUIRED)
#find_package(MNG REQUIRED)
#find_package(LCMS REQUIRED)

include_directories("${CMAKE_SOURCE_DIR}/we")

add_subdirectory(sdpa-gui)

# CTest
#include(CTest)
#if (BUILD_TESTING)
#  add_subdirectory(tests)
#endif (BUILD_TESTING)
