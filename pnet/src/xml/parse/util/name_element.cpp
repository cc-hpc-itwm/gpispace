// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/name_element.hpp>

#include <xml/parse/state.hpp>
#include <xml/parse/util/expect.hpp>

namespace xml
{
  namespace parse
  {
    std::string name_element ( xml_node_type*& node
                             , const state::type& state
                             )
    {
      expect_none_or (node, rapidxml::node_element, state);

      if (!node)
      {
        return "<missing_node>";
      }

      return std::string (node->name(), node->name_size());
    }
  }
}
