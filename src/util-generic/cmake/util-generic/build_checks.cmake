# Copyright (C) 2023 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

include_guard ()

###############################################################################
# Includes
###############################################################################

include (CheckCXXCompilerFlag)
include (CheckCXXSourceRuns)
include (CheckFunctionExists)

###############################################################################
# Run Checks
###############################################################################

check_function_exists (signalfd FUNCTION_EXISTS_SIGNALFD)
check_function_exists (pipe2 FUNCTION_EXISTS_PIPE2)

check_cxx_compiler_flag ("-Werror=old-style-cast"
  COMPILER_FLAG_EXISTS_WERROR_OLD_STYLE_CAST
)

if (COMPILER_FLAG_EXISTS_WERROR_OLD_STYLE_CAST)
  set (CMAKE_REQUIRED_FLAGS_SAVED "${CMAKE_REQUIRED_FLAGS}")
  set (CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -Werror=old-style-cast")
  check_cxx_source_runs ([[
    #include <sys/wait.h>
    int main (int argc, char**)
    {
      return WEXITSTATUS (argc);
      //     ^^^^^^^^^^^
    }
    ]]
    HAS_WEXITSTATUS_WITHOUT_OLD_STYLE_CAST
  )
  check_cxx_source_runs ([[
    #include <signal.h>
    int main (int, char**)
    {
      return SIG_ERR == nullptr;
      //     ^^^^^^^
    }
    ]]
    HAS_SIG_ERR_WITHOUT_OLD_STYLE_CAST
  )
  check_cxx_source_runs ([[
    #include <sys/mman.h>
    int main (int, char**)
    {
      return MAP_FAILED == nullptr;
      //     ^^^^^^^^^^
    }
    ]]
    HAS_MAP_FAILED_WITHOUT_OLD_STYLE_CAST
  )
  set (CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS_SAVED}")
else()
  set (HAS_WEXITSTATUS_WITHOUT_OLD_STYLE_CAST 1)
  set (HAS_SIG_ERR_WITHOUT_OLD_STYLE_CAST 1)
  set (HAS_MAP_FAILED_WITHOUT_OLD_STYLE_CAST 1)
endif()
