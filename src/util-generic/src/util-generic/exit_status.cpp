// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/exit_status.hpp>

#include <sys/wait.h>

#if !HAS_WEXITSTATUS_WITHOUT_OLD_STYLE_CAST
# include <util-generic/warning.hpp>
# define SUPPRESS_OLD_STYLE_CAST_WARNING(what...)       \
  DISABLE_WARNING_GCC ("-Wold-style-cast")              \
  what                                                  \
  RESTORE_WARNING_GCC ("-Wold-style-cast")
#else
# define SUPPRESS_OLD_STYLE_CAST_WARNING(what...)       \
  what
#endif

namespace fhg
{
  namespace util
  {
    int wexitstatus (int status)
    {
      SUPPRESS_OLD_STYLE_CAST_WARNING (return WEXITSTATUS (status);)
    }
    int wifcontinued (int status)
    {
      SUPPRESS_OLD_STYLE_CAST_WARNING (return WIFCONTINUED (status);)
    }
    int wifexited (int status)
    {
      SUPPRESS_OLD_STYLE_CAST_WARNING (return WIFEXITED (status);)
    }
    int wifsignaled (int status)
    {
      SUPPRESS_OLD_STYLE_CAST_WARNING (return WIFSIGNALED (status);)
    }
    int wifstopped (int status)
    {
      SUPPRESS_OLD_STYLE_CAST_WARNING (return WIFSTOPPED (status);)
    }
    int wstopsig (int status)
    {
      SUPPRESS_OLD_STYLE_CAST_WARNING (return WSTOPSIG (status);)
    }
    int wtermsig (int status)
    {
      SUPPRESS_OLD_STYLE_CAST_WARNING (return WTERMSIG (status);)
    }
  }
}
