cmake_minimum_required (VERSION 3.13 FATAL_ERROR)
project (gspc VERSION 20.04 LANGUAGES CXX)

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
list (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")
list (APPEND CMAKE_MODULE_PATH "$ENV{CMAKE_MODULE_PATH}")

add_subdirectory (cmake)

include (add_macros)
include (beautify_find_boost)
include (require_compiler_version)
include (git)
include (git_submodules)

set (PROJECT_CONTACT "gpispace-support@itwm.fraunhofer.de" CACHE STRING "Contact information")

determine_git_revision ("${CMAKE_SOURCE_DIR}" PROJECT_REVISION)

install_git_submodules_information ("git.submodules")

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
  system
  test_exec_monitor
  thread
  timer
  unit_test_framework
)
find_package (GASPI 1.3.1 REQUIRED QUIET)
find_package (HWLOC REQUIRED QUIET)
find_package (LibRT REQUIRED QUIET)
find_package (Qt5 COMPONENTS Core Gui Widgets Xml Network REQUIRED QUIET)
find_package (Threads REQUIRED QUIET)

include (CTest)
include (CompilerFlags)
include (InstallRequiredSystemLibraries)
include (fhg_test_macros)

macro (fhg_add_runtime_executable)
  extended_add_executable (DONT_APPEND_EXE_SUFFIX INSTALL ${ARGN})
endmacro()

include_directories ("${CMAKE_SOURCE_DIR}/share")
include_directories ("${CMAKE_SOURCE_DIR}/src")

if (CMAKE_USE_PTHREADS_INIT)
  link_libraries (${CMAKE_THREAD_LIBS_INIT} rt)
endif()

include (CheckCXXSourceRuns)

check_cxx_source_runs ([[
  #include <memory>
  int main (int, char**)
  {
    char c;
    void* ptr (&c);
    std::size_t space (sizeof (c));

    return std::align (1, 0, ptr, space) ? 0 : 1;
    //     ^^^^^^^^^^
  }
  ]]
  HAS_STD_ALIGN
)

bundle_boost (DESTINATION "external/boost")
list (APPEND INSTALL_RPATH_DIRS "lib")

add_subdirectory (external)
add_subdirectory (doc)
add_subdirectory (playground/bl)
add_subdirectory (share)
add_subdirectory (src)
add_subdirectory (test)

add_test (NAME installed_headers_are_selfcontained
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test/installed_headers_are_selfcontained.sh"
          "${CMAKE_INSTALL_PREFIX}"
          "${CMAKE_CXX_COMPILER}"
)
set_property (TEST installed_headers_are_selfcontained
  APPEND PROPERTY LABELS "requires_installation"
)
set_property (TEST installed_headers_are_selfcontained
  APPEND PROPERTY REQUIRED_FILES ${FILES_REQUIRED_IN_INSTALLATION}
)

install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
  DESTINATION "share/gspc/"
)
