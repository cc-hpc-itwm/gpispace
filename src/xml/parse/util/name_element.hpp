#pragma once

#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/state.fwd.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    std::string name_element (xml_node_type*&, const state::type&);
  }
}
