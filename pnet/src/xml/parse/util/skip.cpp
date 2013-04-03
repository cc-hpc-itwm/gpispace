// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/skip.hpp>

namespace xml
{
  namespace parse
  {
    void skip (xml_node_type*& node, const rapidxml::node_type t)
    {
      while (node && (node->type() == t))
        {
          node = node->next_sibling();
        }
    }
  }
}
