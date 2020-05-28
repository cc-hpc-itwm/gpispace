#pragma once

#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/state.hpp>

#include <list>
#include <string>

namespace xml
{
  namespace parse
  {
    std::list<std::string> parse_cdata ( const xml_node_type*
                                       , const state::type&
                                       );
  }
}
