// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_EXPECT_HPP
#define _XML_PARSE_UTIL_EXPECT_HPP

#include <parse/rapidxml/1.13/rapidxml.hpp>

#include <parse/types.hpp>
#include <parse/error.hpp>

#include <parse/util/skip.hpp>

namespace xml
{
  namespace parse
  {
    void expect (xml_node_type * & node, const rapidxml::node_type t)
    {
      skip (node, rapidxml::node_comment);

      if (!node)
        {
          throw error::missing_node (t);
        }

      if (node->type() != rapidxml::node_element)
        {
          throw error::wrong_node (t, node->type());
        }
    }
  }
}

#endif
