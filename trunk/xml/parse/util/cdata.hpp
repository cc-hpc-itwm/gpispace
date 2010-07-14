// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_CDATA_HPP
#define _XML_PARSE_UTIL_CDATA_HPP

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
    void
    cdata ( xml_node_type * & node
          , const boost::filesystem::path & path
          )
    {
      skip (node, rapidxml::node_comment);

      try
        {
          expect (node, rapidxml::node_data, rapidxml::node_cdata, path);
        }
      catch (const error::missing_node &)
        {
          // do nothing, there was just no (c)data given
        }

      return;
    }

    std::vector<std::string>
    parse_cdata ( const xml_node_type * node
                , const boost::filesystem::path & path
                )
    {
      std::vector<std::string> v;

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          cdata (child, path);

          if (child)
            {
              v.push_back (std::string (child->value()));
            }
        }

      return v;
    }
  }
}

#endif
