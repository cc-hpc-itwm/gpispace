// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_REQUIRED_HPP
#define _XML_PARSE_UTIL_REQUIRED_HPP

#include <parse/rapidxml/1.13/rapidxml.hpp>

#include <parse/types.hpp>

namespace xml
{
  namespace parse
  {
    static std::string
    required ( const std::string & pre
             , const xml_node_type * node
             , const Ch * attr
             )
    {
      if (!node->first_attribute (attr))
        {
          throw exception::missing_attr (pre, attr);
        }

      return node->first_attribute (attr)->value();
    }
  }
}

#endif
