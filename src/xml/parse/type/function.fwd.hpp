// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <list>
#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct conditions_type : public std::list<std::string>
      {
        std::string flatten() const;
      };

      conditions_type operator+ (conditions_type, conditions_type const&);

      struct function_type;
    }
  }
}
