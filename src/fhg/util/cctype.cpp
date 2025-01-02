// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/cctype.hpp>

#include <cctype>

namespace fhg
{
  namespace util
  {
#define WRAP(F)                                         \
    bool F (char c)                                     \
    {                                                   \
      return std::F (static_cast<unsigned char> (c));   \
    }

    WRAP (isalnum)
    WRAP (isalpha)
    WRAP (isblank)
    WRAP (iscntrl)
    WRAP (isdigit)
    WRAP (isgraph)
    WRAP (islower)
    WRAP (isprint)
    WRAP (ispunct)
    WRAP (isspace)
    WRAP (isupper)
    WRAP (isxdigit)
    WRAP (tolower)
    WRAP (toupper)

#undef WRAP
  }
}
