cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
set (CMAKE_MODULE_PATH
  "${CMAKE_MODULE_PATH}"
  "${CMAKE_SOURCE_DIR}/modules"
  "${CMAKE_SOURCE_DIR}/cmake/modules"
  "$ENV{CMAKE_MODULE_PATH}"
)

include ("cmake/parse_arguments.cmake")
include ("cmake/add_macros.cmake")
include ("cmake/beautify_find_boost.cmake")

set (PROJECT_VERSION "14.03")
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

if (NOT BOOST_ROOT)
  set (BOOST_ROOT $ENV{BOOST_ROOT})
endif()

if (BOOST_ROOT)
  set (Boost_NO_SYSTEM_PATHS ON)
endif()

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
  add_definitions ("-DFHG_ASSERT_MODE=0")
else()
  add_definitions ("-DFHG_ASSERT_MODE=1")
endif()

set (FHG_QT_MOC_BOOST_HACK_OPTIONS "-DBOOST_LEXICAL_CAST_INCLUDED;-DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED;-DBOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION;-DBOOST_TT_HAS_PLUS_HPP_INCLUDED;-DBOOST_TT_HAS_PLUS_ASSIGN_HPP_INCLUDED;-DBOOST_TT_HAS_MINUS_HPP_INCLUDED;-DBOOST_TT_HAS_MINUS_ASSIGN_HPP_INCLUDED")
set (PNETC_LIB_DIR "share/sdpa/xml/lib")

add_definitions ("-DBOOST_FILESYSTEM_VERSION=3")
add_definitions ("-DBOOST_FILESYSTEM_NO_DEPRECATED")
add_definitions ("-DBOOST_NO_CXX11_SCOPED_ENUMS") # required if linking against boost not built with c++11

# Boost.Thread now throws when preconditions are not met.
# This is the case when calling join() in some tests, while
# joinable() == false.  This define is only a hacky workaround and
# the precondition shall be checked everywhere, where needed.
add_definitions (-DBOOST_THREAD_THROW_IF_PRECONDITION_NOT_SATISFIED)

# sdpa needs boost::mpl::vector with more than $default elements for state machine
add_definitions ("-DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS")
add_definitions ("-DBOOST_MPL_LIMIT_VECTOR_SIZE=40")

find_boost (1.55 REQUIRED QUIET COMPONENTS
  date_time
  filesystem
  iostreams
  program_options
  random
  regex
  serialization
  signals
  system
  test_exec_monitor
  thread
  unit_test_framework
)
find_package (GASPI 1.1.0 REQUIRED QUIET)
find_package (Graphviz COMPONENTS cdt gvc graph pathplan REQUIRED QUIET)
find_package (HWLOC REQUIRED QUIET)
find_package (LibRT REQUIRED QUIET)
find_package (LibReadline REQUIRED QUIET)
find_package (Qt4 4.8 COMPONENTS QtCore QtGui QtXml QtNetwork REQUIRED QUIET)
find_package (Threads REQUIRED QUIET)
find_package (asciidoc 8.6.7 REQUIRED QUIET)
find_package (source_highlite REQUIRED QUIET)

include (${QT_USE_FILE})
include (CTest)
include (CompilerFlags)
include (InstallRequiredSystemLibraries)
include (car_cdr_macros)
include (fhg_test_macros)

macro (fhg_add_runtime_executable)
  extended_add_executable (DONT_APPEND_EXE_SUFFIX INSTALL ${ARGN})
endmacro()

macro (ADD_ASCIIDOC)
  set (options)
  set (one_value_options INPUT OUTPUT BACKEND)
  set (multi_value_options)
  set (required_options INPUT OUTPUT BACKEND)
  parse_arguments_with_unknown (DOC "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})
  set (DOC_TARGET ${DOC_UNPARSED_ARGUMENTS})

  if (ASCIIDOC_FOUND AND SOURCE_HIGHLITE_FOUND)
    add_custom_command (
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DOC_OUTPUT}
      COMMAND ${CMAKE_COMMAND}
              -DPATH="${SOURCE_HIGHLITE_BINDIR}:${GRAPHVIZ_BIN_DIR}:$ENV{PATH}"
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

include_directories ("${CMAKE_SOURCE_DIR}/fhglog")
include_directories ("${CMAKE_SOURCE_DIR}/sdpa")
include_directories ("${CMAKE_SOURCE_DIR}/share")
include_directories ("${CMAKE_SOURCE_DIR}/src")
include_directories ("${CMAKE_SOURCE_DIR}/util")
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

# TODO : Remove again?
link_directories (${Boost_LIBRARY_DIRS})

if (CMAKE_USE_PTHREADS_INIT)
  link_libraries (${CMAKE_THREAD_LIBS_INIT} rt)
endif()

add_subdirectory (bundle)
add_subdirectory (doc)
add_subdirectory (fhglog)
add_subdirectory (playground/bl)
add_subdirectory (sdpa)
add_subdirectory (share)
add_subdirectory (src)
add_subdirectory (test)
add_subdirectory (util)

configure_file ("${CMAKE_SOURCE_DIR}/CTestCustom.cmake" "${CMAKE_BINARY_DIR}/CTestCustom.cmake" COPYONLY)
