# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

include_guard()

###############################################################################
# Includes
###############################################################################

include (ProcessorCount)

include (util-cmake/parse_arguments)

###############################################################################
# Private
###############################################################################

set (_util_cmake_scripts_dir "${CMAKE_CURRENT_LIST_DIR}/../../scripts")

###############################################################################
# Public
###############################################################################

#[============================================================================[
### Description:
This function creates a system test (aka full integration tests). System tests
are configured, built, and executed during CTest execution and not during
the regular CMake phases. This ensures the necessary isolation from the
build-tree. System tests require a project to be installed and will fail
otherwise. System tests are automatically labelled as `system-test`.

#### Command:
```cmake
util_cmake_add_system_test (
  NAME <test-name>
  [NPROC <num-processors>]
  [LABELS <label>...]
  [CMAKE_DEFINITIONS <definition>...]
  [ARGS <application-argument>...]
)
```
Required: `NAME`
#]============================================================================]
function (util_cmake_add_system_test _name)
  if (NOT BUILD_TESTING)
    return()
  endif()

  ProcessorCount (_nproc_default)
  if (_nproc_default EQUAL 0)
    set (_nproc_default 1)
  endif()

  util_cmake_parse_arguments (_arg
    "${ARGN}"
    SINGLE_VALUE NPROC
      DESCRIPTION "Number of processors to use"
      DEFAULT ${_nproc_default}
    SINGLE_VALUE SOURCE_DIR
      DESCRIPTION "System test source directory"
      DEFAULT "${CMAKE_CURRENT_SOURCE_DIR}/${_name}"
    SINGLE_VALUE BUILD_DIR
      DESCRIPTION "System test build directory"
      DEFAULT "${PROJECT_BINARY_DIR}/system_test/${_name}"
    MULTI_VALUE LABELS
      DESCRIPTION "Additional test labels"
      DEFAULT "system-test"
      APPEND
    MULTI_VALUE CMAKE_DEFINITIONS
      DESCRIPTION "System test CMake configuration definitions"
    MULTI_VALUE ARGS
      DESCRIPTION "Application arguments"
    NO_UNPARSED_ARGUMENTS
    NO_MISSING_VALUES
  )

  set (_test_name "system-test_${_name}")

  add_test (
    NAME ${_test_name}
    COMMAND
      ${CMAKE_COMMAND} -P "${_util_cmake_scripts_dir}/run_system_test.cmake"
        NAME ${_name}
        SOURCE_DIR ${_arg_SOURCE_DIR}
        BUILD_DIR ${_arg_BUILD_DIR}/build
        INSTALL_DIR ${_arg_BUILD_DIR}/install
        CMAKE_DEFINITIONS ${_arg_CMAKE_DEFINITIONS}
        NPROC ${_arg_NPROC}
        ARGS ${_arg_ARGS}
  )

  set_tests_properties (${_test_name}
    PROPERTIES
      FAIL_REGULAR_EXPRESSION "FAILURE.*${_name} :"
  )

  set_property (TEST ${_test_name}
    APPEND PROPERTY
      LABELS ${_arg_LABELS}
  )
endfunction()
