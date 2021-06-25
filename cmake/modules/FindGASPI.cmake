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

# hints gaspi pkgconfig dir

# library GASPI::static


set (_pkgname GPI2)
set (_libnamespace ${CMAKE_FIND_PACKAGE_NAME})
# \todo Fix target name: not guaranteed to be static.
set (_libname static)

if (TARGET ${_libnamespace}::${_libname})
  return()
endif()

#"BEGIN INCLUDE"
include (detail/pkgconfig_helper)
#"END INCLUDE"

_pkgconfig_find_library_module (${_pkgname} ${_libnamespace} ${_libname})







_pkgconfig_find_library_module_finalize (_PC_${CMAKE_FIND_PACKAGE_NAME}_PREFIX)

if (NOT ${${CMAKE_FIND_PACKAGE_NAME}_FOUND})
  return()
endif()






set (${CMAKE_FIND_PACKAGE_NAME}_HOME "${_PC_${CMAKE_FIND_PACKAGE_NAME}_PREFIX}")
