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

#[============================================================================[
### FindLibssh2

Finds the Libssh2 library.

#### Configuration Variables

- **Libssh2_NO_CMAKE**
  Set this variable to `ON` to skip the CMake find_package call in CONFIG mode.
  Alternatively an environment variable can be set to override the configured
  behavior.

#### Result Variables

- **Libssh2_VERSION**
  Full version string of found package in the format `major.minor.patch.tweak`

- **Libssh2_VERSION_MAJOR**
  Major version number

- **Libssh2_VERSION_MINOR**
  Minor version number

- **Libssh2_VERSION_PATCH**
  Patch version number

- **Libssh2_VERSION_TWEAK**
  Tweak version number

- **Libssh2_VERSION_COUNT**
  Number of set version numbers

#### Cache Variables

- **Libssh2_INCLUDE_DIR**
  Libssh2 include directory

- **Libssh2_LIBRARY**
  Libssh2 library path (e.g. /opt/libssh2/lib/libssh2.so)

#### Imported Targets

- **Libssh2::libssh2**
  Libssh2 CMake library target
#]============================================================================]

###############################################################################
# Includes
###############################################################################

set (_include_dir ${CMAKE_CURRENT_LIST_DIR}/../include/util-cmake)

include (FindPackageHandleStandardArgs)
include (CMakeFindDependencyMacro)

include (${_include_dir}/target_guard.cmake)
include (${_include_dir}/check_version.cmake)

###############################################################################
# Checks and Definitions
###############################################################################

util_cmake_target_guard (Libssh2::libssh2)

if (DEFINED ENV{${CMAKE_FIND_PACKAGE_NAME}_NO_CMAKE})
    set (${CMAKE_FIND_PACKAGE_NAME}_NO_CMAKE
      $ENV{${CMAKE_FIND_PACKAGE_NAME}_NO_CMAKE}
    )
endif()

###############################################################################
# Search
###############################################################################

