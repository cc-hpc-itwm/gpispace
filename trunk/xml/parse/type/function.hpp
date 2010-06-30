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

        int level;
      };

      std::ostream & operator << (std::ostream & s, const function & f)
      {
        s << level(f.level) << "function (" << std::endl;

        s << level(f.level+1)
          << "name = " << f.name
          << ", interal = " << f.internal
          << std::endl;
        ;

        s << level(f.level+1) << "port_in = " << std::endl;

        for ( function::port_vec_type::const_iterator pos (f.in.begin())
            ; pos != f.in.end()
            ; ++pos
            )
          {
            s << level(f.level+2) << *pos << std::endl;
          }

        s << level(f.level+1) << "port_out = " << std::endl;

        for ( function::port_vec_type::const_iterator pos (f.out.begin())
            ; pos != f.out.end()
            ; ++pos
            )
          {
            s << level(f.level+2) << *pos << std::endl;
          }

        s << level(f.level+1) << "condition = " << std::endl;

        for ( function::cond_vec_type::const_iterator pos (f.cond.begin())
            ; pos != f.cond.end()
            ; ++pos
            )
          {
            s << level(f.level+2) << *pos << std::endl;
          }

        s << level(f.level+1) << "fun = ";

        boost::apply_visitor (visitor::show (s), f.f);

        s << std::endl;

        return s << level(f.level) << ") // function";
      }
    }
  }
}

#endif
