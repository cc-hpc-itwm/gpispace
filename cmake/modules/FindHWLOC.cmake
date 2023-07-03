# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

# hints hwloc pkgconfig dir

# library HWLOC::static


set (_pkgname hwloc)
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







_pkgconfig_find_library_module_finalize()
