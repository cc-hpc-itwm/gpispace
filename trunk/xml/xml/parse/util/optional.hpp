// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_OPTIONAL_HPP
#define _XML_PARSE_UTIL_OPTIONAL_HPP

#include <xml/parse/rapidxml/1.13/rapidxml.hpp>

#include <xml/parse/types.hpp>
#include <fhg/util/maybe.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    inline fhg::util::maybe<std::string>
    optional (const xml_node_type * node, const Ch * attr)
    {
      return node->first_attribute (attr)
        ? fhg::util::Just<>(std::string(node->first_attribute (attr)->value()))
        : fhg::util::Nothing<std::string>()
        ;
    }
  }
}

#endif
