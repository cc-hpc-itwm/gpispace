// bernd.loerwald@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_MAP_TYPE_HPP
#define _XML_PARSE_TYPE_MAP_TYPE_HPP

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef boost::unordered_map<std::string, std::string> type_map_type;
      typedef boost::unordered_set<std::string> type_get_type;
    }
  }
}

#endif
