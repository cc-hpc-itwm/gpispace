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
project (stochastic_with_heureka VERSION 1 LANGUAGES CXX)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (SWH_DEPLOYMENT_STRATEGIES
  # The installed application can neither be moved to a different
  # directory nor host. Dependencies are referenced by absolute path.
  "InstallationCanNotBeMovedAndRefersToDependenciesUsingAbsolutePaths"
  # The installed application can freely be moved between directories
  # and hosts. All dependencies are contained within the installation
  # folder. No environment variables are required either. Using this
  # strategy is only possible if all dependencies have this property
  # as well.
  "LocationAndHostIndependentBundle"
)
set (SWH_DEPLOYMENT_STRATEGY "InstallationCanNotBeMovedAndRefersToDependenciesUsingAbsolutePaths"
  CACHE STRING "How to deploy Stochastic-with-Heureka (see SWH_DEPLOYMENT_STRATEGIES)"
)
set_property (CACHE SWH_DEPLOYMENT_STRATEGY PROPERTY
  STRINGS ${SWH_DEPLOYMENT_STRATEGIES}
)
if (NOT SWH_DEPLOYMENT_STRATEGY IN_LIST SWH_DEPLOYMENT_STRATEGIES)
  message (FATAL_ERROR "SWH_DEPLOYMENT_STRATEGY must be one of ${SWH_DEPLOYMENT_STRATEGIES}")
endif()

message (STATUS "Using deployment strategy '${SWH_DEPLOYMENT_STRATEGY}'")
include ("deployment/${SWH_DEPLOYMENT_STRATEGY}.cmake")

include (CTest)

find_package (GPISpace REQUIRED
  COMPONENTS
    # No need to check submodule versions as the SWH source directory
    # is no git repository and it doesn't use copies of submodules
    # shared with GPI-Space.
    DO_NOT_CHECK_GIT_SUBMODULES
)
_swh_deployment_gpispace()

find_package (Boost 1.55 REQUIRED
  COMPONENTS filesystem
             program_options
             unit_test_framework
)

# boost::unit_test_framework, when linked using a shared library,
# needs to be explicitly told to generate `main()` as well. CMake's
# FindBoost does not do that, so detect what type of library it gave
# us and fix up that flag if needed.
if (Boost_UNIT_TEST_FRAMEWORK_LIBRARY MATCHES "[.]so[0-9.]*$")
  target_compile_definitions (Boost::unit_test_framework
    INTERFACE BOOST_TEST_DYN_LINK
  )
endif()

include_directories ("${CMAKE_SOURCE_DIR}")

install (FILES interface.hpp DESTINATION include/stochastic_with_heureka)

# Avoid building modules that don't unload on dlclose().
add_compile_options ($<$<AND:$<COMPILE_LANGUAGE:CXX>,$<STREQUAL:GNU,$<CXX_COMPILER_ID>>>:-fno-gnu-unique>)

# Don't re-export symbols from libraries linked against.
add_link_options (LINKER:--exclude-libs,ALL)
# Only export symbols explicitly marked as exported.
set (CMAKE_PROPERTIES CXX_VISIBILITY_PRESET hidden)
# Don't export inline class member functions.
set (CMAKE_VISIBILITY_INLINES_HIDDEN true)

include (CMakeParseArguments)

function (swh_add_implementation_module)
  cmake_parse_arguments ("_arg" "" "NAME" "SOURCES;LIBRARIES" ${ARGN})
  if (_arg_UNPARSED_ARGUMENTS)
    message (FATAL_ERROR "swh_add_implementation_module: unknown arguments ${_arg_UNPARSED_ARGUMENTS}")
  endif()

  list (APPEND _arg_LIBRARIES PRIVATE GPISpace::workflow_development)

  add_library (${_arg_NAME} MODULE ${_arg_SOURCES})
  target_link_libraries (${_arg_NAME} ${_arg_LIBRARIES})
  install (TARGETS ${_arg_NAME} LIBRARY DESTINATION "implementation")
  _swh_deployment_implementation_module (${_arg_NAME})
endfunction()

function (swh_add_implementation_exe)
  cmake_parse_arguments ("_arg" "" "NAME" "SOURCES;LIBRARIES" ${ARGN})
  if (_arg_UNPARSED_ARGUMENTS)
    message (FATAL_ERROR "swh_add_implementation_exe: unknown arguments ${_arg_UNPARSED_ARGUMENTS}")
  endif()

  list (APPEND _arg_LIBRARIES PRIVATE run-stochastic_with_heureka)

  add_executable (run-${_arg_NAME} ${_arg_SOURCES})
  target_link_libraries (run-${_arg_NAME} ${_arg_LIBRARIES})
  install (TARGETS run-${_arg_NAME} RUNTIME DESTINATION "bin")
  _swh_deployment_implementation_exe (run-${_arg_NAME})
endfunction()

add_library (run-stochastic_with_heureka STATIC "bin/run.cpp")
target_link_libraries (run-stochastic_with_heureka
  PUBLIC GPISpace::execution
  PRIVATE Boost::program_options
  PRIVATE Boost::filesystem
)
target_compile_definitions (run-stochastic_with_heureka
  PRIVATE SWH_DEPLOYMENT_STRATEGY_${SWH_DEPLOYMENT_STRATEGY}
  PRIVATE GSPC_HOME="${GSPC_HOME}"
)

swh_add_implementation_exe (NAME generic-stochastic_with_heureka
  SOURCES "bin/run-generic.cpp"
)

set (workflow_description
  ${CMAKE_CURRENT_SOURCE_DIR}/stochastic_with_heureka.xpnet
)

add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stochastic_with_heureka.pnet
  COMMAND GPISpace::pnetc
          --input=${workflow_description}
          --output=${CMAKE_CURRENT_BINARY_DIR}/stochastic_with_heureka.pnet
  DEPENDS ${workflow_description}
)
add_custom_target (stochastic_with_heureka-pnet
  ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/stochastic_with_heureka.pnet
)

add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gen/pnetc/op/libstochastic_with_heureka.so
  COMMAND GPISpace::pnetc
          --gen-cxxflags=-O3
          --gen-cxxflags=-I${CMAKE_SOURCE_DIR}
          --force-overwrite-file=true
          --input=${workflow_description}
          --output=/dev/null
          --path-to-cpp=${CMAKE_CURRENT_BINARY_DIR}/gen
          && make
          GSPC_HOME=${GSPC_HOME}
          BOOST_ROOT=${BOOST_ROOT}
          -C ${CMAKE_CURRENT_BINARY_DIR}/gen
  DEPENDS ${workflow_description}
)
add_custom_target (stochastic_with_heureka.pnet-lib
  ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/gen/pnetc/op/libstochastic_with_heureka.so
)

install (FILES
  ${CMAKE_CURRENT_BINARY_DIR}/stochastic_with_heureka.pnet
  ${CMAKE_CURRENT_BINARY_DIR}/gen/pnetc/op/libstochastic_with_heureka.so
  DESTINATION "workflow"
)

add_subdirectory (implementation/finance)
add_subdirectory (implementation/miller-rabin)
add_subdirectory (implementation/pcp)
add_subdirectory (implementation/pi)
add_subdirectory (implementation/tasks_with_normal_distributed_duration)
