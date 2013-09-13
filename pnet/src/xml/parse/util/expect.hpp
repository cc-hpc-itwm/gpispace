// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_EXPECT_HPP
#define _XML_PARSE_UTIL_EXPECT_HPP

#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/state.fwd.hpp>

namespace xml
{
  namespace parse
  {
    void expect_none_or ( xml_node_type*& node
                        , const rapidxml::node_type
                        , const state::type&
                        );
    void expect_none_or ( xml_node_type*& node
                        , const rapidxml::node_type
                        , const rapidxml::node_type
                        , const state::type&
                        );
  }
}

#endif
