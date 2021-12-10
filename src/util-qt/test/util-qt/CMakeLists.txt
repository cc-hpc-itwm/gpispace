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

function (util_qt_unit_test _path)
  _parse_arguments_with_unknown (ARG "" "" "LIBRARIES;SOURCES" "" ${ARGN})
  string (REPLACE "/" "-" _name "util-qt-${_path}")

  add_unit_test (NAME "${_name}"
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${_path}.cpp" ${ARG_SOURCES}
    INCLUDE_DIRECTORIES
      PRIVATE "${PROJECT_SOURCE_DIR}/test"
      PRIVATE "${PROJECT_SOURCE_DIR}/src"
    USE_BOOST
    LIBRARIES Util::Qt ${ARG_LIBRARIES}
    ${ARG_UNPARSED_ARGUMENTS}
  )
endfunction()

###############################################################################
# Test declarations
###############################################################################

util_qt_unit_test (do_after_event_loop)

util_qt_unit_test (qdatastream_enum)

util_qt_unit_test (scoped_signal_block)
