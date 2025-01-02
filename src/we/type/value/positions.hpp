// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/value.hpp>

#include <list>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      std::list<std::pair<std::list<std::string>, value_type>>
        positions (value_type const&);
    }
  }
}
