// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_SKIP_HPP
#define _XML_PARSE_UTIL_SKIP_HPP

#include <xml/parse/rapidxml/types.hpp>

namespace xml
{
  namespace parse
  {
    void skip (xml_node_type*&, const rapidxml::node_type);
  }
}

#endif
