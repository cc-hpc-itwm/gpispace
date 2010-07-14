// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_NAME_ELEMENT_HPP
#define _XML_PARSE_UTIL_NAME_ELEMENT_HPP

#include <parse/rapidxml/1.13/rapidxml.hpp>

#include <parse/types.hpp>
#include <parse/error.hpp>
#include <parse/util/skip.hpp>
#include <parse/util/expect.hpp>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    std::string
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
