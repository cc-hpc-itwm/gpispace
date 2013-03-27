// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_OPTIONAL_HPP
#define _XML_PARSE_UTIL_OPTIONAL_HPP

#include <xml/parse/rapidxml/types.hpp>

#include <string>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    boost::optional<std::string> optional (const xml_node_type*, const Ch*);
  }
}

#endif
