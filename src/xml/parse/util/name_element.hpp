// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_NAME_ELEMENT_HPP
#define _XML_PARSE_UTIL_NAME_ELEMENT_HPP

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

#endif
