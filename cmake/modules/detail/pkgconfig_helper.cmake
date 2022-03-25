#! This script is not intended to be used stand-alone but is used by

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

#! find modules that rely on pkg-config to do the actual finding.

#! A find module using this wrapper should
#!
#! - Avoid running if the target already exists to avoid duplicate
#!   work.
#!
#! - Call `_pkgconfig_find_library_module()` with the name used to find
#!   by pkg-config as well as the intended CMake target's namespace and
#!   name.
#!
#! - Optionally search additional requirements that aren't provided by
#!   pkg-config, e.g. tool binaries that come extra with a library.
#!
#! - Call `_pkgconfig_find_library_module_finalize()`, optionally with
#!   a list of additional variables to require being set.
#!
#! - Optionally, if `${${CMAKE_FIND_PACKAGE_NAME}_FOUND}`, provide
#!   additional variables, compile flags or alike, and `return()`
#!   otherwise.
#!
#! Since there are some quirks in how dependencies are resolved, a
#! module author is advised to read implementation comments in this
#! file.

find_package (PkgConfig REQUIRED)

# The macros will search for the given package using pkg-config. If
# not found, an interface target with the given name is created. This
# target is then set up to link pkg-config-given include directories,
# and set up a chain of libraries to link the main library as well as
# its dependencies.
#
# These libraries are using `_resolve_dependency()` to dispatch to one
# of three ways to find that dependency:
#
# - `_use_find_package()`: Prefer to find the dependency using
#   `find_package()` to automatically benefit from dependency search,
#   compiler flags etc a package may know better than `-lpkg` would.
#
# - `_use_imported_libname()`: Tell CMake to just `-lpkg` without an
#   absolute path or `-L`, for system libraries or libraries where the
#   static-over-shared and relocatable preference should be ignored,
#   and the system/compiler be trusted instead.
#
# - `_use_imported_location()`: Try finding `libpkg.a` or `libpkg.so`
#   and link by absolute path. A static library is preferred, but if
#   the package should be relocatable and `libpkg.a` does not contain
#   symbols marked as relocatable it is ignored.
#
# The `_pkgconfig_find_library_module_finalize()` call handles the
# standard `find_package_handle_standard_args()` boilerplate.

macro (_use_find_package _target_name _pkg _pkg_target)
  # When forwarding to a different find_package script, we rely on it
  # already setting up all its dependencies correctly. This means that
  # there is no dependency between the actual target we find here and
  # successors. Hopefully there are no common dependencies with
  # different paths.
  # \note These are always required as it is only used if the package
  # was found and needs a dependency. If the package exists but a
  # dependency does not, that's an unexpected and broken environment.
  # \todo This is dangerous and may lead to linking a shared and
  # static version of such a common dependency at the same time! It is
  # unclear how to solve this without controlling all find_package
  # scripts and also pkg-config scripts.
  find_package (${_pkg} REQUIRED)
  add_library (${_target_name} INTERFACE)
  target_link_libraries (${_target_name} INTERFACE ${_pkg_target})
endmacro()

macro (_use_imported_libname _target_name _name)
  # For system defined libraries, just add the string to the linker
  # command line.
  add_library (${_target_name} INTERFACE IMPORTED)
  set_target_properties (${_target_name} PROPERTIES IMPORTED_LIBNAME "${_name}")
endmacro()

