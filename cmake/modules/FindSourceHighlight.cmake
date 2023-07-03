# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

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

#"BEGIN INCLUDE"
include (detail/pkgconfig_helper)
#"END INCLUDE"

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
