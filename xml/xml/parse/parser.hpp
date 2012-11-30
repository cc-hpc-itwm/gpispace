// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_PARSER_HPP
#define _XML_PARSE_PARSER_HPP

#include <xml/parse/id/types.fwd.hpp>
#include <xml/parse/state.fwd.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    id::ref::function just_parse (state::type&, const std::string&);
  }
}

#endif
