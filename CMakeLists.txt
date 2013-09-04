project (sdpa_main)

include (./version.cmake)

set (PROJECT_VERSION      CACHE STRING "Project version")
set (PROJECT_REVISION     CACHE STRING "Source code revision")
set (PROJECT_BUILD    "0" CACHE STRING "Build number")
set (PROJECT_DESCRIPTION "GPI-Space - productive parallel programming and efficient execution" CACHE STRING "Project description")
set (PROJECT_DESCRIPTION_SUMMARY "GPI-Space - productive parallel programming and efficient execution" CACHE STRING "Project description summary")
set (PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM" CACHE STRING "Project vendor")
set (PROJECT_COPYRIGHT   "Copyright 2009-2012 Fraunhofer ITWM" CACHE STRING "Copyright")
set (PROJECT_CONTACT     "SDPA Developer Team <sdpa-dev@itwm.fraunhofer.de>" CACHE STRING "Contact information")

OPTION (WITH_MANPAGES "Build manpages" YES)
OPTION (ENABLE_LOGGING "Build the project with logging enabled" YES)
OPTION (DISABLE_FHGLOG "Disable FHGLog completely" NO)
OPTION (ENABLE_APPLICATION "Build with application" YES)
OPTION (ENABLE_FVM_PC "Build the deprecated fvm-pc libraries" ON)
OPTION (ENABLE_SDPA_GUI "Build the monitor gui" YES)
OPTION (ENABLE_GPI_SPACE "Build the gpi-space interface" YES)
OPTION (ENABLE_PNETOPT "Build the 'pnetopt' optimizer tool" YES)
OPTION (ENABLE_PNETV "Build the 'pnetv' verifier tool" YES)
OPTION (ENABLE_STATIC_BINARIES "Build static linked binaries" NO)
OPTION (ENABLE_GPI_FUSE "Build the gpi fuse module" OFF)
OPTION (ENABLE_PNET_EDITOR "Build the pnet editor" YES)
OPTION (INSTALL_AS_BUNDLE "Install as a bundle" NO)
OPTION (DEBUG_FHGLOG "Additional debug output from fhglog self" NO)
OPTION (ENABLE_STATISTICS "enable statistics" NO)
set (FHGLOG_STRIP_LEVEL "-1" CACHE STRING "Strip log messages with a level >= N")
set (FHG_ASSERT_MODE CACHE INTEGER "fhg assert mode: 0 ignore, 1 enable, 2 use assert, 3 throw, 4 just log, 5 log and abort")
option (FHG_ASSERT_REPLACE_LEGACY "Replace legacy assert with fhg_assert" ON)
option (ENABLE_BACKTRACE_ON_PARSE_ERROR "Show a backtrace on (pnet) parser errors" OFF)

function (get_revision_info REV_INFO)
  execute_process (
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE _out_git
    ERROR_VARIABLE _err_git
    RESULT_VARIABLE _res_git
    )
  if (${_res_git} EQUAL 0)
    string (STRIP "${_out_git}" _out_git)
    set (${REV_INFO} ${_out_git} PARENT_SCOPE)
  endif()
endfunction()

function (get_version_info VSN_INFO)
  execute_process (
    COMMAND git describe
    COMMAND sed -e "s/^v//"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
    RESULT_VARIABLE _res
  )
  if (${_res} EQUAL 0)
    string (STRIP "${_out}" _out)
    set (${VSN_INFO} ${_out} PARENT_SCOPE)
  else()
    set(__new_vsn "${V_MAJOR}.${V_MINOR}")
    if (NOT ${V_PATCH} STREQUAL "")
      set(__new_vsn "${__new_vsn}-${V_PATCH}")
    endif()
    if (NOT ${V_EXTRA} STREQUAL "")
      set(__new_vsn "${__new_vsn}-${V_EXTRA}")
    endif()
    set (${VSN_INFO} ${__new_vsn} PARENT_SCOPE)
  endif ()
endfunction()

get_revision_info (PROJECT_REVISION)
if (NOT PROJECT_REVISION)
  message (FATAL_ERROR "could not discover revision info, please define -DPROJECT_REVISION by hand!")
endif (NOT PROJECT_REVISION)
set (PROJECT_REVISION "${PROJECT_REVISION}" CACHE STRING "Source code revision" FORCE)

math (EXPR GSPC_VERSION "${V_MAJOR}*10000 + ${V_MINOR} * 100")

if (V_PATCH)
  math(EXPR GSPC_VERSION "${GSPC_VERSION} + ${V_PATCH}")
endif()

get_version_info (PROJECT_VERSION)
if (NOT PROJECT_VERSION)
  message (FATAL_ERROR "could not discover version info, please define -DPROJECT_VERSION by hand!")
endif()
set (PROJECT_VERSION "${PROJECT_VERSION}" CACHE STRING "Project version" FORCE)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cpack-dev.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cpack-dev.cmake @ONLY)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cpack-rel.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/cpack-rel.cmake @ONLY)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/packages.list.cmake.in
                ${CMAKE_CURRENT_BINARY_DIR}/packages.list.cmake @ONLY)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/modules.sdpa.in    ${CMAKE_CURRENT_BINARY_DIR}/module.sdpa     @ONLY)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/configure          ${CMAKE_CURRENT_BINARY_DIR}/configure       @ONLY)

