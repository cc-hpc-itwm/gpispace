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
      typedef std::vector<place> place_vec_type;
      typedef std::vector<function> function_vec_type;
      typedef std::vector<transition> transition_vec_type;

      struct net
      {
      private:
        xml::util::unique<place> _places;
        xml::util::unique<transition> _transitions;
        xml::util::unique<function,maybe<std::string> > _functions;

      public:
        struct_vec_type structs;

        boost::filesystem::path path;

        int level;

        xml::parse::struct_t::set_type structs_resolved;

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

        void push (const place & p)
        {
          place old;

          if (!_places.push (p, old))
            {
              throw error::duplicate_place (p.name, path);
            }
        }

        void push (const transition & t)
        {
          transition old;

          if (!_transitions.push (t, old))
            {
              throw error::duplicate_transition<transition> (t, old);
            }
        }

        void push (const function & f)
        {
          function old;

          if (!_functions.push (f, old))
            {
              throw error::duplicate_function<function> (f, old);
            }
        }

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

          literal::name name;

          for ( place_vec_type::iterator place (_places.elements().begin())
              ; place != _places.elements().end()
              ; ++place
              )
            {
              const st::set_type::const_iterator sig
                (structs_resolved.find (place->type));

              if (sig == structs_resolved.end())
                {
                  if (name.valid (place->type))
                    {
                      place->sig = place->type;
                    }
                  else
                    {
                      throw error::place_type_unknown ( place->name
                                                      , place->type
                                                      , path
                                                      )
                        ;
                    }
                }
              else
                {
                  place->sig = sig->second.sig;
                }

              place->translate (path, state);
            }
        }
      };

      std::ostream & operator << (std::ostream & s, const net & n)
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

        for ( function_vec_type::const_iterator pos (n.functions().begin())
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

        for ( transition_vec_type::const_iterator pos (n.transitions().begin())
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
