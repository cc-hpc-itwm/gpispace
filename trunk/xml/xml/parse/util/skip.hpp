// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_SKIP_HPP
#define _XML_PARSE_UTIL_SKIP_HPP

#include <xml/parse/types.hpp>

namespace xml
{
  namespace parse
  {
    inline void
    skip (xml_node_type * & node, const rapidxml::node_type t)
    {
      while (node && (node->type() == t))
        {
          node = node->next_sibling();
        }
    }
  }
}

#endif
