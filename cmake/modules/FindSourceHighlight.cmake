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

# hints source-highlight pkgconfig dir

# library SOURCE_HIGHLIGHT::static
# executable SOURCE_HIGHLIGHT::source_highlight

set (_pkgname source-highlight)
set (_libnamespace SOURCE_HIGHLIGHT)
# \todo Fix target name: not guaranteed to be static.
set (_libname static)

if (TARGET ${_libnamespace}::${_libname})
  return()
endif()

include (detail/pkgconfig_helper)

_pkgconfig_find_library_module (${_pkgname} ${_libnamespace} ${_libname})

find_program (${CMAKE_FIND_PACKAGE_NAME}_BINARY
  NAMES "source-highlight"
  HINTS ${_PC_${CMAKE_FIND_PACKAGE_NAME}_PREFIX}
  PATH_SUFFIXES "bin"
)

_pkgconfig_find_library_module_finalize (${CMAKE_FIND_PACKAGE_NAME}_BINARY)

if (NOT ${${CMAKE_FIND_PACKAGE_NAME}_FOUND})
  return()
endif()

add_imported_executable (NAME source-highlight
  NAMESPACE ${_libnamespace}
  LOCATION "${${CMAKE_FIND_PACKAGE_NAME}_BINARY}"
)
