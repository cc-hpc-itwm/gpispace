// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      template<typename T>
        inline value_type to_value (T const& x)
      {
        return x;
      }
    }
  }
}