macro (_use_imported_location _target_name _want_relocatable _name _search_path)
  # For all other libraries, prefer static over shared. In the case
  # \a _want_relocatable is given, only use the static version if it
  # is relocatable.
  if (${_want_relocatable})
    find_library (_raw_${_target_name}
      NAMES "lib${_name}.a"
      HINTS ${_search_path}
    )

    set (_relocs "")
    execute_process (COMMAND readelf --relocs "${_raw_${_target_name}}"
      OUTPUT_VARIABLE _relocs
      ERROR_QUIET
    )
    if (NOT _relocs MATCHES "(GOT|PLT|JU?MP_SLOT)")
      unset (_raw_${_target_name} CACHE)
      # \todo instead of directly falling back to .so, remove bad file
      # from search path and try .a again?
      find_library (_raw_${_target_name}
        NAMES "lib${_name}.so"
        HINTS ${_search_path}
      )
    endif()
  else()
    find_library (_raw_${_target_name}
      NAMES "lib${_name}.a" "lib${_name}.so" "${_name}"
      HINTS ${_search_path}
    )
  endif()

  if (NOT _raw_${_target_name})
    set (_relocatable_msg_extra "")
    if (${_want_relocatable})
      set (_relocatable_msg_extra " (relocatable)")
    endif()
    message (FATAL_ERROR "could not find${_relocatable_msg_extra}"
      " lib${_name} in ${_search_path}"
    )
  endif()

  if (TARGET ${_target_name})
    get_property (__previous_definition
      TARGET ${_target_name} PROPERTY IMPORTED_LOCATION
    )
    if (NOT "${__previous_definition}" STREQUAL "${_raw_${_target_name}}")
      message ( FATAL_ERROR "tried finding ${_name} twice with different"
        " results [${__previous_definition} vs ${_raw_${_target_name}}]"
      )
    endif()
  else()
    add_library (${_target_name} UNKNOWN IMPORTED)
    set_property (TARGET ${_target_name}
      PROPERTY IMPORTED_LOCATION "${_raw_${_target_name}}"
    )
  endif()
endmacro()

macro (_resolve_dependency _name _target_name _search_path _want_relocatable)
  if ("${_name}" STREQUAL "pthread")
    _use_find_package (${_target_name} Threads Threads::Threads)

  elseif ("${_name}" STREQUAL "ssl")
    _use_find_package (${_target_name} OpenSSL OpenSSL::SSL)

  elseif ("${_name}" STREQUAL "crypto")
    _use_find_package (${_target_name} OpenSSL OpenSSL::Crypto)

  elseif ("${_name}" STREQUAL "xml2")
    _use_find_package (${_target_name} LibXml2 LibXml2::LibXml2)

  elseif ("${_name}" STREQUAL "z")
    _use_find_package (${_target_name} ZLIB ZLIB::ZLIB)

  elseif ("${_name}" STREQUAL "dl")
    # Not forwarding to ${CMAKE_DL_LIBS}: the pkgconfig library line
    # *has* to contain everything in that string already, otherwise
    # one couldn't link the pkg-config library in the first
    # place. Every content of CMAKE_DL_LIBS has also to be in the
    # pkg-config list.
    _use_imported_libname (${_target_name} "${_name}")

  elseif ("${_name}" STREQUAL "m")
    # Workaround for RHEL7.3 where linking a static libm to a dynamic
    # executable does no longer work, but a dynamic libm needs to be
    # used instead.
    _use_imported_libname (${_target_name} "${_name}")

  elseif ("${_name}" STREQUAL "ibverbs")
    # If libibverbs is linked statically, it expects device drivers to
    # already be linked statically as well, rather than dynamically
    # loading them at runtime. See top/gpispace#707 or
    # eu/intertwine#45
    # \note Should probably explicitly require dynamic rather than
    # system-default, but system-default seems to be dynamic
    # "everywhere".
    _use_imported_libname (${_target_name} "${_name}")

  else()
    # Else, search the path for the library wanted, preferring static
    # over shared, so that we have the full path, not just the name.
    _use_imported_location (${_target_name} ${_want_relocatable} ${_name}
      "${_search_path}"
    )
  endif()
endmacro()

