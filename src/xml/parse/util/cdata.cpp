// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/cdata.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/util/expect.hpp>

namespace xml
{
  namespace parse
  {
    std::list<std::string> parse_cdata ( const xml_node_type* node
                                       , const state::type& state
                                       )
    {
      std::list<std::string> v;

      for ( xml_node_type* child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
      {
        expect_none_or (child, rapidxml::node_data, rapidxml::node_cdata, state);

        if (child)
        {
          v.push_back (std::string (child->value(), child->value_size()));
        }
      }

      return v;
    }
  }
}
