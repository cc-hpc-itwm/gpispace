# This file is part of GPI-Space.
# Copyright (C) 2020 Fraunhofer ITWM
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
project (stochastic_with_heureka VERSION 1 LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

include (CTest)

add_subdirectory (cmake)
include (add_macros)

include (beautify_find_GPISpace)
find_GPISpace (ALLOW_DIFFERENT_GIT_SUBMODULES)
bundle_GPISpace (DESTINATION "libexec/bundle/gpispace"
  COMPONENTS "runtime" "monitoring"
)

include (beautify_find_boost)
find_boost (1.55 REQUIRED QUIET
  FROM_GPISPACE_INSTALLATION
  COMPONENTS filesystem program_options
)

include_directories ("${CMAKE_SOURCE_DIR}")

install (FILES interface.hpp DESTINATION include/stochastic_with_heureka)

extended_add_library (NAME run-stochastic_with_heureka
  SOURCES "bin/run.cpp"
  LIBRARIES GPISpace::execution
            Boost::program_options
            Boost::filesystem
)

extended_add_executable (NAME run-generic-stochastic_with_heureka
  SOURCES "bin/run-generic.cpp"
  LIBRARIES run-stochastic_with_heureka
  DONT_APPEND_EXE_SUFFIX
  INSTALL
)
bundle_GPISpace_add_rpath (TARGET run-generic-stochastic_with_heureka
  INSTALL_DIRECTORY "bin"
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

add_subdirectory (implementation/asian)
add_subdirectory (implementation/BigData)
add_subdirectory (implementation/miller-rabin)
add_subdirectory (implementation/pcp)
add_subdirectory (implementation/pi)
add_subdirectory (implementation/tasks_with_normal_distributed_duration)
