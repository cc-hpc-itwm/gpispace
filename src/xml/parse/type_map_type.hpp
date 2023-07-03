// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      using type_map_type = std::unordered_map<std::string, std::string>;
      using type_get_type = std::unordered_set<std::string>;
    }
  }
}
