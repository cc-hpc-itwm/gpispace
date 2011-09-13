// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_NAME_ELEMENT_HPP
#define _XML_PARSE_UTIL_NAME_ELEMENT_HPP

#include <xml/parse/rapidxml/1.13/rapidxml.hpp>

#include <xml/parse/types.hpp>
#include <xml/parse/error.hpp>
#include <xml/parse/util/skip.hpp>
#include <xml/parse/util/expect.hpp>

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

      return node->name();
    }
  }
}

#endif
