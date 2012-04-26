// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_OPTIONAL_HPP
#define _XML_PARSE_UTIL_OPTIONAL_HPP

#include <xml/parse/rapidxml/1.13/rapidxml.hpp>

#include <xml/parse/types.hpp>
#include <boost/optional.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    inline boost::optional<std::string>
    optional (const xml_node_type * node, const Ch * attr)
    {
      if (!node->first_attribute (attr))
      {
        return boost::none;
      }

      return std::string (node->first_attribute (attr)->value());
    }
  }
}

#endif
