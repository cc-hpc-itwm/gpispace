// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/sig_err.hpp>

#if !HAS_SIG_ERR_WITHOUT_OLD_STYLE_CAST
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
    sighandler_t sig_err()
    {
      SUPPRESS_OLD_STYLE_CAST_WARNING (return SIG_ERR;)
    }
  }
}
