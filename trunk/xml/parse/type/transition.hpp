// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_HPP
#define _XML_PARSE_TYPE_TRANSITION_HPP

#include <parse/types.hpp>

#include <parse/util/maybe.hpp>

#include <vector>

#include <iostream>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

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

      class transition_resolve : public boost::static_visitor<void>
      {
      private:
        const xml::parse::struct_t::set_type global;
        const state::type & state;

      public:
        transition_resolve ( const xml::parse::struct_t::set_type & _global
                           , const state::type & _state
                           )
          : global (_global)
          , state (_state)
        {}

        void operator () (use &) const { return; }

        template<typename T>
        void operator () (T & x) const { x.resolve (global, state); }
      };
      
      struct transition
      {
        typedef std::vector<connect> connect_vec_type;

        connect_vec_type in;
        connect_vec_type out;
        connect_vec_type read;

        typedef boost::variant <function, use> f_type;
        
        f_type f;

        std::string name;
        boost::filesystem::path path;

        int level;

        xml::parse::struct_t::set_type structs_resolved;

        void resolve (const state::type & state)
        {
          const xml::parse::struct_t::set_type empty;

          resolve (empty, state);
        }

        void resolve ( const xml::parse::struct_t::set_type & global
                     , const state::type & state
                     )
        {
          boost::apply_visitor (transition_resolve (global, state), f);
        }
      };

      std::ostream & operator << (std::ostream & s, const transition & t)
      {
        s << level (t.level)     << "transition (" << std::endl;
        s << level (t.level + 1) << "name = " << t.name << std::endl;
        s << level (t.level + 1) << "path = " << t.path << std::endl;

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
