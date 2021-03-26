// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#pragma once

//! cctype wrappers for char
//! see for example https://en.cppreference.com/w/cpp/string/byte/isspace

namespace fhg
{
  namespace util
  {
    bool isalnum (char);
    bool isalpha (char);
    bool isblank (char);
    bool iscntrl (char);
    bool isdigit (char);
    bool isgraph (char);
    bool islower (char);
    bool isprint (char);
    bool ispunct (char);
    bool isspace (char);
    bool isupper (char);
    bool isxdigit (char);
    bool tolower (char);
    bool toupper (char);
  }
}
