// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_NAME_ELEMENT_HPP
#define _XML_PARSE_UTIL_NAME_ELEMENT_HPP

#include <parse/rapidxml/1.13/rapidxml.hpp>

#include <parse/types.hpp>
#include <parse/util/skip.hpp>
#include <parse/util/expect.hpp>

namespace xml
{
  namespace parse
  {
    std::string
    name_element (xml_node_type * & node)
    {
      skip (node, rapidxml::node_comment);
      expect (node, rapidxml::node_element);

      return node->name();
    }
  }
}

#endif
