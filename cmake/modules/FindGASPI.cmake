# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

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
