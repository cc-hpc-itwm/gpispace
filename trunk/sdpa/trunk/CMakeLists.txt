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

if (NO_IMPLICIT_TEMPLATES)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-implicit-templates")
endif (NO_IMPLICIT_TEMPLATES)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")
# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
message(STATUS "CMake is configured to look in ${CMAKE_MODULE_PATH} for modules")

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# Make sure all subdirectories include correctly
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/smc)
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

include(FindUTIL)
if (UTIL_FOUND)
  include_directories(${UTIL_INCLUDE_DIR})
else(UTIL_FOUND)
  message(FATAL_ERROR "UTIL could not be found but is required")
endif(UTIL_FOUND)

# threads
include(FindThreads)
if(CMAKE_USE_PTHREADS_INIT)
    set(HAVE_PTHREAD_H 1)
    link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif(CMAKE_USE_PTHREADS_INIT)

# seda library
include(FindSEDA)
# check for seda
if (SEDA_FOUND)
  include_directories(${SEDA_INCLUDE_DIR})
  link_directories(${SEDA_LIBRARY_DIR})
else (SEDA_FOUND)
  message(FATAL_ERROR "SEDA library is required!")
endif(SEDA_FOUND)

# uuid library
include(FindUUID)
if (UUID_FOUND)
  include_directories(${UUID_INCLUDE_DIR})
else(UUID_FOUND)
  message(FATAL_ERROR "UUID library could not be found but is required")
endif(UUID_FOUND)

# we library
if (USE_REAL_WE)
	include(FindWE)
	if (WE_FOUND)
	  include_directories(${WE_INCLUDE_DIR})
	else(WE_FOUND)
	  message(FATAL_ERROR "WE could not be found but is required")
	endif(WE_FOUND)
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

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.36" "1.36.0" "1.37" "1.37.0" "1.40" "1.40.0")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system filesystem serialization program_options test_exec_monitor iostreams)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

if (ENABLE_LOGGING)
  set(SDPA_ENABLE_LOGGING 1)
else (ENABLE_LOGGING)
  set(SDPA_ENABLE_LOGGING 0)
endif (ENABLE_LOGGING)

# logging
find_package(FhgLog REQUIRED)
include_directories(${FhgLog_INCLUDE_DIR})
set(SDPA_HAVE_FHGLOG 1)

find_package(FhgCom REQUIRED)
include_directories(${FhgCom_INCLUDE_DIR})

find_package(MMGR REQUIRED)
include_directories(${MMGR_INCLUDE_DIR})

find_package(FVM REQUIRED)
message(STATUS "Found FVM: Inc:${FVM_INCLUDE_DIR} Lib:${FVM_LIBRARY}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in    ${CMAKE_CURRENT_BINARY_DIR}/sdpa/sdpa-config.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in   ${CMAKE_CURRENT_BINARY_DIR}/sdpa/version.hpp)
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
add_subdirectory(bin)
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
endif (BUILD_TESTING)

# add some files to the installation target
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/sdpa/sdpa-config.hpp
  DESTINATION include/sdpa
  COMPONENT headers)
INSTALL(FILES etc/sdpac.rc.example
  DESTINATION etc/sdpa
  COMPONENT configurations)
INSTALL(FILES etc/sdpa.env
  DESTINATION etc/sdpa
  COMPONENT configurations)
