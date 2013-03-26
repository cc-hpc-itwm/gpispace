# -*- mode: cmake; -*-
cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)
cmake_policy(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

project(sdpa)

set(PROJECT_DESCRIPTION "Seismic Data Processing Architecture" CACHE STRING "Project description")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM" CACHE STRING "Project vendor")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM" CACHE STRING "Copyright")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>" CACHE STRING "Contact information")

OPTION(ENABLE_LOGGING "Build the project with logging enabled" YES)
OPTION(USE_STL_TR1 "Use tr1 from the STL not boost." NO)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")
# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/smc)
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

# threads
include(FindThreads)
if(CMAKE_USE_PTHREADS_INIT)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_USE_PTHREADS_INIT)

find_package(UTIL REQUIRED QUIET)
include_directories(${UTIL_INCLUDE_DIR})

find_package(FhgKernel REQUIRED QUIET)
include_directories(${FhgKernel_INCLUDE_DIR})

# seda library
find_package(SEDA REQUIRED QUIET)
include_directories(${SEDA_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/pnet/src)

## find zlib
find_package(ZLIB REQUIRED)
include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
message(STATUS "ZLib: Inc:${ZLIB_INCLUDE_DIR} Lib:${ZLIB_LIBRARIES}")

# state machine compiler (only when required)
if (USE_SMC_SC)
  include(FindSMC)
  if (SMC_FOUND)
    message(STATUS "Found StateMachineCompiler: ${SMC_JAR}")
    include_directories(SYSTEM ${SMC_INCLUDE_DIR})
  else (SMC_FOUND)
    message(STATUS "WARNING: SMC could not be found!")
  endif (SMC_FOUND)
endif (USE_SMC_SC)
include (SMCMacros)

include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

if (ENABLE_LOGGING)
  set(SDPA_ENABLE_LOGGING 1)
else (ENABLE_LOGGING)
  set(SDPA_ENABLE_LOGGING 0)
endif (ENABLE_LOGGING)

# logging
find_package(FhgLog REQUIRED QUIET)
include_directories(${FhgLog_INCLUDE_DIR})
set(SDPA_HAVE_FHGLOG 1)

find_package(FhgCom REQUIRED QUIET)
include_directories(${FhgCom_INCLUDE_DIR})

find_package(MMGR REQUIRED QUIET)
include_directories(${MMGR_INCLUDE_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/sdpa/sdpa-config.hpp)
if (PROJECT_REVISION)
else (PROJECT_REVISION)
   message (WARNING "PROJECT_REVISION is not set!")
endif (PROJECT_REVISION)
add_definitions(-DHAVE_CONFIG_H)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include("${CMAKE_MODULE_PATH}/TargetDoc.cmake" OPTIONAL)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# Reflect the package structure
add_subdirectory(sdpa)
add_subdirectory(apps)
add_subdirectory(plugins)
add_subdirectory(bin)
add_subdirectory(etc)
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)
