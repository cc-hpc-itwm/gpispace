// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_NAME_ELEMENT_HPP
#define _XML_PARSE_UTIL_NAME_ELEMENT_HPP

#include <xml/parse/error.hpp>
#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/util/expect.hpp>
#include <xml/parse/util/skip.hpp>

#include <string>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    inline std::string
    name_element ( xml_node_type * & node
                 , const boost::filesystem::path & path
                 )
    {
      skip (node, rapidxml::node_comment);

      try
        {
          expect (node, rapidxml::node_element, path);
        }
      catch (const error::missing_node &)
        {
          return "<missing_node>";
        }

      return std::string (node->name(), node->name_size());
    }
  }
}

#endif
