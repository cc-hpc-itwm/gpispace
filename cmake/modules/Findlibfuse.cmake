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

# hints libfuse pkgconfig dir

# library libfuse::libfuse


set (_pkgname fuse)
set (_libnamespace libfuse)

set (_libname libfuse)

if (TARGET ${_libnamespace}::${_libname})
  return()
endif()

include (detail/pkgconfig_helper)

_pkgconfig_find_library_module (${_pkgname} ${_libnamespace} ${_libname})







_pkgconfig_find_library_module_finalize()

if (NOT ${${CMAKE_FIND_PACKAGE_NAME}_FOUND})
  return()
endif()








if (${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION)
  target_compile_definitions (${_libnamespace}-${_libname} INTERFACE
    FUSE_USE_VERSION=${${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION_MAJOR}${${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION_MINOR}
  )
endif()

# libfuse requires 64bit file offsets but doesn't just check for that
# but requires to explicitly opt-in, even if it is the system default.
#
# Just defining it in the target compile definitions may result in
# some targets having the definition (those that link libfuse) and
# some not having it, which when linked together probably has bad
# results. To avoid that, check that the system uses 64 bits without
# the target-specific definition.
#
# When using a platform that does not use 64 bits by default,
# explicitly opt in *globally* via `-D_FILE_OFFSET_BITS=64`, either in
# the toolchain, user-defined configure-time flags, or hard-coded in
# the application using libfuse, before calling find_package(libfuse).
#
# Note that even with a definition of _FILE_OFFSET_BITS, off_t may
# still be 64-bit, so HAS_64BIT_OFF_T may be true while
# HAS_NO_CONFLICTING_FILE_OFFSET_BITS_DEFINITION is false: Some
# platforms ignore all definitions but 64 and fall back to the
# default, which may also be 64. Technically speaking, this is more
# restrictive than it would have to, but the main objective of these
# checks is sanity of the build environment and assumptions. In
# reality, most platforms/environments will have
#
# - no define, 32bit default -> define _FILE_OFFSET_BITS=64 globally
# - no define, 64bit default -> do nothing
# - define to 64, 32bit default -> do nothing
# - define to 64, 64bit default -> no nothing

check_cxx_source_runs ([[
  #include <climits>
  #include <sys/types.h>
  int main (int, char**)
  {
    return CHAR_BIT * sizeof (off_t) == 64 ? 0 : 1;
  }
  ]]
  HAS_64BIT_OFF_T
)
check_cxx_source_runs ([[
  int main (int, char**)
  {
  #ifndef _FILE_OFFSET_BITS
    return 0;
  #else
    return _FILE_OFFSET_BITS == 64 ? 0 : 1;
  #endif
  }
  ]]
  HAS_NO_CONFLICTING_FILE_OFFSET_BITS_DEFINITION
)

if (NOT HAS_64BIT_OFF_T)
  message (FATAL_ERROR "Platform does not use 64-bit off_t by default. Aborting for safety.
See comment in ${CMAKE_CURRENT_LIST_FILE} for details.")
endif()

if (NOT HAS_NO_CONFLICTING_FILE_OFFSET_BITS_DEFINITION)
  message (FATAL_ERROR "_FILE_OFFSET_BITS is defined, but is not 64. libfuse requires it to be 64.
See comment in ${CMAKE_CURRENT_LIST_FILE} for details.")
endif()

target_compile_definitions (${_libnamespace}-${_libname} INTERFACE
  _FILE_OFFSET_BITS=64
)
