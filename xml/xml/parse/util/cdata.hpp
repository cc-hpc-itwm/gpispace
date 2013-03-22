// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_CDATA_HPP
#define _XML_PARSE_UTIL_CDATA_HPP

#include <xml/parse/error.hpp>
#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/util/expect.hpp>
#include <xml/parse/util/skip.hpp>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    inline void
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
              v.push_back (std::string (child->value(), child->value_size()));
            }
        }

      return v;
    }
  }
}

#endif
