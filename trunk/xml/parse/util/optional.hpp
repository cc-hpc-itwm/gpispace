// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_OPTIONAL_HPP
#define _XML_PARSE_UTIL_OPTIONAL_HPP

#include <xml/parse/rapidxml/1.13/rapidxml.hpp>

#include <xml/parse/types.hpp>
#include <xml/parse/util/maybe.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    maybe<std::string>
    optional (const xml_node_type * node, const Ch * attr)
    {
      return node->first_attribute (attr) 
        ? Just<>(std::string(node->first_attribute (attr)->value()))
        : Nothing<std::string>()
        ;
    }
  }
}

#endif
