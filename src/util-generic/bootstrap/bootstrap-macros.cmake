# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

include_guard ()

include (CMakePackageConfigHelpers)
include (CMakeParseArguments)
include (FetchContent)

# create a directory for adding stub config files for bootstrapped packages
set (_bootstrap_dir "${CMAKE_CURRENT_BINARY_DIR}/_bootstrap")
file (MAKE_DIRECTORY "${_boostrap_dir}")

# bootstrapping code
macro (bootstrap_package _pkg_name)

  #############################################################################
  # argument parsing
  #############################################################################

  # create an upper case c-identifier package name
  string (TOUPPER "${_pkg_name}_BOOTSTRAP" _pkg_bootstrap)
  string (MAKE_C_IDENTIFIER "${_pkg_bootstrap}" _pkg_bootstrap)

  # parse arguments
  set (_options)
  set (_one_value_options
    GIT_REPOSITORY
    GIT_TAG
    URL
    URL_HASH_TYPE
    URL_HASH
    VERSION
    VERSION_COMPATIBILITY
  )
  set (_multi_value_options)
  cmake_parse_arguments ("${_pkg_bootstrap}" "${_options}" "${_one_value_options}" "${_multi_value_options}" ${ARGN})

  # default values
  if (NOT ${_pkg_bootstrap}_VERSION)
    set (${_pkg_bootstrap}_VERSION "0")
  endif ()

  if (NOT ${_pkg_bootstrap}_VERSION_COMPATIBILITY)
    set (${_pkg_bootstrap}_VERSION_COMPATIBILITY "AnyNewerVersion")
  endif ()

  # conditionally required options
  if (${_pkg_bootstrap}_GIT_REPOSITORY)
    list (APPEND _required_options
      GIT_TAG
    )
  endif ()

  if (${_pkg_bootstrap}_URL_HASH OR ${_pkg_bootstrap}_URL_HASH_TYPE)
    list (APPEND _required_options
      URL_HASH_TYPE
      URL_HASH
    )
  endif ()

  #############################################################################
  # error handling
  #############################################################################

  # verify required arguments
  foreach (required ${_required_options})
    if (NOT ${_pkg_bootstrap}_${required})
      message (FATAL_ERROR "required argument ${required} missing")
    endif()
  endforeach()

  # don't allow additional arguments
  if (${_pkg_bootstrap}_UNPARSED_ARGUMENTS)
    message (FATAL_ERROR "unknown arguments: ${ARG_UNPARSED_ARGUMENTS}")
  endif()

  # at least one bootstrapping option is required
  if (NOT ${_pkg_bootstrap}_URL AND NOT ${_pkg_bootstrap}_GIT_REPOSITORY)
    message (FATAL_ERROR "${_pkg_name}: At least one boostrapping option is required!")
  endif ()

  # only one bootstrapping option is supported
  if (${_pkg_bootstrap}_URL AND ${_pkg_bootstrap}_GIT_REPOSITORY)
    message (FATAL_ERROR "${_pkg_name}: Only one bootstrapping option is supported!")
  endif ()

  #############################################################################
  # populate package
  #############################################################################

  # fetch the content with the information specified during the bootstrap_package call
  # additional arguments passed to find_package are ignored for bootstrapping
  # if the FETCHCONTENT_SOURCE_DIR_<uc_pkg_name> cache variable is set, the download steps will be skipped.
  # FetchContent_Populate will then use the sources at that location instead.
  if (NOT ${_pkg_name}_FOUND)
    if (${_pkg_bootstrap}_URL)
      if (${_pkg_bootstrap}_URL_HASH)
        FetchContent_Declare (
          "${_pkg_name}"
          URL "${${_pkg_bootstrap}_URL}"
          URL_HASH "${${_pkg_bootstrap}_URL_HASH_TYPE}=${${_pkg_bootstrap}_URL_HASH}"
        )
      else ()
        FetchContent_Declare (
          "${_pkg_name}"
          URL "${${_pkg_bootstrap}_URL}"
        )
      endif ()
    elseif (${_pkg_bootstrap}_GIT_REPOSITORY)
      FetchContent_Declare (
        "${_pkg_name}"
        GIT_REPOSITORY"${${_pkg_bootstrap}_GIT_REPOSITORY}"
        GIT_TAG "${${_pkg_bootstrap}_GIT_TAG}"
      )
    endif ()

    # populate content and make available
    FetchContent_GetProperties ("${_pkg_name}")
    if (NOT ${_pkg_name}_POPULATED)
      FetchContent_Populate ("${_pkg_name}")

      # create a stub package config file
      set (_pkg_prefix_path "${_bootstrap_dir}/${_pkg_name}")
      file (WRITE "${_pkg_prefix_path}/${_pkg_name}-config.cmake" "")

      # create a stub version file
      write_basic_package_version_file (
        "${_pkg_prefix_path}/${_pkg_name}-config-version.cmake"
        VERSION "${${_pkg_bootstrap}_VERSION}"
        COMPATIBILITY "${${_pkg_bootstrap}_VERSION_COMPATIBILITY}"
      )

      # add fake package to prefix path
      set (${_pkg_name}_DIR "${_pkg_prefix_path}")

      add_subdirectory (${${_pkg_name}_SOURCE_DIR} ${${_pkg_name}_BINARY_DIR})
    endif ()
  endif ()
endmacro ()
