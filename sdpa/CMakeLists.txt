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
OPTION(ENABLE_TYPE_CHECKING "Enable type checking in tokens" YES)
OPTION(USE_STL_TR1 "Use tr1 from the STL not boost." NO)
OPTION(USE_REAL_WE "Use real workflow engine" YES)

set(STATE_MACHINE_IMPLEMENTATION "MSM" CACHE STRING "Statemachine implementation to use (SMC, MSM, BSC)")

set (STATE_MACHINE_IMPLEMENTATION_SPECIFIED 0)
if ("${STATE_MACHINE_IMPLEMENTATION}" MATCHES "SMC")
  set (USE_SMC_SC ON)
  set (STATE_MACHINE_IMPLEMENTATION_SPECIFIED 1)
endif ("${STATE_MACHINE_IMPLEMENTATION}" MATCHES "SMC")

if ("${STATE_MACHINE_IMPLEMENTATION}" MATCHES "BSC")
  set (USE_BOOST_SC ON)
  set (STATE_MACHINE_IMPLEMENTATION_SPECIFIED 1)
endif ("${STATE_MACHINE_IMPLEMENTATION}" MATCHES "BSC")

if ("${STATE_MACHINE_IMPLEMENTATION}" MATCHES "MSM")
  set (USE_BOOST_MSM ON)
  set (STATE_MACHINE_IMPLEMENTATION_SPECIFIED 1)
endif ("${STATE_MACHINE_IMPLEMENTATION}" MATCHES "MSM")

if (${STATE_MACHINE_IMPLEMENTATION_SPECIFIED} EQUAL 0)
  message(SEND_ERROR "Statemachine implementation not set. Please define STATE_MACHINE_IMPLEMENTATION")
endif (${STATE_MACHINE_IMPLEMENTATION_SPECIFIED} EQUAL 0)

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
    set(HAVE_PTHREAD_H 1)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_USE_PTHREADS_INIT)

find_package(UTIL REQUIRED QUIET)
include_directories(${UTIL_INCLUDE_DIR})

find_package(FhgKernel REQUIRED QUIET)
include_directories(${FhgKernel_INCLUDE_DIR})

# seda library
find_package(SEDA REQUIRED QUIET)
include_directories(${SEDA_INCLUDE_DIR})

# we library
if (USE_REAL_WE)
  find_package(WE REQUIRED QUIET)
  include_directories(${WE_INCLUDE_DIR})
endif (USE_REAL_WE)

## find zlib
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
message(STATUS "ZLib: Inc:${ZLIB_INCLUDE_DIR} Lib:${ZLIB_LIBRARIES}")

# state machine compiler (only when required)
if (USE_SMC_SC)
  include(FindSMC)
  if (SMC_FOUND)
    message(STATUS "Found StateMachineCompiler: ${SMC_JAR}")
    include_directories(${SMC_INCLUDE_DIR})
  else (SMC_FOUND)
    message(STATUS "WARNING: SMC could not be found!")
  endif (SMC_FOUND)
endif (USE_SMC_SC)
include (SMCMacros)

include(SDPAFindBoost)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

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

link_directories(${CMAKE_LIBRARY_PATH})
link_directories(${${PROJECT_NAME}_BINARY_DIR})
link_directories(${Boost_LIBRARY_DIRS})

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
