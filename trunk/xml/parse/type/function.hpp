// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_FUNCTION_HPP
#define _XML_PARSE_TYPE_FUNCTION_HPP

#include <parse/types.hpp>

#include <parse/util/maybe.hpp>

#include <vector>

#include <boost/variant.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct function
      {
      public:
        typedef std::vector<port> port_vec_type;
        typedef std::vector<std::string> cond_vec_type;
        typedef boost::variant < expression
                               , mod
                               , boost::recursive_wrapper<net>
                               > type; 

        port_vec_type in;
        port_vec_type out;

        maybe<std::string> name;
        maybe<bool> internal;

        cond_vec_type cond;

        type f;
      };
    }
  }
}

#endif
