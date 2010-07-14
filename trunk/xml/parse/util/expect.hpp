// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_EXPECT_HPP
#define _XML_PARSE_UTIL_EXPECT_HPP

#include <parse/rapidxml/1.13/rapidxml.hpp>

#include <parse/types.hpp>
#include <parse/error.hpp>

#include <parse/util/skip.hpp>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    void expect ( xml_node_type * & node
                , const rapidxml::node_type t
                , const boost::filesystem::path & path
                )
    {
      skip (node, rapidxml::node_comment);

      if (!node)
        {
          throw error::missing_node (t, path);
        }

      if (node->type() != t)
        {
          throw error::wrong_node (t, node->type(), path);
        }
    }

    void expect ( xml_node_type * & node
                , const rapidxml::node_type t1
                , const rapidxml::node_type t2
                , const boost::filesystem::path & path
                )
    {
      skip (node, rapidxml::node_comment);

      if (!node)
        {
          throw error::missing_node (t1, t2, path);
        }

      if (node->type() != t1 && node->type() != t2)
        {
          throw error::wrong_node (t1, t2, node->type(), path);
        }
    }
  }
}

#endif
