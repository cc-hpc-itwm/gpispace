// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_OPTIONAL_HPP
#define _XML_PARSE_UTIL_OPTIONAL_HPP

#include <xml/parse/rapidxml/types.hpp>

#include <string>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    inline boost::optional<std::string>
    optional (const xml_node_type * node, const Ch * attr)
    {
      if (node->first_attribute (attr))
      {
        return std::string ( node->first_attribute (attr)->value()
                           , node->first_attribute (attr)->value_size()
                           );
      }
      return boost::none;
    }
  }
}

#endif
