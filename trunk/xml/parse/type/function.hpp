// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_FUNCTION_HPP
#define _XML_PARSE_TYPE_FUNCTION_HPP

#include <parse/types.hpp>

#include <parse/util/maybe.hpp>
#include <parse/util/unique.hpp>

#include <vector>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

#include <we/type/literal.hpp>

#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::vector<port_type> port_vec_type;
      typedef std::vector<std::string> cond_vec_type;

      // ******************************************************************* //

      class function_resolve : public boost::static_visitor<void>
      {
      private:
        const xml::parse::struct_t::set_type global;
        const state::type & state;
        const xml::parse::struct_t::forbidden_type & forbidden;

      public:
        function_resolve
        ( const xml::parse::struct_t::set_type & _global
        , const state::type & _state
        , const xml::parse::struct_t::forbidden_type & _forbidden
        )
          : global (_global)
          , state (_state)
          , forbidden (_forbidden)
        {}

        void operator () (expression_type &) const { return; }
        void operator () (mod_type &) const { return; }

        template<typename T>
        void operator () (T & x) const
        {
          x.resolve (global, state, forbidden);
        }
      };

      // ******************************************************************* //

      class function_type_check : public boost::static_visitor<void>
      {
      private:
        const state::type & state;

      public:
        function_type_check (const state::type & _state) : state (_state) {}

        void operator () (const expression_type &) const { return; }
        void operator () (const mod_type &) const { return; }
        
        template<typename T>
        void operator () (const T & x) const
        {
          x.type_check (state);
        }
      };

      // ******************************************************************* //

      struct function_type
      {
      private:
        xml::util::unique<port_type> _in;
        xml::util::unique<port_type> _out;

      public:
        typedef boost::variant < expression_type
                               , mod_type
                               , boost::recursive_wrapper<net_type>
                               > type; 

        struct_vec_type structs;

        maybe<std::string> name;
        maybe<bool> internal;

        cond_vec_type cond;

        type f;

        boost::filesystem::path path;

        int level;

        xml::parse::struct_t::set_type structs_resolved;

        // ***************************************************************** //

        const port_vec_type & in (void) const { return _in.elements(); }
        const port_vec_type & out (void) const { return _out.elements(); }

        bool get_port_in (const std::string & name, port_type & port) const
        {
          return _in.by_key (name, port);
        }

        bool get_port_out (const std::string & name, port_type & port) const
        {
          return _out.by_key (name, port);
        }

        // ***************************************************************** //

        void push_in (const port_type & p)
        {
          port_type old;

          if (!_in.push (p, old))
            {
              throw error::duplicate_port ("in", p.name, path);
            }
        }

        void push_out (const port_type & p)
        {
          port_type old;

          if (!_out.push (p, old))
            {
              throw error::duplicate_port ("out", p.name, path);
            }
        }

        // ***************************************************************** //

        xml::parse::struct_t::forbidden_type
        forbidden_below (void) const
        {
          xml::parse::struct_t::forbidden_type forbidden;

          for ( port_vec_type::const_iterator pos (in().begin())
              ; pos != in().end()
              ; ++pos
              )
            {
              forbidden[pos->type] = pos->name;
            }

          for ( port_vec_type::const_iterator pos (out().begin())
              ; pos != out().end()
              ; ++pos
              )
            {
              forbidden[pos->type] = pos->name;
            }

          return forbidden;
        }

        // ***************************************************************** //

        void resolve ( const state::type & state
                     , xml::parse::struct_t::forbidden_type & forbidden
                     )
        {
          const xml::parse::struct_t::set_type empty;

          resolve (empty, state, forbidden);
        }

        void resolve 
        ( const xml::parse::struct_t::set_type & global
        , const state::type & state
        , const xml::parse::struct_t::forbidden_type & forbidden
        )
        {
          namespace st = xml::parse::struct_t;

          structs_resolved =
            st::join (global, st::make (structs), forbidden, state);

          for ( st::set_type::iterator pos (structs_resolved.begin())
              ; pos != structs_resolved.end()
              ; ++pos
              )
            {
              boost::apply_visitor 
                ( st::resolve (structs_resolved, pos->second.path)
                , pos->second.sig
                );
            }

          boost::apply_visitor 
            (function_resolve (structs_resolved, state, forbidden_below()), f);
        }

        // ***************************************************************** //

        void type_check (const state::type & state) const
        {
          literal::name name;

          for ( port_vec_type::const_iterator port (in().begin())
              ; port != in().end()
              ; ++port
              )
            {
              if (!name.valid (port->type))
                {
                  xml::parse::struct_t::set_type::const_iterator pos
                    (structs_resolved.find (port->type));

                  if (pos == structs_resolved.end())
                    {
                      throw error::port_with_unknown_type
                        ("in", port->name, port->type, path);
                    }
                }

              boost::apply_visitor 
                (port_type_check<net_type> ("in", *port, path, state), f);
            }

          for ( port_vec_type::const_iterator port (out().begin())
              ; port != out().end()
              ; ++port
              )
            {
              if (!name.valid (port->type))
                {
                  xml::parse::struct_t::set_type::const_iterator pos
                    (structs_resolved.find (port->type));

                  if (pos == structs_resolved.end())
                    {
                      throw error::port_with_unknown_type
                        ("out", port->name, port->type, path);
                    }
                }

              boost::apply_visitor 
                (port_type_check<net_type> ("out", *port, path, state), f);
            }

          boost::apply_visitor (function_type_check (state), f);
        }
      };

      // ******************************************************************* //

      std::ostream & operator << (std::ostream & s, const function_type & f)
      {
        s << level(f.level) << "function (" << std::endl;

        s << level(f.level+1)
          << "name = " << f.name
          << ", internal = " << f.internal
          << std::endl;
        s << level (f.level+1) << "path = " << f.path << std::endl;
        ;

        s << level(f.level+1) << "port_in = " << std::endl;

        for ( port_vec_type::const_iterator pos (f.in().begin())
            ; pos != f.in().end()
            ; ++pos
            )
          {
            s << level(f.level+2) << *pos << std::endl;
          }

        s << level(f.level+1) << "port_out = " << std::endl;

        for ( port_vec_type::const_iterator pos (f.out().begin())
            ; pos != f.out().end()
            ; ++pos
            )
          {
            s << level(f.level+2) << *pos << std::endl;
          }

        s << level(f.level+1) << "structs = " << std::endl;

        for ( struct_vec_type::const_iterator pos (f.structs.begin())
            ; pos != f.structs.end()
            ; ++pos
            )
          {
            type::struct_t deep (*pos);

            deep.level = f.level + 2;

            s << deep << std::endl;
          }

        s << level (f.level+1) << "resolved structs = " << std::endl;

        namespace st = xml::parse::struct_t;

        for ( st::set_type::const_iterator pos (f.structs_resolved.begin())
            ; pos != f.structs_resolved.end()
            ; ++pos
            )
          {
            type::struct_t deep (pos->second);

            deep.level = f.level + 2;

            s << deep << std::endl;
          }

        s << level(f.level+1) << "condition = " << std::endl;

        for ( cond_vec_type::const_iterator pos (f.cond.begin())
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
