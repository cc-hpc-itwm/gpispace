// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_CDATA_HPP
#define _XML_PARSE_UTIL_CDATA_HPP

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

#endif
