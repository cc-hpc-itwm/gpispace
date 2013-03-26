// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_REQUIRED_HPP
#define _XML_PARSE_UTIL_REQUIRED_HPP

#include <xml/parse/rapidxml/types.hpp>

#include <string>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    std::string required ( const std::string&
                         , const xml_node_type*
                         , const Ch*
                         , const boost::filesystem::path&
                         );
  }
}

#endif
