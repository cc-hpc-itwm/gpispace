// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_HPP
#define _XML_PARSE_TYPE_TRANSITION_HPP

#include <parse/types.hpp>

#include <parse/util/maybe.hpp>

#include <vector>

#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct transition
      {
        typedef std::vector<connect> connect_vec_type;

        connect_vec_type in;
        connect_vec_type out;
        connect_vec_type read;

        // WORK HERE: mutal exclusion not ensured by xsd scheme
        maybe<function> f;
        maybe<std::string> use;

        std::string name;
      };

      std::ostream & operator << (std::ostream & s, const transition & t)
      {
        s << "transition ("
          << "name = " << t.name
          ;

        s << ", connect-in = ";

        for ( transition::connect_vec_type::const_iterator pos (t.in.begin())
            ; pos != t.in.end()
            ; ++pos
            )
          {
            s << *pos << ", ";
          }

        s << "connect-read = ";

        for ( transition::connect_vec_type::const_iterator pos (t.read.begin())
            ; pos != t.read.end()
            ; ++pos
            )
          {
            s << *pos << ", ";
          }

        s << "connect-out = ";

        for ( transition::connect_vec_type::const_iterator pos (t.out.begin())
            ; pos != t.out.end()
            ; ++pos
            )
          {
            s << *pos << ", ";
          }

        
        s << ", use = " << t.use
          << ", function = " << t.f
          ;

        return s << ")";
      }
    }
  }
}

#endif
