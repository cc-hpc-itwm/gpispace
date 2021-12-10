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

###############################################################################
# Checks
###############################################################################

if (NOT BUILD_TESTING OR NOT ${_project_entry}_BUILD_TESTING)
  return ()
endif ()

###############################################################################
# Includes
###############################################################################

include (util-cmake/add_unit_test)

###############################################################################
# Definitions
###############################################################################

function (rpc_unit_test _path)
  _parse_arguments_with_unknown (ARG "" "" "LIBRARIES;SOURCES" "" ${ARGN})
  string (REPLACE "/" "-" _name "rpc-${_path}")

  add_unit_test (NAME "${_name}"
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${_path}.cpp" ${ARG_SOURCES}
    INCLUDE_DIRECTORIES
      PRIVATE "${PROJECT_SOURCE_DIR}/test"
      PRIVATE "${PROJECT_SOURCE_DIR}/src"
    USE_BOOST
    LIBRARIES Util::RPC ${ARG_LIBRARIES}
    ${ARG_UNPARSED_ARGUMENTS}
  )
endfunction()

###############################################################################
# Test declarations
###############################################################################

rpc_unit_test (basic)

rpc_unit_test (exceptions)

rpc_unit_test (future)

rpc_unit_test (locked_with_info_file)

rpc_unit_test (many_threads_hammer_one_server RUN_SERIAL)

rpc_unit_test (performance PERFORMANCE_TEST)
set_tests_properties (rpc-performance PROPERTIES TIMEOUT 300)

rpc_unit_test (socket)

rpc_unit_test (socket_operation_preconditions)

rpc_unit_test (templates)
