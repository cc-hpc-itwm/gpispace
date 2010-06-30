// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_FUNCTION_HPP
#define _XML_PARSE_TYPE_FUNCTION_HPP

#include <parse/types.hpp>

#include <parse/util/maybe.hpp>

#include <vector>

#include <boost/variant.hpp>

#include <iostream>

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

      std::ostream & operator << (std::ostream & s, const function & f)
      {
        s << "function ("
          << "name = " << f.name
          << ", interal = " << f.internal
          ;

        s << ", port_in = ";

        for ( function::port_vec_type::const_iterator pos (f.in.begin())
            ; pos != f.in.end()
            ; ++pos
            )
          {
            s << *pos << ", ";
          }

        s << "port_out = ";

        for ( function::port_vec_type::const_iterator pos (f.out.begin())
            ; pos != f.out.end()
            ; ++pos
            )
          {
            s << *pos << ", ";
          }

        s << "condition = ";

        for ( function::cond_vec_type::const_iterator pos (f.cond.begin())
            ; pos != f.cond.end()
            ; ++pos
            )
          {
            s << *pos << ", ";
          }

        s << "fun = ";

        boost::apply_visitor (visitor::show (s), f.f);

        return s << ")";
      }
    }
  }
}

#endif
