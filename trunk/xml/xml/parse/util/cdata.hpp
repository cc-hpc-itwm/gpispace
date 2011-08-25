// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_CDATA_HPP
#define _XML_PARSE_UTIL_CDATA_HPP

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

    template<typename Container>
    Container
    parse_cdata ( const xml_node_type * node
                , const boost::filesystem::path & path
                )
    {
      Container v;

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
