# This file is part of GPI-Space.
# Copyright (C) 2022 Fraunhofer ITWM
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

cmake_minimum_required (VERSION 3.15)
project (preferences_and_multimodules
  LANGUAGES CXX
)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

find_package (GPISpace REQUIRED)
find_package (util-generic REQUIRED)
find_package (util-cmake REQUIRED)
find_package (Boost 1.61.0 REQUIRED
  COMPONENTS
     filesystem
     program_options
)

include (util-cmake/add_macros)

set (BUNDLE_ROOT "libexec/bundle")
bundle_GPISpace (DESTINATION "${BUNDLE_ROOT}/gpispace"
  COMPONENTS runtime
)

set (PETRI_NET "${PROJECT_SOURCE_DIR}/workflow/${PROJECT_NAME}.xpnet")
set (COMPILED_PETRI_NET "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pnet")
set (WRAPPER_DIRECTORY "${PROJECT_BINARY_DIR}/gen")
set (WRAPPER_LIBRARIES "${WRAPPER_DIRECTORY}/pnetc/op/lib${PROJECT_NAME}.so")
set (DRIVER_BINARY "${PROJECT_NAME}")

add_custom_command (
  COMMAND
    GPISpace::pnetc
      -I${GPISpace_ROOT}/share/GPISpace/xml
      -I${GPISpace_ROOT}/share/GPISpace/xml/lib
      --gen-cxxflags=-O3
      --gen-cxxflags=-I${CMAKE_SOURCE_DIR}
      --force-overwrite-file=true --input="${PETRI_NET}" --output="${COMPILED_PETRI_NET}"
  OUTPUT "${COMPILED_PETRI_NET}"
  DEPENDS "${PETRI_NET}"
)
add_custom_target (compiled_petri_net
  ALL
  DEPENDS
    "${COMPILED_PETRI_NET}"
)

add_custom_command (
  COMMAND
    GPISpace::pnetc
    -I${GPISpace_ROOT}/share/GPISpace/xml
    -I${GPISpace_ROOT}/share/GPISpace/xml/lib
      --gen-cxxflags=-O3
      --gen-cxxflags=-I${CMAKE_SOURCE_DIR}
      --force-overwrite-file=true
      --input="${PETRI_NET}"
      --output="/dev/null"
      --path-to-cpp="${WRAPPER_DIRECTORY}"
  COMMAND
    +make -C "${WRAPPER_DIRECTORY}"
  OUTPUT "${WRAPPER_LIBRARIES}"
  DEPENDS "${PETRI_NET}"
)
add_custom_target (workflow_library
  ALL
  DEPENDS
    "${WRAPPER_LIBRARIES}"
)

extended_add_executable (NAME "${DRIVER_BINARY}"
  DONT_APPEND_EXE_SUFFIX
  SOURCES
    src/main.cpp
    src/parse_parameters_from_commandline.cpp
    src/execute.cpp
    src/workflow.cpp
    src/WorkflowResult.cpp
    src/ValuesOnPorts.cpp
  INCLUDE_DIRECTORIES PRIVATE include
  LIBRARIES PRIVATE Util::Generic
                    GPISpace::execution
                    Boost::headers
                    Boost::filesystem
                    Boost::program_options
  INSTALL
  INSTALL_DESTINATION bin
)

bundle_GPISpace_add_rpath (TARGET "${DRIVER_BINARY}" INSTALL_DIRECTORY "bin")

install (FILES
  "${COMPILED_PETRI_NET}"
  DESTINATION pnet
)
install (DIRECTORY
  "${WRAPPER_DIRECTORY}/pnetc/op/"
  DESTINATION lib
  FILES_MATCHING PATTERN "*.so"
)
