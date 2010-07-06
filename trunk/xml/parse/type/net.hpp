// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_NET_HPP
#define _XML_PARSE_TYPE_NET_HPP

#include <parse/types.hpp>

#include <vector>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct net
      {
        typedef boost::variant < struct_t
                               , place
                               , boost::recursive_wrapper<function>
                               , transition
                               > element_type;

        typedef std::vector<element_type> element_vec_type;

        element_vec_type element;

        boost::filesystem::path path;

        int level;
      };

      std::ostream & operator << (std::ostream & s, const net & n)
      {
        s << "net (path = " << n.path << std::endl;

        for ( net::element_vec_type::const_iterator pos (n.element.begin())
            ; pos != n.element.end()
            ; ++pos
            )
          {
            boost::apply_visitor (visitor::show (s), *pos);

            s << std::endl;
          }

        return s << level (n.level) << ") //net";
      }
    }
  }
}

#endif
