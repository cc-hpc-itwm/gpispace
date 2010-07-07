// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_HPP
#define _XML_PARSE_TYPE_TRANSITION_HPP

#include <parse/types.hpp>

#include <parse/util/maybe.hpp>
#include <parse/util/unique.hpp>

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
      // ******************************************************************* //

      struct use_type
      {
        std::string name;
        int level;

        use_type (const std::string & _name, const int & _level) 
          : name (_name) 
          , level (_level)
        {}
      };

      std::ostream & operator << (std::ostream & s, const use_type & u)
      {
        return s << level (u.level) << "use (" << u.name << ")";
      }

      // ******************************************************************* //

      class transition_resolve : public boost::static_visitor<void>
      {
      private:
        const xml::parse::struct_t::set_type global;
        const state::type & state;
        const xml::parse::struct_t::forbidden_type & forbidden;

      public:
        transition_resolve 
        ( const xml::parse::struct_t::set_type & _global
        , const state::type & _state
        , const xml::parse::struct_t::forbidden_type & _forbidden
        )
          : global (_global)
          , state (_state)
          , forbidden (_forbidden)
        {}

        void operator () (use_type &) const { return; }

        template<typename T>
        void operator () (T & x) const
        {
          x.resolve (global, state, forbidden);
        }
      };

      // ******************************************************************* //

      class transition_type_check : public boost::static_visitor<void>
      {
      private:
        const state::type & state;

      public:
        transition_type_check (const state::type & _state) : state (_state) {}

        void operator () (const use_type &) const { return; }
        
        template<typename T>
        void operator () (const T & x) const
        {
          x.type_check (state);
        }
      };
      
      // ******************************************************************* //

      typedef std::vector<connect_type> connect_vec_type;

      struct transition_type
      {
      private:
        xml::util::unique<connect_type> _in;
        xml::util::unique<connect_type> _out;
        xml::util::unique<connect_type> _read;

      public:
        typedef boost::variant <function_type, use_type> f_type;
        
        f_type f;

        std::string name;
        boost::filesystem::path path;

        int level;

        xml::parse::struct_t::set_type structs_resolved;

        // ***************************************************************** //

        const connect_vec_type & in (void) const { return _in.elements(); }
        const connect_vec_type & out (void) const { return _out.elements(); }
        const connect_vec_type & read (void) const { return _read.elements(); }

        // ***************************************************************** //

        void push_in (const connect_type & connect)
        {
          if (!_in.push (connect))
            {
              throw error::duplicate_connect ("in", connect.name, name, path);
            }
        }

        void push_out (const connect_type & connect)
        {
          if (!_out.push (connect))
            {
              throw error::duplicate_connect ("out", connect.name, name, path);
            }
        }

        void push_read (const connect_type & connect)
        {
          if (!_read.push (connect))
            {
              throw error::duplicate_connect ("read", connect.name, name, path);
            }
        }

        // ***************************************************************** //

        void resolve ( const state::type & state
                     , const xml::parse::struct_t::forbidden_type & forbidden
                     )
        {
          const xml::parse::struct_t::set_type empty;

          resolve (empty, state, forbidden);
        }

        void resolve ( const xml::parse::struct_t::set_type & global
                     , const state::type & state
                     , const xml::parse::struct_t::forbidden_type & forbidden
                     )
        {
          boost::apply_visitor ( transition_resolve (global, state, forbidden)
                               , f
                               );
        }

        // ***************************************************************** //

        void type_check (const state::type & state) const
        {
          // local checks

          // recurs
          boost::apply_visitor (transition_type_check (state), f);
        }
      };

      // ******************************************************************* //

      std::ostream & operator << (std::ostream & s, const transition_type & t)
      {
        s << level (t.level)     << "transition (" << std::endl;
        s << level (t.level + 1) << "name = " << t.name << std::endl;
        s << level (t.level + 1) << "path = " << t.path << std::endl;

        s << level (t.level + 1) << "connect-in = " << std::endl;

        for ( connect_vec_type::const_iterator pos (t.in().begin())
            ; pos != t.in().end()
            ; ++pos
            )
          {
            s << level (t.level + 2) << *pos << std::endl;
          }

        s << level (t.level + 1) << "connect-read = " << std::endl;

        for ( connect_vec_type::const_iterator pos (t.read().begin())
            ; pos != t.read().end()
            ; ++pos
            )
          {
            s << level (t.level + 2) << *pos << std::endl;
          }

        s << level (t.level + 1) << "connect-out = " << std::endl;

        for ( connect_vec_type::const_iterator pos (t.out().begin())
            ; pos != t.out().end()
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
