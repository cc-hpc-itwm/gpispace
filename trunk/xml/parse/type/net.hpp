// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_NET_HPP
#define _XML_PARSE_TYPE_NET_HPP

#include <parse/types.hpp>

#include <vector>

#include <boost/variant.hpp>

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
    }
  }
}

#endif
