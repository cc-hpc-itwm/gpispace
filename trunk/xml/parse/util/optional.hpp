// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_OPTIONAL_HPP
#define _XML_PARSE_UTIL_OPTIONAL_HPP

#include <parse/rapidxml/1.13/rapidxml.hpp>

#include <parse/types.hpp>
#include <parse/util/maybe.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    static bool
    optional (const xml_node_type * node, const Ch * attr, std::string & val)
    {
      if (node->first_attribute (attr))
        {
          val = std::string (node->first_attribute (attr)->value());
        }

      return node->first_attribute (attr);
    }

    static maybe<std::string>
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