#! Using pkg-config, find \a _pkgname, adding a library
#! `_libnamespace::_libname`. Guaranteed to use the prefix
#! _PC_${CMAKE_FIND_PACKAGE_NAME} for pkgconfig variables, for use by
#! the caller.
macro (_pkgconfig_find_library_module _pkgname _libnamespace _libname)
  set (_pc_search_extra_args)
  if (${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
    list (APPEND _pc_search_extra_args REQUIRED)
  endif()
  if (${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    list (APPEND _pc_search_extra_args QUIET)
  endif()
  pkg_search_module (_PC_${CMAKE_FIND_PACKAGE_NAME} ${_pc_search_extra_args} ${_pkgname})

  if (${_PC_${CMAKE_FIND_PACKAGE_NAME}_FOUND})
    _pkgconfig_find_library_module_handle_success (${_pkgname} ${_libnamespace} ${_libname})
  endif()
endmacro()

macro (_pkgconfig_find_library_module_handle_success _pkgname _libnamespace _libname)
  set (_${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIRS)
  if (_PC_${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIRS)
    set (_${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIRS ${_PC_${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIRS})
  else()
    set (_${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIRS ${_PC_${CMAKE_FIND_PACKAGE_NAME}_INCLUDEDIR})
  endif()

  add_library (${_libnamespace}-${_libname} INTERFACE)
  add_library (${_libnamespace}::${_libname} ALIAS ${_libnamespace}-${_libname})
  target_include_directories (${_libnamespace}-${_libname} SYSTEM INTERFACE
    ${_${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIRS}
  )

  set (_want_relocatable FALSE)
  foreach (component ${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS)
    if (${component} STREQUAL "_relocatable")
      set (_want_relocatable TRUE)
    endif()
  endforeach()

  set (_libs)
  set (_dirs)
  if (_PC_${CMAKE_FIND_PACKAGE_NAME}_STATIC_LIBRARIES)
    set (_libs "${_PC_${CMAKE_FIND_PACKAGE_NAME}_STATIC_LIBRARIES}")
    set (_dirs "${_PC_${CMAKE_FIND_PACKAGE_NAME}_STATIC_LIBRARY_DIRS}")
  else()
    set (_libs "${_PC_${CMAKE_FIND_PACKAGE_NAME}_LIBRARIES}")
    set (_dirs "${_PC_${CMAKE_FIND_PACKAGE_NAME}_LIBDIR}")
  endif()

  # As pkg-config only gives us a linear list of dependencies in "a
  # correct linking order" we can't build a proper tree except for
  # those dependencies we forward to `find_package()`, so instead make
  # the most degenerated-but-correct tree possible:
  #
  #   ssh2
  #     ssh2-dep0-ssh2: import libssh2
  #       ssh2-dep1-ssl:
  #         OpenSSL::SSL: import libssl
  #         ssh2-dep2-crypto:
  #           OpenSSL::Crypto: import libcrypto
  #           ssh2-dep3-dl: import_name dl
  #             ssh2-dep4-zlib:
  #               ZLIB::ZLIB: import libz
  #
  # which is slightly stupid, but should at least be correct and force
  # CMake not to reorder libraries.

  set (_prev_lib ${_libnamespace}-${_libname})
  set (_counter 0)

  set (_${CMAKE_FIND_PACKAGE_NAME}_LIBS)
  foreach (_lib ${_libs})
    set (_dep "")
    set (_target_name "${CMAKE_FIND_PACKAGE_NAME}-dep${_counter}-${_lib}")
    math (EXPR _counter "${_counter} + 1")
    _resolve_dependency ("${_lib}" "${_target_name}" "${_dirs}"
      ${_want_relocatable}
    )

    target_link_libraries (${_prev_lib} INTERFACE ${_target_name})

    unset (_dep)
  endforeach()
endmacro()

#! Finalize a previous `_pkgconfig_find_library_module` call, with \a
#! ARGN being additional variables to require to be set.
macro (_pkgconfig_find_library_module_finalize)
  # FindPkgConfig's `_pkgconfig_unset()` doesn't actually unset but
  # set the value to empty, so the variable is defined and
  # FindPackageHandleStandardArgs in turn thinks it was found and
  # should be printed, leading to a weird looking error message. By
  # actually unsetting the error message no longer complains about a
  # mismatching version "".
  if ("${_PC_${CMAKE_FIND_PACKAGE_NAME}_VERSION}" STREQUAL "")
    unset (_PC_${CMAKE_FIND_PACKAGE_NAME}_VERSION CACHE)
  endif()

  include (FindPackageHandleStandardArgs)
  find_package_handle_standard_args (${CMAKE_FIND_PACKAGE_NAME}
    REQUIRED_VARS _${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIRS
                  ${ARGN}
    VERSION_VAR _PC_${CMAKE_FIND_PACKAGE_NAME}_VERSION
  )
endmacro()
