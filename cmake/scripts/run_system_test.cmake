#!/bin/env -S cmake -P

# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later


#[============================================================================[
### Description
Script for performing system tests (aka full integration tests) with regular
CMake projects.
The script first parses its command-line arguments before executing the system
test.
A system test consists of 3 steps: Configuration, Build, and Test.
All three steps need to pass for the overall test to be considered successful.

The script itself is executable, but can also be called using CMake's script
mode (i.e. `cmake -P <script>`).

### Usage:
```
usage:

required arguments:
  NAME <value>                System test name
  SOURCE_DIR <value>          System test source directory

optional arguments:
  VERBOSE                     Always print stdout and stderr
  HELP                        Displays this message
  BUILD_DIR <value>           System test build directory (default: ${CMAKE_CURRENT_SOURCE_DIR}/build)
  INSTALL_DIR <value>         System test install directory (default: ${CMAKE_CURRENT_SOURCE_DIR}/install)
  NPROC <value>               Number of processors to use (default: 8)
  CMAKE_DEFINITIONS <value>...
                              System test CMake configuration definitions
  ARGS <value>...             Application arguments
```

### Note:

The verbosity of the output can also be controlled by setting the environment
variable `UTIL_CMAKE_VERBOSE_SYSTEM_TEST` to a true or false value recognized
by CMake.
#]============================================================================]

cmake_minimum_required (VERSION 3.15)

###############################################################################
# Definitions
###############################################################################

set (_include_dir "${CMAKE_CURRENT_LIST_DIR}/../include/util-cmake")

###############################################################################
# Includes
###############################################################################

include (ProcessorCount)

include ("${_include_dir}/colors.cmake")
include ("${_include_dir}/parse_arguments.cmake")

###############################################################################
# Helpers
###############################################################################

macro (_print_output)
  util_cmake_color_string (_stdout_title bold_blue "[[ OUT ]]")
  util_cmake_color_string (_stderr_title bold_blue "[[ ERR ]]")
  util_cmake_color_string (_stderr red "${_stderr}")

  set (_offset "\n   ")
  string (REPLACE "\n" "${_offset}" _stdout "${_stdout}")
  string (REPLACE "\n" "${_offset}" _stderr "${_stderr}")

  message (STATUS
    "${_stdout_title}${_offset}${_offset}${_stdout}"
  )
  message (STATUS
    "${_stderr_title}${_offset}${_offset}${_stderr}"
  )
endmacro()

macro (_status_message _step)
  string (TOUPPER "${_step}" _uc_step)
  util_cmake_color_string (_success bold_green "SUCCESS")
  util_cmake_color_string (_failure bold_red "FAILURE")
  if (_status EQUAL 0)
    message (STATUS
      "[${_success}] ${_arg_NAME} : ${_uc_step}"
    )
    if (_arg_VERBOSE)
      _print_output()
    endif()
  else()
    message (STATUS
      "[${_failure}] ${_arg_NAME} : ${_uc_step}"
    )
    _print_output()
    return()
  endif()
endmacro()

###############################################################################
# Options
###############################################################################

foreach (_index RANGE ${CMAKE_ARGC})
  list (APPEND _argv ${CMAKE_ARGV${_index}})
endforeach()

ProcessorCount (_nproc_default)
if (_nproc_default EQUAL 0)
  set (_nproc_default 1)
endif()

util_cmake_parse_arguments (
  _arg
  "${_argv}"
  OPTION VERBOSE
    DESCRIPTION "Always print stdout and stderr"
  SINGLE_VALUE NAME
    DESCRIPTION "System test name"
    REQUIRED
  SINGLE_VALUE SOURCE_DIR
    DESCRIPTION "System test source directory"
    REQUIRED
  SINGLE_VALUE BUILD_DIR
    DESCRIPTION "System test build directory"
    DEFAULT ${CMAKE_CURRENT_SOURCE_DIR}/build
  SINGLE_VALUE INSTALL_DIR
    DESCRIPTION "System test install directory"
    DEFAULT ${CMAKE_CURRENT_SOURCE_DIR}/install
  SINGLE_VALUE NPROC
    DESCRIPTION "Number of processors to use"
    DEFAULT ${_nproc_default}
  MULTI_VALUE CMAKE_DEFINITIONS
    DESCRIPTION "System test CMake configuration definitions"
  MULTI_VALUE ARGS
    DESCRIPTION "Application arguments"
  NO_MISSING_VALUES
  ENABLE_HELP
)

if (_arg_ARGS)
  string (CONFIGURE "${_arg_ARGS}" _arg_ARGS)
endif()

if (NOT _arg_VERBOSE AND DEFINED ENV{UTIL_CMAKE_VERBOSE_SYSTEM_TESTS})
    set (_arg_VERBOSE $ENV{UTIL_CMAKE_VERBOSE_SYSTEM_TESTS})
endif()

###############################################################################
# Configuration Step
###############################################################################

execute_process (
  COMMAND
    ${CMAKE_COMMAND}
      -D CMAKE_BUILD_TYPE=Release
      -D CMAKE_INSTALL_PREFIX=${_arg_INSTALL_DIR}
      ${_arg_CMAKE_DEFINITIONS}
      -B ${_arg_BUILD_DIR}
      -S ${_arg_SOURCE_DIR}
  OUTPUT_VARIABLE _stdout
  ERROR_VARIABLE _stderr
  RESULT_VARIABLE _status
)

_status_message ("configuration")

###############################################################################
# Build Step
###############################################################################

execute_process (
  COMMAND
    ${CMAKE_COMMAND}
      --build ${_arg_BUILD_DIR}
      --target install
      -j ${_arg_NPROC}
  OUTPUT_VARIABLE _stdout
  ERROR_VARIABLE _stderr
  RESULT_VARIABLE _status
)

_status_message ("build")

###############################################################################
# Test Step
###############################################################################

execute_process (
  COMMAND
    ${_arg_INSTALL_DIR}/bin/${_arg_NAME}
    ${_arg_ARGS}
  OUTPUT_VARIABLE _stdout
  ERROR_VARIABLE _stderr
  RESULT_VARIABLE _status
)

_status_message ("test")
