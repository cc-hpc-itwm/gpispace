// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_EXPECT_HPP
#define _XML_PARSE_UTIL_EXPECT_HPP

#include <xml/parse/rapidxml/types.hpp>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    void expect ( xml_node_type*&
                , const rapidxml::node_type
                , const boost::filesystem::path&
                );
    void expect ( xml_node_type*& node
                , const rapidxml::node_type
                , const rapidxml::node_type
                , const boost::filesystem::path&
                );
  }
}

#endif
