// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_NET_HPP
#define _XML_PARSE_TYPE_NET_HPP

#include <parse/types.hpp>
#include <parse/util/unique.hpp>

#include <vector>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

#include <we/type/literal/name.hpp>

#include <iostream>

#include <set>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct net_type
      {
      private:
        xml::util::unique<place_type> _places;
        xml::util::unique<transition_type> _transitions;
        xml::util::unique<function_type,maybe<std::string> > _functions;

      public:
        typedef std::vector<function_type> function_vec_type;
        typedef std::vector<transition_type> transition_vec_type;

        struct_vec_type structs;

        boost::filesystem::path path;

        int level;

        xml::parse::struct_t::set_type structs_resolved;

        // ***************************************************************** //

        bool get_place (const std::string & name, place_type & place) const
        {
          return _places.by_key (name, place);
        }

        bool get_function (const std::string & name, function_type & fun) const
        {
          return _functions.by_key (maybe<std::string>(name), fun);
        }

        // ***************************************************************** //

        const place_vec_type & places (void) const
        {
          return _places.elements();
        }

        const transition_vec_type & transitions (void) const
        {
          return _transitions.elements();
        }

        const function_vec_type & functions (void) const
        {
          return _functions.elements();
        }

        // ***************************************************************** //

        void push (const place_type & place)
        {
          place_type old;

          if (!_places.push (place, old))
            {
              throw error::duplicate_place (place.name, path);
            }
        }

        void push (const transition_type & t)
        {
          transition_type old;

          if (!_transitions.push (t, old))
            {
              throw error::duplicate_transition<transition_type> (t, old);
            }
        }

        void push (const function_type & f)
        {
          function_type old;

          if (!_functions.push (f, old))
            {
              throw error::duplicate_function<function_type> (f, old);
            }
        }

        // ***************************************************************** //

        signature::type type_of_place (const place_type & place) const
        {
          if (literal::valid_name (place.type))
            {
              return signature::type (place.type);
            }

          const xml::parse::struct_t::set_type::const_iterator sig
            (structs_resolved.find (place.type));

          if (sig == structs_resolved.end())
            {
              throw error::place_type_unknown (place.name, place.type, path);
            }

          return signature::type (sig->second.sig, sig->second.name);
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

          st::forbidden_type empty;

          for ( function_vec_type::iterator fun (_functions.elements().begin())
              ; fun != _functions.elements().end()
              ; ++fun
              )
            {
              fun->resolve (structs_resolved, state, empty);
            }

          for ( transition_vec_type::iterator
                  trans (_transitions.elements().begin())
              ; trans != _transitions.elements().end()
              ; ++trans
              )
            {
              trans->resolve (structs_resolved, state, empty);
            }

          for ( place_vec_type::iterator place (_places.elements().begin())
              ; place != _places.elements().end()
              ; ++place
              )
            {
              place->sig = type_of_place (*place);
              place->translate (path, state);
            }
        }

        // ***************************************************************** //

        void type_check (const state::type & state) const
        {
          for ( transition_vec_type::const_iterator trans (transitions().begin())
              ; trans != transitions().end()
              ; ++trans
              )
            {
              trans->type_check<net_type> (*this, state);
            }

          for ( function_vec_type::const_iterator fun (functions().begin())
              ; fun != functions().end()
              ; ++fun
              )
            {
              fun->type_check (state);
            }
        }
      };

      // ******************************************************************* //

      std::ostream & operator << (std::ostream & s, const net_type & n)
      {
        s << "net (path = " << n.path << std::endl;

        s << level(n.level) << "structs =" << std::endl;

        for ( struct_vec_type::const_iterator pos (n.structs.begin())
            ; pos != n.structs.end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level (n.level) << "resolved structs = " << std::endl;

        namespace st = xml::parse::struct_t;

        for ( st::set_type::const_iterator pos (n.structs_resolved.begin())
            ; pos != n.structs_resolved.end()
            ; ++pos
            )
          {
            type::struct_t deep (pos->second);

            deep.level = n.level + 1;

            s << deep << std::endl;
          }

        s << level(n.level) << "functions =" << std::endl;

        for ( net_type::function_vec_type::const_iterator pos
                (n.functions().begin())
            ; pos != n.functions().end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level(n.level) << "places =" << std::endl;

        for ( place_vec_type::const_iterator pos (n.places().begin())
            ; pos != n.places().end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level(n.level) << "transitions =" << std::endl;

        for ( net_type::transition_vec_type::const_iterator pos
                (n.transitions().begin())
            ; pos != n.transitions().end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        return s << level (n.level) << ") //net";
      }
    }
  }
}

#endif
