#!/bin/env -S cmake -P

#[============================================================================[
### Description
Script for running a system test (aka full integration test) that is
a standalone CMake project.

A system test consists of 3 steps: configure, build and install, test.
All three steps need to pass for the overall test to be considered
successful.

Invoked via `cmake -P run_system_test.cmake <arguments>`.

### Arguments:
```
required:
  NAME <value>                System test name
  SOURCE_DIR <value>          System test source directory

optional:
  VERBOSE                     Always print stdout and stderr
  BUILD_DIR <value>           System test build directory
  INSTALL_DIR <value>         System test install directory
  NPROC <value>               Number of processors to use
  CMAKE_DEFINITIONS <value>...
                              Additional CMake definitions
  ARGS <value>...             Application arguments
```

### Note:

The verbosity of the output can also be controlled by setting the
environment variable `GSPC_VERBOSE_SYSTEM_TESTS` to a true or false
value recognized by CMake.
#]============================================================================]

cmake_minimum_required (VERSION 3.15)

###############################################################################
# Includes
###############################################################################

include (ProcessorCount)

###############################################################################
# Helpers
###############################################################################

macro (_print_output)
  set (_offset "\n   ")
  string (REPLACE "\n" "${_offset}" _stdout "${_stdout}")
  string (REPLACE "\n" "${_offset}" _stderr "${_stderr}")

  message (STATUS
    "[[ OUT ]]${_offset}${_offset}${_stdout}"
  )
  message (STATUS
    "[[ ERR ]]${_offset}${_offset}${_stderr}"
  )
endmacro()

macro (_status_message _step)
  string (TOUPPER "${_step}" _uc_step)
  if (_status EQUAL 0)
    message (STATUS
      "[SUCCESS] ${_arg_NAME} : ${_uc_step}"
    )
    if (_arg_VERBOSE)
      _print_output()
    endif()
  else()
    message (STATUS
      "[FAILURE] ${_arg_NAME} : ${_uc_step}"
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

cmake_parse_arguments (_arg
  "VERBOSE"
  "NAME;SOURCE_DIR;BUILD_DIR;INSTALL_DIR;NPROC"
  "CMAKE_DEFINITIONS;ARGS"
  ${_argv}
)

if (_arg_KEYWORDS_MISSING_VALUES)
  message (FATAL_ERROR
    "arguments missing values: ${_arg_KEYWORDS_MISSING_VALUES}"
  )
endif()

if (NOT _arg_NAME)
  message (FATAL_ERROR "NAME is required")
endif()
if (NOT _arg_SOURCE_DIR)
  message (FATAL_ERROR "SOURCE_DIR is required")
endif()
if (NOT _arg_BUILD_DIR)
  set (_arg_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")
endif()
if (NOT _arg_INSTALL_DIR)
  set (_arg_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/install")
endif()
if (NOT _arg_NPROC)
  set (_arg_NPROC "${_nproc_default}")
endif()

if (_arg_ARGS)
  string (CONFIGURE "${_arg_ARGS}" _arg_ARGS)
endif()

if (NOT _arg_VERBOSE AND DEFINED ENV{GSPC_VERBOSE_SYSTEM_TESTS})
    set (_arg_VERBOSE $ENV{GSPC_VERBOSE_SYSTEM_TESTS})
endif()

###############################################################################
# Configuration
###############################################################################

execute_process (
  COMMAND
    ${CMAKE_COMMAND}
      -D CMAKE_BUILD_TYPE=Debug
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
# Build
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
# Test
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
