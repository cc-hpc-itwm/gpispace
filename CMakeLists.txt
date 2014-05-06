cmake_minimum_required (VERSION "2.8.4" FATAL_ERROR)

project (sdpa_main)

set(V_MAJOR 14)
set(V_MINOR 03)
set(V_PATCH )
set(V_EXTRA )

set (PROJECT_VERSION      CACHE STRING "Project version")
set (PROJECT_REVISION     CACHE STRING "Source code revision")
set (PROJECT_BUILD    "0" CACHE STRING "Build number")
set (PROJECT_DESCRIPTION "GPI-Space - productive parallel programming and efficient execution" CACHE STRING "Project description")
set (PROJECT_DESCRIPTION_SUMMARY "GPI-Space - productive parallel programming and efficient execution" CACHE STRING "Project description summary")
set (PROJECT_VENDOR      "Fraunhofer Institute for Industrial Mathematics ITWM" CACHE STRING "Project vendor")
set (PROJECT_COPYRIGHT   "Copyright 2009-2014 Fraunhofer ITWM" CACHE STRING "Copyright")
set (PROJECT_CONTACT     "Developer Team <sdpa-dev@itwm.fraunhofer.de>" CACHE STRING "Contact information")

OPTION (ENABLE_LICENSE_CHECK "enable license check" OFF)
option (FHG_ASSERT_REPLACE_LEGACY "Replace legacy assert with fhg_assert" ON)

  execute_process (
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE _out_git
    ERROR_VARIABLE _err_git
    RESULT_VARIABLE _res_git
    )
  if (${_res_git} EQUAL 0)
    string (STRIP "${_out_git}" _out_git)
    set (PROJECT_REVISION ${_out_git})
  endif()

if (NOT PROJECT_REVISION)
  message (FATAL_ERROR "could not discover revision info, please define -DPROJECT_REVISION by hand!")
endif (NOT PROJECT_REVISION)
set (PROJECT_REVISION "${PROJECT_REVISION}" CACHE STRING "Source code revision" FORCE)

set(__new_vsn "${V_MAJOR}.${V_MINOR}")
if (NOT ${V_PATCH} STREQUAL "")
  set(__new_vsn "${__new_vsn}-${V_PATCH}")
endif()
if (NOT ${V_EXTRA} STREQUAL "")
  set(__new_vsn "${__new_vsn}-${V_EXTRA}")
endif()
set (PROJECT_VERSION "${__new_vsn}" CACHE STRING "Project version" FORCE)
set (__new_vsn)

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

include (fhg_test_macros)

include (CompilerFlags)

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
  add_definitions ("-DFHG_ASSERT_MODE=0")
else()
  add_definitions ("-DFHG_ASSERT_MODE=1")
endif()

if (FHG_ASSERT_REPLACE_LEGACY)
  add_definitions ("-DFHG_ASSERT_REPLACE_LEGACY")
endif()

add_definitions ("-DBOOST_FILESYSTEM_VERSION=3")
add_definitions ("-DBOOST_FILESYSTEM_NO_DEPRECATED")
add_definitions ("-DBOOST_NO_CXX11_SCOPED_ENUMS") # required if linking against boost not built with c++11

find_package (Threads REQUIRED QUIET)
if (CMAKE_USE_PTHREADS_INIT)
  link_libraries (${CMAKE_THREAD_LIBS_INIT} rt)
endif()

# Boost
if (NOT BOOST_ROOT)
  set (BOOST_ROOT $ENV{BOOST_ROOT})
endif()

if (BOOST_ROOT)
  set (Boost_NO_SYSTEM_PATHS ON)
endif()

find_package (Boost 1.55 REQUIRED QUIET COMPONENTS
  thread system filesystem serialization program_options
  regex signals date_time test_exec_monitor unit_test_framework random)

# Boost.Thread now throws when preconditions are not met.
# This is the case when calling join() in some tests, while
# joinable() == false.  This define is only a hacky workaround and
# the precondition shall be checked everywhere, where needed.
add_definitions (-DBOOST_THREAD_THROW_IF_PRECONDITION_NOT_SATISFIED)

get_filename_component (BOOST_ROOT "${Boost_INCLUDE_DIR}" PATH CACHE)

include_directories (SYSTEM ${Boost_INCLUDE_DIRS})
# TODO : Remove again?
link_directories (${Boost_LIBRARY_DIRS})

# sdpa needs boost::mpl::vector with more than $default elements for state machine
add_definitions ("-DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS")
add_definitions ("-DBOOST_MPL_LIMIT_VECTOR_SIZE=40")

set (FHG_QT_MOC_BOOST_HACK_OPTIONS "-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED;-DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION")

find_package(HWLOC REQUIRED QUIET)
find_package (Graphviz COMPONENTS cdt gvc graph pathplan REQUIRED QUIET)

# RPATH configuration
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/:\$ORIGIN/../lib/:\$ORIGIN/../external/boost/lib:\$ORIGIN/../../../lib/")

# replacement for find_package (<one of our subprojects>)
set (UTIL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/util)
set (UTIL_LIBRARIES fhg-revision fhg-util)

set (FhgLog_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/fhglog)
set (FhgLog_LIBRARY fhglog)
set (FhgLog_LIBRARY_SHARED fhglog-shared)

set (MMGR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)
set (MMGR_LIBRARY mmgr)
set (MMGR_LIBRARY_SHARED mmgr.shared)

set (FhgKernel_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)

set (SDPA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/sdpa)

set (FhgCom_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)
set (FhgCom_LIBRARY fhgcom)

set (FVM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/fvm-pc)

set (GPI_SPACE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)
set (GPI_SPACE_LIBRARY gpi-space-pc-client-static gpi-space-pc-segment)

set (PNETC_LIB_DIR "share/sdpa/xml/lib")
set (PNETC_EXAMPLES_DIR "share/sdpa/xml/examples")
set (SDPA_APPS_DIR "libexec/sdpa/apps")

include (CTest)
add_subdirectory (fhglog)
  add_subdirectory (fvm-pc)
add_subdirectory (util)
add_subdirectory (application)
add_subdirectory (src)
add_subdirectory (sdpa)
add_subdirectory(bundle)

add_subdirectory (test)

INCLUDE (InstallRequiredSystemLibraries)

add_subdirectory (playground/ap/licensing)
add_subdirectory (playground/bl)

include (cmake_parse_arguments)
include (car_cdr_macros)

find_package (source_highlite REQUIRED QUIET)
find_package (asciidoc 8.6.7 REQUIRED QUIET)

macro (ADD_ASCIIDOC)
  parse_arguments (DOC
    "INPUT;OUTPUT;BACKEND"
    ""
    ${ARGN}
    )
  car (DOC_TARGET ${DOC_DEFAULT_ARGS})

  if (ASCIIDOC_FOUND AND SOURCE_HIGHLITE_FOUND)
    add_custom_command (
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DOC_OUTPUT}
      COMMAND ${CMAKE_COMMAND}
              -DPATH="${SOURCE_HIGHLITE_BINDIR}:$ENV{PATH}"
              -DASCIIDOC="${ASCIIDOC_EXECUTABLE}"
              -DBACKEND="${DOC_BACKEND}"
              -DOUTPUT="${CMAKE_CURRENT_BINARY_DIR}/${DOC_OUTPUT}"
              -DINPUT="${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT}"
              -P "${CMAKE_SOURCE_DIR}/modules/asciidoc.cmake"
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT}
    )
    add_custom_target (${DOC_TARGET}
      ALL
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DOC_OUTPUT}
    )
  endif()
endmacro()

add_subdirectory (doc)
add_subdirectory (share)
