// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_NAME_ELEMENT_HPP
#define _XML_PARSE_UTIL_NAME_ELEMENT_HPP

#include <xml/parse/rapidxml/types.hpp>

#include <string>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    std::string name_element ( xml_node_type*&
                             , const boost::filesystem::path&
                             );
  }
}

#endif
