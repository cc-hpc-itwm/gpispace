#! This script is not intended to be used stand-alone but is used by

# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! `add_unit_test.cmake` to wrap (unit) tests with code to execute before

# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

#! and after running the test itself.

# Copyright (C) 2025 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later


if (pre_test_hook)
  execute_process (COMMAND "${CMAKE_COMMAND}" "-P" "${pre_test_hook}")
endif()

execute_process (COMMAND "${test_command}" ${test_args} RESULT_VARIABLE result)

if (post_test_hook)
  execute_process (COMMAND "${CMAKE_COMMAND}" "-P" "${post_test_hook}")
endif()

if (NOT ${result} EQUAL 0)
  # Do not change without adjusting FAIL_REGULAR_EXPRESSION in
  # add_unit_test.cmake's add_unit_test().
  # \note The alternative to a regex would be using a FATAL_ERROR, but
  # that's including a backtrace and loads of whitespace which
  # distracts from the actual test failure. There is no way to trigger
  # a non-zero exit code of a CMake script without a message() either.
  message ("### Test failed with exit code ${result}. ###")
endif()
