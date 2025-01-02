// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
