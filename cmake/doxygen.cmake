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

include (parse_arguments)
include (install_directory)

macro (_require_unset_and_set _variable _value)
  if (DEFINED ${_variable})
    message (FATAL_ERROR "Tried setting ${_variable} to ${_value} but already"
      "set (to ${${_variable}}). Not proceeding for safety."
    )
  endif()
  set ("${_variable}" ${_value})
endmacro()

# Generates and installs doxygen documentation.
#
# - SOURCES: files and directories to let doxygen crawl in
# - INSTALL_DESTINATION: where to install to
# - optional: MIN_VERSION: doxygen minimum version requirement
# - optional: VERBOSE: do not set DOXYGEN_QUIET
#
# Any unparsed argument is expected to begin with DOXYGEN_<x> and
# introduce a list of values to be set in the Doxyfile for the variable x.
#
# Note: The variables
# - DOXYGEN_CREATE_SUBDIRS = YES
# - DOXYGEN_OUTPUT_DIRECTORY = a unique directory in the build tree
# - if VERBOSE is not given: DOXYGEN_QUIET = YES
# are set by the function automatically and are thus not to be
# specified by the caller. The function will error out if the
# variables are already set, for safety.
#
# Example usage:
#   doxygen (SOURCES "src"
#     MIN_VERSION 1.8.14
#     INSTALL_DESTINATION "doc/doxygen"
#
#     DOXYGEN_GENERATE_HTML YES
#     DOXYGEN_GENERATE_TODOLIST NO
#   )
function (doxygen)
  set (options VERBOSE)
  set (one_value_options MIN_VERSION INSTALL_DESTINATION)
  set (multi_value_options SOURCES)
  set (required_options SOURCES)
  _parse_arguments_with_unknown (_arg "${options}" "${one_value_options}"
    "${multi_value_options}" "${required_options}" ${ARGN}
  )

  cmake_policy (PUSH)
  cmake_policy (SET CMP0057 NEW)
  find_package (Doxygen REQUIRED)
  cmake_policy (POP)
  if (${_arg_MIN_VERSION})
    if (DOXYGEN_VERSION VERSION_LESS _arg_MIN_VERSION)
      message (FATAL_ERROR "Found doxygen, but version ${DOXYGEN_VERSION} "
        "< ${_arg_MIN_VERSION}"
      )
    endif()
  endif()

  set (_target_name "doxygen")

  set (_output_dir "${CMAKE_CURRENT_BINARY_DIR}/${_target_name}")

  # fan out files into directories, in a known-clean directory
  _require_unset_and_set (DOXYGEN_CREATE_SUBDIRS YES)
  _require_unset_and_set (DOXYGEN_OUTPUT_DIRECTORY "${_output_dir}")

  if (NOT _arg_VERBOSE)
    _require_unset_and_set (DOXYGEN_QUIET YES)
  endif()

  # Assume that all unparsed arguments starting with DOXYGEN_
  # introduce values to pass to doxygen.
  # This looks ugly but essentially iterates the list, accumulating
  # all values for a key until it finds the next key or end of list,
  # setting the key-value pairs it finds.
  list (LENGTH _arg_UNPARSED_ARGUMENTS _num_unparsed_args)
  set (_i 0)
  while (_i LESS ${_num_unparsed_args})
    list (GET _arg_UNPARSED_ARGUMENTS ${_i} _key)
    math (EXPR _i "${_i} + 1")

    string (FIND "${_key}" "DOXYGEN_" _prefix_index)
    if (NOT ${_prefix_index} EQUAL 0)
      message (FATAL_ERROR "argument ${_key} is not a DOXYGEN_ option")
    endif()

    set (_values)

    set (_check_for_values TRUE)
    while (_check_for_values AND _i LESS ${_num_unparsed_args})
      list (GET _arg_UNPARSED_ARGUMENTS ${_i} _value)
      string (FIND "${_value}" "DOXYGEN_" _prefix_index)
      if (${_prefix_index} EQUAL 0)
        set (_check_for_values FALSE)
      else()
        math (EXPR _i "${_i} + 1")
        set (_values ${_values} ${_value})
      endif()
    endwhile()

    _require_unset_and_set ("${_key}" "${_values}")
  endwhile()

  doxygen_add_docs (${_target_name} ${_arg_SOURCES})

  if (_arg_INSTALL_DESTINATION)
    # the target is not added to ALL but we want to ensure it being
    # built for installation, so explicitly build it during install.
    install (CODE "execute_process (COMMAND \"${CMAKE_COMMAND}\"
      --build \"${CMAKE_CURRENT_BINARY_DIR}\" --target \"${_target_name}\")"
    )

    install_directory (SOURCE "${_output_dir}"
      DESTINATION "${_arg_INSTALL_DESTINATION}"
    )
  endif()
endfunction()
