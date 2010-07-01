// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_HPP
#define _XML_PARSE_TYPE_TRANSITION_HPP

#include <parse/types.hpp>

#include <parse/util/maybe.hpp>

#include <vector>

#include <iostream>

#include <boost/variant.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct use
      {
        std::string name;
        int level;

        use (const std::string & _name, const int & _level) 
          : name (_name) 
          , level (_level)
        {}
      };

      std::ostream & operator << (std::ostream & s, const use & u)
      {
        return s << level (u.level) << "use (" << u.name << ")";
      }

      struct transition
      {
        typedef std::vector<connect> connect_vec_type;

        connect_vec_type in;
        connect_vec_type out;
        connect_vec_type read;

        typedef boost::variant < function
                               , use
                               > f_type;
        
        f_type f;

        std::string name;

        int level;
      };

      std::ostream & operator << (std::ostream & s, const transition & t)
      {
        s << level (t.level)     << "transition (" << std::endl;
        s << level (t.level + 1) << "name = " << t.name << std::endl;

        s << level (t.level + 1) << "connect-in = " << std::endl;

        for ( transition::connect_vec_type::const_iterator pos (t.in.begin())
            ; pos != t.in.end()
            ; ++pos
            )
          {
            s << level (t.level + 2) << *pos << std::endl;
          }

        s << level (t.level + 1) << "connect-read = " << std::endl;

        for ( transition::connect_vec_type::const_iterator pos (t.read.begin())
            ; pos != t.read.end()
            ; ++pos
            )
          {
            s << level (t.level + 2) << *pos << std::endl;
          }

        s << level (t.level + 1) << "connect-out = " << std::endl;

        for ( transition::connect_vec_type::const_iterator pos (t.out.begin())
            ; pos != t.out.end()
            ; ++pos
            )
          {
            s << level (t.level + 2) << *pos << std::endl;
          }

        s << level (t.level + 1) << "def = " << std::endl;

        boost::apply_visitor (visitor::show (s), t.f);

        s << std::endl;

        return s << level (t.level) << ") // transition";
      }
    }
  }
}

#endif
