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

include (parse_arguments)

function (require_compiler_version_ge)
  set (options)
  set (one_value_options COMPILER VERSION)
  set (multi_value_options)
  set (required_options COMPILER VERSION)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  if (CMAKE_CXX_COMPILER_ID STREQUAL "${ARG_COMPILER}")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "${ARG_VERSION}")
      message (FATAL_ERROR "${PROJECT_NAME} requires compiler ${CMAKE_CXX_COMPILER_ID} to be of version >= ${ARG_VERSION} (got ${CMAKE_CXX_COMPILER_VERSION}).")
    endif()
  endif()
endfunction()