cmake_minimum_required (VERSION "2.6.7" FATAL_ERROR)
# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

include (fhg_test_macros)

include (CheckCompiler)
include (CompilerFlags)

if (NOT DEBUG_FHGLOG)
  add_definitions("-DNDEBUG_FHGLOG")
endif()

if (NOT FHG_ASSERT_MODE)
  if (${CMAKE_BUILD_TYPE} MATCHES "Release")
    set (FHG_ASSERT_MODE "0")
  else()
    set (FHG_ASSERT_MODE "1")
  endif()
endif()

add_definitions ("-DFHG_ASSERT_MODE=${FHG_ASSERT_MODE}")
if (FHG_ASSERT_REPLACE_LEGACY)
  add_definitions ("-DFHG_ASSERT_REPLACE_LEGACY")
endif()

add_definitions ("-DBOOST_FILESYSTEM_VERSION=3")
add_definitions ("-DBOOST_FILESYSTEM_NO_DEPRECATED")

if (DISABLE_FHGLOG)
  add_definitions ("-DFHGLOG_DISABLE_LOGGING=1")
else()
  add_definitions ("-DFHGLOG_STRIP_LEVEL=${FHGLOG_STRIP_LEVEL}")
endif()

if (ENABLE_STATISTICS)
else()
  add_definitions ("-DNSTATISTICS")
endif()

set (BUILDING_FROM_MAIN 1)

include (FindThreads)
if (CMAKE_USE_PTHREADS_INIT)
  link_libraries (${CMAKE_THREAD_LIBS_INIT} rt)
endif()

# RPATH configuration
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/:\$ORIGIN/../lib/:\$ORIGIN/../../../lib/")

set (FHG_QT_MOC_BOOST_HACK_OPTIONS "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED;-DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION")

include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/json_spirit)

include (CTest)
add_subdirectory (fhglog)
add_subdirectory (fhgcom)
add_subdirectory (pnet)
add_subdirectory (mmgr)
if (ENABLE_FVM_PC)
  add_subdirectory (fvm-pc)
endif()
add_subdirectory (seda)
add_subdirectory (util)
add_subdirectory (fhgkernel)
add_subdirectory (gspcnet)
add_subdirectory (gspcrif)
add_subdirectory (gspcctl)
add_subdirectory (gspcmon)
add_subdirectory (sdpa)
if (ENABLE_GPI_SPACE)
  add_subdirectory (gpi-space)
endif (ENABLE_GPI_SPACE)
if (ENABLE_GPI_FUSE)
  add_subdirectory (gpi-fuse)
endif (ENABLE_GPI_FUSE)
if (ENABLE_APPLICATION)
  add_subdirectory (application)
endif (ENABLE_APPLICATION)
add_subdirectory (editor)

include (TargetDoc)

if (INSTALL_AS_BUNDLE)
  add_subdirectory(bundle)
endif()

add_subdirectory (test)

INCLUDE (InstallRequiredSystemLibraries)
#set (CPACK_GENERATOR "RPM;STGZ;TGZ;TBZ2")
set (CPACK_GENERATOR "TBZ2")
set (CPACK_SOURCE_GENERATOR "TBZ2")
set (CPACK_PACKAGE_VENDOR "${PROJECT_VENDOR}")
set (CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set (CPACK_PACKAGE_VERSION_MAJOR "${V_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${V_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${V_PATCH}")
INCLUDE (CPack)

add_subdirectory (playground/bl)
