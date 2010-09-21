# -*- mode: cmake; -*-
project(sdpa_main)

include (./version.cmake)

set(PROJECT_DESCRIPTION "Seismic Data Processing Architecture")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>")
set(PROJECT_VERSION     "${V_MAJOR}.${V_MINOR}.${V_PATCH}")
set(PROJECT_README      "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/README")
set(PROJECT_COPYING     "${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/COPYING")

set (GPI_BINARY_DIR  "/opt/cluster/GPI/bin" CACHE STRING "path to the GPI binary directory")
OPTION(ENABLE_LOGGING "Build the project with logging enabled" YES)
OPTION(DISABLE_FHGLOG "Disable FHGLog completely" NO)
OPTION(ENABLE_APPLICATION "Build with application" YES)
OPTION(ENABLE_MONITOR_GUI "Build the monitor gui" YES)
OPTION(ENABLE_SDPA_GPI "Build the sdpa-gpi interface" YES)
OPTION(DEBUG_FHGLOG "Additional debug output from fhglog self" NO)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cpack-dev.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cpack-dev.cmake @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cpack-rel.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cpack-rel.cmake @ONLY)

cmake_minimum_required(VERSION "2.6" FATAL_ERROR)
# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

##########################################################
include(CheckCompiler)
info_compiler()
include (CompilerFlags)

if (DEBUG_FHGLOG)
  message (STATUS "additional messages from fhglog have been enabled")
else (DEBUG_FHGLOG)
  add_definitions("-DNDEBUG_FHGLOG")
endif (DEBUG_FHGLOG)

if (DISABLE_FHGLOG)
  add_definitions("-DFHGLOG_DISABLE_LOGGING=1")
endif (DISABLE_FHGLOG)

##########################################################
include(CTest)
add_subdirectory(modules)
message(STATUS "=== [ fhglog ] ===")
add_subdirectory(fhglog)
message(STATUS "=== [ fhgcom ] ===")
add_subdirectory(fhgcom)
message(STATUS "=== [ xml ] ===")
add_subdirectory(xml)
message(STATUS "=== [ we ] ===")
add_subdirectory(we)
message(STATUS "=== [ fvm-pc ] ===")
add_subdirectory(fvm-pc)
message(STATUS "=== [ mmgr ] ===")
add_subdirectory(mmgr)
message(STATUS "=== [ seda ] ===")
add_subdirectory(seda)
message(STATUS "=== [ sdpa ] ===")
add_subdirectory(sdpa)
message(STATUS "=== [ util ] ===")
add_subdirectory(util)
message(STATUS "=== [ selftest ] ===")
add_subdirectory(selftest)

## dependency fixes
if (WITH_FAKE_PC)
  add_dependencies (fvm-pc-fake mmgr)
  add_dependencies (fvm-pc-fake.shared mmgr.shared)
endif (WITH_FAKE_PC)

## optional components
if (ENABLE_MONITOR_GUI)
  message(STATUS "=== [ monitor ] ===")
  add_subdirectory(monitor)
endif (ENABLE_MONITOR_GUI)

if (ENABLE_SDPA_GPI)
  message(STATUS "=== [ sdpa-gpi ] ===")
  add_subdirectory(sdpa-gpi)
endif (ENABLE_SDPA_GPI)

if (ENABLE_APPLICATION)
  message(STATUS "=== [ applications ] ===")
  add_subdirectory(application)
endif (ENABLE_APPLICATION)

include(TargetDoc.cmake OPTIONAL)
