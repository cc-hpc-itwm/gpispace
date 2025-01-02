// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      template<typename T> inline std::string const& name_of (T const&);
    }
  }
}

#include <we/type/value/name_of.ipp>
