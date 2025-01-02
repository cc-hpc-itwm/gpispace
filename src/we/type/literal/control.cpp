// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/literal/control.hpp>

#include <iostream>
#include <string>

namespace we
{
  namespace type
  {
    namespace literal
    {
      bool operator== (control const&, control const&)
      {
        return true;
      }

      std::ostream& operator<< (std::ostream& os, control const&)
      {
        return os << std::string("[]");
      }

      std::size_t hash_value (control const&)
      {
        return 42;
      }

      bool operator< (control const&, control const&)
      {
        return false;
      }
    }
  }
}
