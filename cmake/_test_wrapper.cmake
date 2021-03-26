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

if (pre_test_hook)
  execute_process (COMMAND "${CMAKE_COMMAND}" "-P" "${pre_test_hook}")
endif()

execute_process (COMMAND "${test_command}" ${test_args} RESULT_VARIABLE result)

if (post_test_hook)
  execute_process (COMMAND "${CMAKE_COMMAND}" "-P" "${post_test_hook}")
endif()

if (NOT ${result} EQUAL 0)
  # Do not change without adjusting FAIL_REGULAR_EXPRESSION in
  # add_macros.cmake's add_unit_test().
  # \note The alternative to a regex would be using a FATAL_ERROR, but
  # that's including a backtrace and loads of whitespace which
  # distracts from the actual test failure. There is no way to trigger
  # a non-zero exit code of a CMake script without a message() either.
  message ("### Test failed with exit code ${result}. ###")
endif()
