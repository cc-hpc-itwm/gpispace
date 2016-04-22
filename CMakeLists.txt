cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set (CMAKE_MODULE_PATH
  "${CMAKE_MODULE_PATH}"
  "${CMAKE_SOURCE_DIR}/modules"
  "$ENV{CMAKE_MODULE_PATH}"
)

add_subdirectory (cmake)

include (add_macros)
include (beautify_find_boost)
include (require_compiler_version)

set (PROJECT_VERSION "16.04")
set (PROJECT_CONTACT "gpispace-support@itwm.fraunhofer.de" CACHE STRING "Contact information")
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
else()
  message (FATAL_ERROR "could not discover revision info, please define -DPROJECT_REVISION by hand!")
endif()

set (required_string_option_sentinel_value "required_string_option_sentinel_value-NOT_SET")

SET (SHARED_DIRECTORY_FOR_TESTS ${required_string_option_sentinel_value} CACHE PATH
  "shared directory for tests"
)
if (SHARED_DIRECTORY_FOR_TESTS STREQUAL ${required_string_option_sentinel_value})
  message (FATAL_ERROR "required variable SHARED_DIRECTORY_FOR_TESTS not set")
endif()

SET (TESTING_RIF_STRATEGY ${required_string_option_sentinel_value}
  CACHE STRING "strategy to use for starting scoped rif daemons"
)
if (TESTING_RIF_STRATEGY STREQUAL ${required_string_option_sentinel_value})
  message (FATAL_ERROR "required variable TESTING_RIF_STRATEGY not set")
endif()
SET (TESTING_RIF_STRATEGY_PARAMETERS ""
  CACHE STRING "strategy parameters to use for starting scoped rif daemons"
)

set (Boost_NO_SYSTEM_PATHS ON)

if (NOT FHG_ASSERT_MODE)
  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set (FHG_ASSERT_MODE 0 CACHE STRING "FHG_ASSERT_MODE")
  else()
    set (FHG_ASSERT_MODE 1 CACHE STRING "FHG_ASSERT_MODE")
  endif()
endif()

add_definitions ("-DFHG_ASSERT_MODE=${FHG_ASSERT_MODE}")

set (FHG_QT_MOC_BOOST_HACK_OPTIONS "-DBOOST_LEXICAL_CAST_INCLUDED;-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED;-DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION;-DBOOST_TT_HAS_PLUS_HPP_INCLUDED;-DBOOST_TT_HAS_PLUS_ASSIGN_HPP_INCLUDED;-DBOOST_TT_HAS_MINUS_HPP_INCLUDED;-DBOOST_TT_HAS_MINUS_ASSIGN_HPP_INCLUDED")
set (PNETC_LIB_DIR "share/gspc/xml/lib")

add_definitions ("-DBOOST_NO_CXX11_SCOPED_ENUMS") # required if linking against boost not built with c++11
add_definitions ("-DBOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES")

# Boost.Thread now throws when preconditions are not met.
# This is the case when calling join() in some tests, while
# joinable() == false.  This define is only a hacky workaround and
# the precondition shall be checked everywhere, where needed.
add_definitions (-DBOOST_THREAD_THROW_IF_PRECONDITION_NOT_SATISFIED)

# gpi-space needs boost::mpl::vector with more than $default elements for state machine
add_definitions ("-DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS")
add_definitions ("-DBOOST_MPL_LIMIT_VECTOR_SIZE=40")

require_compiler_version_ge (COMPILER "GNU" VERSION 4.9.2)

find_boost (1.59 REQUIRED QUIET COMPONENTS
  chrono
  date_time
  filesystem
  iostreams
  program_options
  random
  serialization
  signals
  system
  test_exec_monitor
  thread
  timer
  unit_test_framework
)
find_package (GASPI 1.3.0 REQUIRED QUIET)
find_package (HWLOC REQUIRED QUIET)
find_package (LibRT REQUIRED QUIET)
find_package (Qt4 4.8 COMPONENTS QtCore QtGui QtXml QtNetwork REQUIRED QUIET)
find_package (Threads REQUIRED QUIET)

include (${QT_USE_FILE})
include (CTest)
include (CompilerFlags)
include (InstallRequiredSystemLibraries)
include (fhg_test_macros)

macro (fhg_add_runtime_executable)
  extended_add_executable (DONT_APPEND_EXE_SUFFIX INSTALL ${ARGN})
endmacro()

include_directories ("${CMAKE_SOURCE_DIR}/share")
include_directories ("${CMAKE_SOURCE_DIR}/src")
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

if (CMAKE_USE_PTHREADS_INIT)
  link_libraries (${CMAKE_THREAD_LIBS_INIT} rt)
endif()

bundle_boost (DESTINATION "external/boost")

add_subdirectory (external)
add_subdirectory (doc)
add_subdirectory (playground/bl)
add_subdirectory (share)
add_subdirectory (src)
add_subdirectory (test)
