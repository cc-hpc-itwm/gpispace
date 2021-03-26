# This file is part of GPI-Space.
# Copyright (C) 2021 Fraunhofer ITWM
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required (VERSION 3.13 FATAL_ERROR)
project (gspc VERSION 21.03 LANGUAGES CXX)
set (PROJECT_VERSION "21.03")
set (PROJECT_VERSION_MINOR "03")

# Sane default build type with optimizations enabled.
if (NOT CMAKE_BUILD_TYPE)
  set (_default_build_type "Release")
  message (STATUS "Defaulting to CMAKE_BUILD_TYPE='${_default_build_type}'.")
  set (CMAKE_BUILD_TYPE "${_default_build_type}" CACHE
    STRING "Specifies the build type on single-configuration generators." FORCE
  )
endif()

# Do not deploy with relocatable bundle by default: Users/application
# developers need to be aware of how they deploy, so the default
# should be as plain as possible.
# \note Done before including cmake/add_macros.cmake to overwrite default
# given there.
if (NOT INSTALL_DO_NOT_BUNDLE)
  set (INSTALL_DO_NOT_BUNDLE ON CACHE BOOL "")
endif()

# Where are the additional libraries installed? Note: provide includes
# path here, subsequent checks will resolve everything else
list (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")
list (APPEND CMAKE_MODULE_PATH "$ENV{CMAKE_MODULE_PATH}")

set (BUILD_TESTING OFF CACHE BOOL "")

add_subdirectory (cmake)

include (add_macros)
include (beautify_find_boost)
include (require_compiler_version)

set (PROJECT_CONTACT "gpispace-support@itwm.fraunhofer.de" CACHE STRING "Contact information")

# BEGIN RELEASE v21.03
install (FILES "${CMAKE_SOURCE_DIR}/git.submodules" DESTINATION .)
# END RELEASE

include (CTest)

if (BUILD_TESTING)
  set (required_string_option_sentinel_value "required_string_option_sentinel_value-NOT_SET")

  SET (SHARED_DIRECTORY_FOR_TESTS ${required_string_option_sentinel_value} CACHE PATH
    "shared directory for tests"
  )
  if (SHARED_DIRECTORY_FOR_TESTS STREQUAL ${required_string_option_sentinel_value})
    message (FATAL_ERROR "required variable SHARED_DIRECTORY_FOR_TESTS not set")
  endif()

  # The strategy tests use to start gspc-rifd.
  SET (TESTING_RIF_STRATEGY "ssh"
    CACHE STRING "strategy to use for starting scoped rif daemons"
  )
  # Additional options for the gspc-rifd startup strategy.
  SET (TESTING_RIF_STRATEGY_PARAMETERS ""
    CACHE STRING "strategy parameters to use for starting scoped rif daemons"
  )

  set (IML_TESTING_RIF_STRATEGY ${TESTING_RIF_STRATEGY}
    CACHE STRING
    "The strategy to bootstrap passed to --iml-rif-strategy in tests."
  )
  set (IML_TESTING_RIF_STRATEGY_PARAMETERS ${TESTING_RIF_STRATEGY_PARAMETERS}
    CACHE STRING
    "A semicolon-separated list of parameters to pass to
     --iml-rif-strategy-parameters in tests."
  )
  # Might not be a BeeGFS directory, but IML will check that.
  set (IML_TESTING_BEEGFS_DIRECTORY ${SHARED_DIRECTORY_FOR_TESTS}
    CACHE PATH
    "(Shared) directory to use for BeeGFS tests."
  )
endif()

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

require_compiler_version_ge (COMPILER "GNU" VERSION 4.9.4)

set (_boost_minimum_version 1.61.0)
set (_boost_maximum_version 1.63.0)
find_boost (${_boost_minimum_version} REQUIRED COMPONENTS
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
if (Boost_VERSION VERSION_GREATER ${_boost_maximum_version})
  message (FATAL_ERROR "${PROJECT_NAME} requires Boost to be in version range >= ${_boost_minimum_version} and <= ${_boost_maximum_version} (got ${Boost_VERSION}).")
endif()

find_package (GASPI 1.3.2 REQUIRED)
find_package (HWLOC REQUIRED)
find_package (LibRT REQUIRED)
find_package (Qt5 COMPONENTS Core Gui Widgets Xml Network REQUIRED)
find_package (Threads REQUIRED)
find_package (OpenSSL REQUIRED)

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

find_program (_xmllint_exe NAMES "xmllint")
if (_xmllint_exe)
  add_test (NAME xpnets_are_valid
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/test/xpnets_are_valid.sh"
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${_xmllint_exe}"
  )
  set_property (TEST xpnets_are_valid
    APPEND PROPERTY RUN_SERIAL 1
  )
endif()

install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
  DESTINATION "share/gspc/"
)

file (READ "${CMAKE_SOURCE_DIR}/cmake/git_submodules.cmake"
  ASSERT_GIT_SUBMODULE_VERSIONS_SOURCE
)

set (GPISpace_installconfig_destination "share/GPISpace/cmake")
set (GSPC_HOME ".")

set (GPISpace_installconfig_boost_coroutine_warning_suppression)
if (Boost_VERSION VERSION_GREATER 1.61.0 AND Boost_VERSION VERSION_LESS 1.70.0)
  if (Boost_VERSION VERSION_EQUAL 1.62.0)
    set (GPISpace_installconfig_boost_coroutine_warning_suppression [==[
# With Boost 1.62, Boost.Coroutine is deprecated and should be
# replaced with Boost.Coroutine2. Boost.Asio never did, so suppress
# the warning. In 1.70 they removed the warning again.
target_compile_definitions (Boost::boost INTERFACE
  BOOST_COROUTINES_NO_DEPRECATION_WARNING
  BOOST_COROUTINE_NO_DEPRECATION_WARNING
)]==])
  else()
    set (GPISpace_installconfig_boost_coroutine_warning_suppression [==[
# With Boost 1.62, Boost.Coroutine is deprecated and should be
# replaced with Boost.Coroutine2. Boost.Asio never did, so suppress
# the warning. In 1.70 they removed the warning again.
target_compile_definitions (Boost::boost INTERFACE
  BOOST_COROUTINES_NO_DEPRECATION_WARNING
)]==])
  endif()
endif()

include (CMakePackageConfigHelpers)
configure_package_config_file (GPISpaceConfig.cmake.in GPISpaceConfig.cmake
  INSTALL_DESTINATION "${GPISpace_installconfig_destination}"
  PATH_VARS GSPC_HOME
)
write_basic_package_version_file (GPISpaceConfigVersion.cmake
  COMPATIBILITY ExactVersion
)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/GPISpaceConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/GPISpaceConfigVersion.cmake"
  DESTINATION "${GPISpace_installconfig_destination}"
)