# config mode
if (NOT ${CMAKE_FIND_PACKAGE_NAME}_NO_CMAKE)
  find_package (${CMAKE_FIND_PACKAGE_NAME} CONFIG QUIET)

  # populate output cache variables, otherwise there won't be an entry in the
  # configuration output
  if (${CMAKE_FIND_PACKAGE_NAME}_FOUND)
    get_target_property (${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR
      Libssh2::libssh2
      INTERFACE_INCLUDE_DIRECTORIES
    )
    get_target_property (${CMAKE_FIND_PACKAGE_NAME}_LIBRARY
      Libssh2::libssh2
      LOCATION
    )
    set (${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR
      ${${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR}
      CACHE PATH "${CMAKE_FIND_PACKAGE_NAME} include directory"
    )
    set (${CMAKE_FIND_PACKAGE_NAME}_LIBRARY
      ${${CMAKE_FIND_PACKAGE_NAME}_LIBRARY}
      CACHE PATH "${CMAKE_FIND_PACKAGE_NAME} library"
    )
  endif()
endif()

# module mode
if (NOT ${CMAKE_FIND_PACKAGE_NAME}_FOUND)
  # locate libssh2 include path
  find_path (${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR
    libssh2.h
  )

  # locate libssh2 library
  find_library (${CMAKE_FIND_PACKAGE_NAME}_LIBRARY
    NAMES ssh2
  )

  if (${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR AND ${CMAKE_FIND_PACKAGE_NAME}_LIBRARY)
    # determine the version number of the found libssh2
    file (STRINGS
      ${${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR}/libssh2.h
      ${CMAKE_FIND_PACKAGE_NAME}_VERSION_MAJOR
      REGEX "^#define[\t ]+LIBSSH2_VERSION_MAJOR[\t ]+"
    )
    string (REGEX
      REPLACE "^#define[\t ]+LIBSSH2_VERSION_MAJOR[\t ]+([0-9]+)" "\\1"
      ${CMAKE_FIND_PACKAGE_NAME}_VERSION_MAJOR
      "${${CMAKE_FIND_PACKAGE_NAME}_VERSION_MAJOR}"
    )

    file (STRINGS
      ${${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR}/libssh2.h
      ${CMAKE_FIND_PACKAGE_NAME}_VERSION_MINOR
      REGEX "^#define[\t ]LIBSSH2_VERSION_MINOR[\t ]"
    )
    string (REGEX
      REPLACE "^#define[\t ]+LIBSSH2_VERSION_MINOR[\t ]+([0-9]+)" "\\1"
      ${CMAKE_FIND_PACKAGE_NAME}_VERSION_MINOR
      "${${CMAKE_FIND_PACKAGE_NAME}_VERSION_MINOR}"
    )

    file (STRINGS
      ${${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR}/libssh2.h
      ${CMAKE_FIND_PACKAGE_NAME}_VERSION_PATCH
      REGEX "^#define[\t ]LIBSSH2_VERSION_PATCH[\t ]"
    )
    string (REGEX
      REPLACE "^#define[\t ]+LIBSSH2_VERSION_PATCH[\t ]+([0-9]+)" "\\1"
      ${CMAKE_FIND_PACKAGE_NAME}_VERSION_PATCH
      "${${CMAKE_FIND_PACKAGE_NAME}_VERSION_PATCH}"
    )

    set (${CMAKE_FIND_PACKAGE_NAME}_VERSION "${${CMAKE_FIND_PACKAGE_NAME}_VERSION_MAJOR}.${${CMAKE_FIND_PACKAGE_NAME}_VERSION_MINOR}.${${CMAKE_FIND_PACKAGE_NAME}_VERSION_PATCH}")
    set (${CMAKE_FIND_PACKAGE_NAME}_VERSION_TWEAK 0)
    set (${CMAKE_FIND_PACKAGE_NAME}_VERSION_COUNT 3)

    # verify version compatibility
    set (_mode WARNING)
    if (${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
      set (_mode FATAL_ERROR)
    endif()

    set (_compatibility SameMajorVersion)
    if (${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION)
      util_cmake_check_version (${_compatibility}
        _is_compatible
        ${${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION}
        ${${CMAKE_FIND_PACKAGE_NAME}_VERSION}
      )
      if (NOT _is_compatible)
        if (NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
          message (${_mode}
            "Could not find a package \"${CMAKE_FIND_PACKAGE_NAME}\" that is ${_compatibility} compatible with requested version \"${${CMAKE_FIND_PACKAGE_NAME}_FIND_VERSION}\". (found: version ${${CMAKE_FIND_PACKAGE_NAME}_VERSION})"
          )
        endif()
        unset (_is_compatible)
        unset (_compatibility)
        return()
      endif()
      unset (_is_compatible)
    endif()
    unset (_compatibility)
    unset (_mode)

    # detect library type
    get_filename_component (_ext
      "${${CMAKE_FIND_PACKAGE_NAME}_LIBRARY}"
      EXT
    )
    if (_ext STREQUAL CMAKE_STATIC_LIBRARY_SUFFIX)
      set (_lib_type "STATIC")
    elseif (_ext STREQUAL CMAKE_SHARED_LIBRARY_SUFFIX)
      set (_lib_type "SHARED")
    else()
      set (_lib_type "UNKNOWN")
    endif()
    unset (_ext)

    # create imported target Libssh2::libssh2
    add_library (Libssh2::libssh2 ${_lib_type} IMPORTED)
    target_include_directories (Libssh2::libssh2
      INTERFACE ${${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR}
    )
    set_target_properties (Libssh2::libssh2 PROPERTIES
      IMPORTED_LOCATION ${${CMAKE_FIND_PACKAGE_NAME}_LIBRARY}
    )

    if (NOT _lib_type STREQUAL SHARED)
      find_dependency (OpenSSL)
      find_dependency (ZLIB)

      target_link_libraries (Libssh2::libssh2
        INTERFACE
          OpenSSL::SSL
          OpenSSL::Crypto
          ZLIB::ZLIB
      )
    endif()

    unset (_lib_type)
  endif()
endif()

###############################################################################
# Handle Standard Arguments
###############################################################################

find_package_handle_standard_args (${CMAKE_FIND_PACKAGE_NAME}
  VERSION_VAR
    ${CMAKE_FIND_PACKAGE_NAME}_VERSION
  REQUIRED_VARS
    ${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR
    ${CMAKE_FIND_PACKAGE_NAME}_LIBRARY
)
mark_as_advanced (
  ${CMAKE_FIND_PACKAGE_NAME}_INCLUDE_DIR
  ${CMAKE_FIND_PACKAGE_NAME}_LIBRARY
)
