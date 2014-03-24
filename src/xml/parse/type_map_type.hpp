// bernd.loerwald@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_MAP_TYPE_HPP
#define _XML_PARSE_TYPE_MAP_TYPE_HPP

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

#endif
