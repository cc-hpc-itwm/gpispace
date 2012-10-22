// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_REQUIRED_HPP
#define _XML_PARSE_UTIL_REQUIRED_HPP

#include <xml/parse/error.hpp>
#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/types.hpp>

#include <string>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    inline std::string
    required ( const std::string & pre
             , const xml_node_type * node
             , const Ch * attr
             , const boost::filesystem::path & path
             )
    {
      if (!node->first_attribute (attr))
        {
          throw error::missing_attr (pre, attr, path);
        }

      return node->first_attribute (attr)->value();
    }
  }
}

#endif
