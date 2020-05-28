#pragma once

#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/state.fwd.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    std::string required
      (const std::string&, const xml_node_type*, const Ch*, const state::type&);
  }
}
