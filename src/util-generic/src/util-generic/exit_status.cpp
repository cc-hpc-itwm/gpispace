// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
