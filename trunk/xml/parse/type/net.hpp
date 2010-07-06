// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_NET_HPP
#define _XML_PARSE_TYPE_NET_HPP

#include <parse/types.hpp>

#include <vector>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

#include <we/type/literal/name.hpp>

#include <iostream>

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
        struct_vec_type structs;
        function_vec_type functions;
        place_vec_type places;
        transition_vec_type transitions;

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
          namespace st = xml::parse::struct_t;

          structs_resolved = st::join (global, st::make (structs), state);

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

          for ( function_vec_type::iterator fun (functions.begin())
              ; fun != functions.end()
              ; ++fun
              )
            {
              fun->resolve (structs_resolved, state);
            }

          for ( transition_vec_type::iterator trans (transitions.begin())
              ; trans != transitions.end()
              ; ++trans
              )
            {
              trans->resolve (structs_resolved, state);
            }

          literal::name name;

          for ( place_vec_type::iterator place (places.begin())
              ; place != places.end()
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

        for ( function_vec_type::const_iterator pos (n.functions.begin())
            ; pos != n.functions.end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level(n.level) << "places =" << std::endl;

        for ( place_vec_type::const_iterator pos (n.places.begin())
            ; pos != n.places.end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level(n.level) << "transitions =" << std::endl;

        for ( transition_vec_type::const_iterator pos (n.transitions.begin())
            ; pos != n.transitions.end()
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
