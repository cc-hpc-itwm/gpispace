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
