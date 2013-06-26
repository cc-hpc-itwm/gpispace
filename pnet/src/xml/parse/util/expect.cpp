// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/expect.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/util/skip.hpp>

namespace xml
{
  namespace parse
  {
    void expect_none_or ( xml_node_type*& node
                        , const rapidxml::node_type t
                        , const boost::filesystem::path& path
                        )
    {
      skip (node, rapidxml::node_comment);

      if (node && node->type() != t)
      {
        throw error::wrong_node (t, node->type(), path);
      }
    }
    void expect_none_or ( xml_node_type*& node
                        , const rapidxml::node_type t1
                        , const rapidxml::node_type t2
                        , const boost::filesystem::path& path
                        )
    {
      skip (node, rapidxml::node_comment);

      if (node && node->type() != t1 && node->type() != t2)
      {
        throw error::wrong_node (t1, t2, node->type(), path);
      }
    }
  }
}
