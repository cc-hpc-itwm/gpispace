# -*- mode: cmake; -*-
project(sdpa_main)

include (./version.cmake)

set(PROJECT_REVISION     CACHE STRING "Source code revision")
set(PROJECT_BUILD    "0" CACHE STRING "Build number")
set(PROJECT_DESCRIPTION "Seismic Development and Processing Architecture" CACHE STRING "Project description")
set(PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM" CACHE STRING "Project vendor")
set(PROJECT_COPYRIGHT   "Copyright 2011 Fraunhofer ITWM" CACHE STRING "Copyright")
set(PROJECT_CONTACT     "Alexander Petry <alexander.petry@itwm.fraunhofer.de>" CACHE STRING "Contact information")

OPTION(ENABLE_LOGGING "Build the project with logging enabled" YES)
OPTION(DISABLE_FHGLOG "Disable FHGLog completely" NO)
OPTION(ENABLE_APPLICATION "Build with application" YES)
OPTION(ENABLE_MONITOR_GUI "Build the monitor gui" YES)
OPTION(ENABLE_SDPA_GUI "Build the monitor gui" YES)
OPTION(ENABLE_SDPA_GPI "Build the sdpa-gpi interface" NO)
OPTION(ENABLE_GPI_SPACE "Build the gpi-space interface" YES)
OPTION(ENABLE_STATIC_BINARIES "Build static linked binaries" YES)
OPTION(ENABLE_GPI_FUSE "Build the gpi fuse module" OFF)
OPTION(ENABLE_PNET_EDITOR "Build the pnet editor" YES)
OPTION(DEBUG_FHGLOG "Additional debug output from fhglog self" NO)
set(FHGLOG_STRIP_LEVEL "-1" CACHE STRING "Strip log messages with a level >= N")
set(WITH_COMM OFF)

function(get_revision_info REV_INFO)
  execute_process(
    COMMAND svn info
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
    RESULT_VARIABLE _res
    )
  if (NOT _res)
    string (REGEX MATCH "Revision: [0-9]+" _rev ${_out})
    string (REPLACE "Revision: " "r" _rev ${_rev})
    set (${REV_INFO} ${_rev} PARENT_SCOPE)
  else(_res)
    execute_process(
      COMMAND git describe --always
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE _out_git
      ERROR_VARIABLE _err_git
      RESULT_VARIABLE _res_git
      )
    if (NOT _res_git)
      set (${REV_INFO} ${_out_git} PARENT_SCOPE)
    else (_res_git)
      message(WARNING "Could not discover revision information")
    endif (NOT _res_git)
  endif (NOT _res)
endfunction()

# TODO: WORK HERE this does not seem to work with ccmake, cmake however does the right thing (TM) (I think)
if (PROJECT_REVISION)
else (PROJECT_REVISION)
  get_revision_info (PROJECT_REVISION)
  if (NOT PROJECT_REVISION)
    message (FATAL_ERROR "could not discover revision info, please define -DPROJECT_REVISION by hand!")
  else (NOT PROJECT_REVISION)
    message (STATUS "Revision: ${PROJECT_REVISION}")
    set(PROJECT_REVISION "${PROJECT_REVISION}" CACHE STRING "Source code revision" FORCE)
  endif (NOT PROJECT_REVISION)
endif (PROJECT_REVISION)
message (STATUS "Revision: ${PROJECT_REVISION}")
set(PROJECT_REVISION "${PROJECT_REVISION}" CACHE STRING "Source code revision" FORCE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cpack-dev.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cpack-dev.cmake @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cpack-rel.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cpack-rel.cmake @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packages.list.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/packages.list.cmake @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/modules.sdpa.in    ${CMAKE_CURRENT_BINARY_DIR}/module.sdpa     @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure          ${CMAKE_CURRENT_BINARY_DIR}/configure       @COPYONLY)

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

add_definitions("-DBOOST_FILESYSTEM_VERSION=3")
add_definitions("-DBOOST_FILESYSTEM_NO_DEPRECATED")

# to avoid warnings when using gcc 4.5
add_definitions("-fno-strict-aliasing")

if (DISABLE_FHGLOG)
  add_definitions("-DFHGLOG_DISABLE_LOGGING=1")
else (DISABLE_FHGLOG)
  add_definitions("-DFHGLOG_STRIP_LEVEL=${FHGLOG_STRIP_LEVEL}")
endif (DISABLE_FHGLOG)

##########################################################
set(BUILDING_FROM_MAIN 1)

include(CTest)
add_subdirectory(modules)
message(STATUS "=== [ fhglog ] ===")
add_subdirectory(fhglog)
message(STATUS "=== [ fhgcom ] ===")
add_subdirectory(fhgcom)
message(STATUS "=== [ we ] ===")
add_subdirectory(we)
message(STATUS "=== [ xml ] ===")
add_subdirectory(xml)
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

## optional components
if (ENABLE_MONITOR_GUI)
  message(STATUS "=== [ monitor ] (deprecated) ===")
  add_subdirectory(monitor)
endif (ENABLE_MONITOR_GUI)

if (ENABLE_SDPA_GUI)
  message(STATUS "=== [ sdpa-gui ] ===")
  add_subdirectory(sdpa-gui)
endif (ENABLE_SDPA_GUI)

if (ENABLE_SDPA_GPI)
  message(STATUS "=== [ sdpa-gpi ] ===")
  add_subdirectory(sdpa-gpi)
endif (ENABLE_SDPA_GPI)

if (ENABLE_GPI_SPACE)
  message(STATUS "=== [ gpi-space ] ===")
  add_subdirectory(gpi-space)
endif (ENABLE_GPI_SPACE)

if (ENABLE_GPI_FUSE)
  message(STATUS "=== [ gpi-fuse ] ===")
  add_subdirectory(gpi-fuse)
endif (ENABLE_GPI_FUSE)

if (ENABLE_APPLICATION)
  message(STATUS "=== [ applications ] ===")
  add_subdirectory(application)
endif (ENABLE_APPLICATION)

if (ENABLE_PNET_EDITOR)
  message(STATUS "=== [ pnete ] ===")
  add_subdirectory(pnete)
endif (ENABLE_PNET_EDITOR)

include(TargetDoc.cmake OPTIONAL)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/module.sdpa
  DESTINATION etc/modules/soft/sdpa
  RENAME current
  COMPONENT configurations)

# CPACK packaging
INCLUDE(InstallRequiredSystemLibraries)
#set(CPACK_GENERATOR "RPM;STGZ;TGZ;TBZ2")
set(CPACK_GENERATOR "TGZ;TBZ2")
set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
set(CPACK_PACKAGE_VENDOR "${PROJECT_VENDOR}")
set(CPACK_PACKAGE_VERSION "${PROJECT_REVISION}")
INCLUDE(CPack)
