// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_NET_HPP
#define _XML_PARSE_TYPE_NET_HPP

#include <parse/types.hpp>

#include <vector>

#include <boost/variant.hpp>

#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct net
      {
        typedef boost::variant < place
                               , boost::recursive_wrapper<function>
                               , transition
                               > element_type;

        typedef std::vector<element_type> element_vec_type;

        element_vec_type element;
      };

      std::ostream & operator << (std::ostream & s, const net & n)
      {
        s << "net (";

        for ( net::element_vec_type::const_iterator pos (n.element.begin())
            ; pos != n.element.end()
            ; ++pos
            )
          {
            if (pos != n.element.begin())
              {
                s << ", ";
              }

            boost::apply_visitor (visitor::show (s), *pos);
          }

        return s << ")";
      }
    }
  }
}

#endif
