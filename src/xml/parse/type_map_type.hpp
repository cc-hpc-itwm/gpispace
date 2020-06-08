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
      typedef std::unordered_map<std::string, std::string> type_map_type;
      typedef std::unordered_set<std::string> type_get_type;
    }
  }
}
