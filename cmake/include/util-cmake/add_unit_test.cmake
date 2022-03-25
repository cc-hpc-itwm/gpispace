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

#"BEGIN INCLUDE"
include (util-cmake/parse_arguments)
include (util-cmake/add_macros)
include (util-cmake/_bundle)
include (util-cmake/beautify_find_boost)
#"END INCLUDE"

set (_add_macros_test_wrapper "${CMAKE_CURRENT_LIST_DIR}/_test_wrapper.cmake")

#! Convenience wrapper around `add_executable()` and `add_test()` as
#! well as various properties.
#!
#! A test with name \a NAME will be added, executing the given \a
#! SOURCES and \a MOC. The executable will be called with the given \a
#! ARGS as command line arguments. Optionally a \a DESCRIPTION can be
#! given, which is free text and for informational purposes only.
#!
#! For convenience \a USE_BOOST will automatically set up everything
#! so that a single source file including boost/test and adding a
#! `BOOST_AUTO_TEST_CASE()` will be enough. The flag will
#! automatically handle modification of \a ARGS, define
#! `-DBOOST_TEST_MODULE` and link `Boost::unit_test_framework`.
#!
#! Tests that are unable to share resources (CPU, memory, hardcoded
#! filepaths/sockets, ...) can specify \a RUN_SERIAL in order to not
#! be executed in parallel if `ctest -j` is used. Tests that are
#! performance critical shall use \a PERFORMANCE_TEST instead, which
#! implies \a RUN_SERIAL but additionally adds the `performance_test`
#! label for sake of filtering.
#!
#! Labels for filtering using `ctest -L` can be added using \a LABELS.
#!
#! If files are required to exist for the test to run, the list \a
#! REQUIRED_FILES can be set. If the files do not exist when `ctest`
#! is executed, the test will be marked "not run" instead of
#! failing. This can be used for tests that require the project to be
#! installed while still allowing to run tests before installing.
#!
#! While tests should be self-contained, there might be some cleanup
#! needed for unsuccessful tests leaking resources. To do that, \a
#! PRE_TEST_HOOK and \a POST_TEST_HOOK can point to CMake scripts that
#! are executed before and after the test runs, regardless of
#! result. Their results are ignored so that they are not abused as
#! pre/post-test conditions that should be in the actual test. Their
#! output is not suppressed though. Such a script could ensure a
#! directory doesn't exist, a filesystem is mounted, or that no
#! process of a given name is running.
#! \todo Timeouts are *not* taken into account, so the post hook will
#! not be executed if the test hangs!
#! \note While the scripts can differ from each other, this is likely
#! a sign of abusing the hooks for something that should be in the
#! test: Cleaning up behind a failed test and cleaning up a previously
#! failed test that didn't clean up should be the same.
#!
#! The arguments \a LIBRARIES, \a INCLUDE_DIRECTORIES, \a
#! SYSTEM_INCLUDE_DIRECTORIES, \a COMPILE_FLAGS, \a
#! COMPILE_DEFINITIONS, \a DEPENDS and \a NO_RDYNAMIC work as
#! described in the documentation for \see extended_add_executable().
#!
#! \note The automatically generated underlying executable target is
#! named `${NAME}.test`. The test target is named `${NAME}`.
function (add_unit_test)
  set (QT_OPTIONS)
  if (TARGET Qt4::QtCore OR TARGET Qt5::Core)
    set (QT_OPTIONS MOC)
  endif()

  set (options USE_BOOST PERFORMANCE_TEST RUN_SERIAL NO_RDYNAMIC)
  set (one_value_options NAME DESCRIPTION PRE_TEST_HOOK POST_TEST_HOOK)
  set (multi_value_options LIBRARIES ${QT_OPTIONS} SOURCES INCLUDE_DIRECTORIES
    SYSTEM_INCLUDE_DIRECTORIES ARGS DEPENDS COMPILE_FLAGS LABELS REQUIRED_FILES
    COMPILE_DEFINITIONS)
  set (required_options NAME SOURCES)
  _parse_arguments (ARG "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  if (NOT BUILD_TESTING)
    return()
  endif()

  _default_if_unset (ARG_DESCRIPTION "${ARG_NAME}")

  set (target_name ${ARG_NAME}.test)

  _moc (${ARG_NAME}_mocced ${ARG_MOC})

  _ensure_rpath_globals_before_target_addition()
  add_executable (${target_name} ${${ARG_NAME}_mocced} ${ARG_SOURCES})

  if (NOT ARG_NO_RDYNAMIC)
    set_property (TARGET ${target_name} APPEND_STRING
      PROPERTY LINK_FLAGS " -rdynamic"
      )
  endif()

  if (ARG_USE_BOOST)
    find_boost (1.59 REQUIRED QUIET COMPONENTS unit_test_framework)

    list (APPEND ARG_LIBRARIES Boost::unit_test_framework)
    target_compile_definitions (${target_name} PRIVATE
      "-DBOOST_TEST_MODULE=\"${ARG_DESCRIPTION}\""
    )

    if (Boost_VERSION VERSION_EQUAL 1.60 OR Boost_VERSION VERSION_GREATER 1.60)
      list (INSERT ARG_ARGS 0 "--")
    endif()
  endif()

  if (ARG_SYSTEM_INCLUDE_DIRECTORIES)
    target_include_directories (${target_name} SYSTEM
      ${ARG_SYSTEM_INCLUDE_DIRECTORIES})
  endif()
  if (ARG_INCLUDE_DIRECTORIES)
    target_include_directories (${target_name} ${ARG_INCLUDE_DIRECTORIES})
  endif()

  target_link_libraries (${target_name} PRIVATE ${ARG_LIBRARIES})
  if (NOT ARG_PRE_TEST_HOOK AND NOT ARG_POST_TEST_HOOK)
    add_test (NAME ${ARG_NAME} COMMAND
      $<TARGET_FILE:${target_name}> ${ARG_ARGS}
    )
  else()
    add_test (NAME ${ARG_NAME} COMMAND
      "${CMAKE_COMMAND}"
        -D "test_command=$<TARGET_FILE:${target_name}>"
        -D "test_args=${ARG_ARGS}"
        -D "pre_test_hook=${ARG_PRE_TEST_HOOK}"
        -D "post_test_hook=${ARG_POST_TEST_HOOK}"
        -P "${_add_macros_test_wrapper}"
    )
    set_tests_properties (${ARG_NAME} PROPERTIES
      FAIL_REGULAR_EXPRESSION "### Test failed with exit code .*[.] ###"
    )
  endif()

  if (ARG_DEPENDS)
    add_dependencies (${target_name} ${ARG_DEPENDS})
  endif()

  if (ARG_PERFORMANCE_TEST)
    list (APPEND ARG_LABELS "performance_test")
    set (ARG_RUN_SERIAL 1)
  endif()

  if (ARG_COMPILE_FLAGS)
    set_property (TARGET ${target_name} PROPERTY COMPILE_FLAGS ${ARG_COMPILE_FLAGS})
  endif()

  if (ARG_COMPILE_DEFINITIONS)
    target_compile_definitions (${target_name} ${ARG_COMPILE_DEFINITIONS})
  endif()

  if (ARG_RUN_SERIAL)
    set_property (TEST ${ARG_NAME} APPEND PROPERTY RUN_SERIAL 1)
  endif()

  if (ARG_LABELS)
    set_property (TEST ${ARG_NAME} APPEND PROPERTY LABELS ${ARG_LABELS})
  endif()

  if (ARG_REQUIRED_FILES)
    set_property (TEST ${ARG_NAME} PROPERTY REQUIRED_FILES ${ARG_REQUIRED_FILES})
  endif()
endfunction()
