# -*- mode: cmake; -*-
project(sdpa_main)

include (./version.cmake)

set(PROJECT_REVISION CACHE STRING "Source code revision")
set(PROJECT_BUILD    "0" CACHE STRING "Build number")
set(PROJECT_DESCRIPTION "Seismic Data Processing Architecture" CACHE STRING "Project description")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM" CACHE STRING "Project vendor")
set(PROJECT_COPYRIGHT   "Copyright 2010 Fraunhofer ITWM" CACHE STRING "Copyright")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>" CACHE STRING "Contact information")
set(PROJECT_VERSION     "${V_MAJOR}.${V_MINOR}.${V_PATCH}" CACHE STRING "Project version")

set (GPI_BINARY_DIR  "/opt/cluster/GPI/bin" CACHE STRING "path to the GPI binary directory")
set (GPI_CONFIG_PATH "/p/herc/itwm/hpc/soft/sdpa/fvm-pc/etc/fvm.cfg" CACHE STRING "path to the sdpa-gpi config file")
OPTION(ENABLE_LOGGING "Build the project with logging enabled" YES)
OPTION(DISABLE_FHGLOG "Disable FHGLog completely" NO)
OPTION(ENABLE_APPLICATION "Build with application" YES)
OPTION(ENABLE_MONITOR_GUI "Build the monitor gui" YES)
OPTION(ENABLE_SDPA_GPI "Build the sdpa-gpi interface" YES)
OPTION(DEBUG_FHGLOG "Additional debug output from fhglog self" NO)

function(get_foo_revision_info REV_INFO)
  execute_process(
	COMMAND svn info ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE _out
	RESULT_VARIABLE _res
  )
  if (NOT _res)
	string (REGEX MATCH "Revision: [0-9]+" _rev ${_out})
	string (REPLACE "Revision: " "r" _rev ${_rev})
    set (${REV_INFO} ${_rev} PARENT_SCOPE)
  else(_res)
	execute_process(
	  COMMAND git describe ${CMAKE_CURRENT_SOURCE_DIR}
	  OUTPUT_VARIABLE _out_git
	  RESULT_VARIABLE _res_git
	)
	if (NOT _res_git)
	  set (${REV_INFO} ${_out_git} PARENT_SCOPE)
	else (_res_git)
		message(WARNING "Could not discover revision information")
	endif (NOT _res_git)
  endif (NOT _res)
endfunction()

if (PROJECT_REVISION)
else (PROJECT_REVISION)
  get_foo_revision_info (PROJECT_REVISION)
endif (PROJECT_REVISION)
message (STATUS "Revision: ${PROJECT_REVISION}")

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
