// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_NET_HPP
#define _XML_PARSE_TYPE_NET_HPP

#include <xml/parse/types.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/util/unique.hpp>

#include <vector>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

#include <we/type/literal/name.hpp>
#include <we/type/property.hpp>

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
        xml::util::unique<function_type,maybe<std::string> > _templates;
        xml::util::unique<specialize_type> _specializes;

      public:
        typedef std::vector<function_type> function_vec_type;
        typedef std::vector<function_type> template_vec_type;
        typedef std::vector<transition_type> transition_vec_type;
        typedef std::vector<specialize_type> specialize_vec_type;

        struct_vec_type structs;

        boost::filesystem::path path;

        we::type::property::type prop;

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

        bool get_template (const std::string & name, function_type & tmpl) const
        {
          return _templates.by_key (maybe<std::string>(name), tmpl);
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
        
        const specialize_vec_type & specializes (void) const
        {
          return _specializes.elements();
        }

        const template_vec_type & templates (void) const
        {
          return _templates.elements();
        }

        // ***************************************************************** //

        void push_place (const place_type & place)
        {
          place_type old;

          if (!_places.push (place, old))
            {
              throw error::duplicate_place (place.name, path);
            }
        }

        void push_transition (const transition_type & t)
        {
          transition_type old;

          if (!_transitions.push (t, old))
            {
              throw error::duplicate_transition<transition_type> (t, old);
            }
        }

        void push_function (const function_type & f)
        {
          function_type old;

          if (!_functions.push (f, old))
            {
              throw error::duplicate_function<function_type> (f, old);
            }
        }

        void push_template (const function_type & t)
        {
          function_type old;

          if (!_templates.push (t, old))
            {
              throw error::duplicate_template<function_type> (t, old);
            }
        }

        void push_specialize ( const specialize_type & s
                             , const state::type & state
                             )
        {
          if (!_specializes.push (s))
            {
              throw error::duplicate_specialize ( s.name
                                                , state.file_in_progress()
                                                );
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

        void type_map_apply ( const type::type_map_type & outer_map
                            , type::type_map_type & inner_map
                            )
        {
          for ( type_map_type::iterator inner (inner_map.begin())
              ; inner != inner_map.end()
              ; ++inner
              )
            {
              const type::type_map_type::const_iterator
                outer (outer_map.find (inner->second));

              if (outer != outer_map.end())
                {
                  inner->second = outer->second;
                }
            }
        }

        void specialize ( const type::type_map_type & map
                        , const type::type_get_type & get
                        , const state::type & state
                        )
        {
          for ( specialize_vec_type::iterator
                  specialize (_specializes.elements().begin())
              ; specialize != _specializes.elements().end()
              ; ++specialize
              )
            {
              function_type tmpl;
              
              if (!get_template (specialize->use, tmpl))
                {
                  throw error::unknown_template (specialize->use, path);
                }

              tmpl.name = specialize->name;

              type_map_apply (map, specialize->type_map);

              tmpl.specialize ( specialize->type_map
                              , specialize->type_get
                              , state
                              );

              split_structs ( tmpl.structs
                            , structs
                            , specialize->type_get
                            , state
                            );

              push_function (tmpl);
            }

          _specializes.clear();

          for ( function_vec_type::iterator fun (_functions.elements().begin())
              ; fun != _functions.elements().end()
              ; ++fun
              )
            {
              fun->specialize (map, get, state);

              split_structs (fun->structs, structs, get, state);
            }

          for ( transition_vec_type::iterator
                  trans (_transitions.elements().begin())
              ; trans != _transitions.elements().end()
              ; ++trans
              )
            {
              trans->specialize (map, get, state);

              split_structs (trans->structs, structs, get, state);
            }

          for ( place_vec_type::iterator place (_places.elements().begin())
              ; place != _places.elements().end()
              ; ++place
              )
            {
              place->specialize (map, state);
            }

          specialize_structs (map, structs, state);
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
        s << level(n.level) << "net" << std::endl;
        s << level(n.level+1) << "path = " << n.path << std::endl;
        s << level(n.level+1) << "properties = " << std::endl;

        n.prop.writeTo (s, n.level+2);

        s << level(n.level+1) << "structs =" << std::endl;

        for ( struct_vec_type::const_iterator pos (n.structs.begin())
            ; pos != n.structs.end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level (n.level+1) << "resolved structs = " << std::endl;

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

        s << level(n.level+1) << "specializes =" << std::endl;

        for ( net_type::specialize_vec_type::const_iterator pos
                (n.specializes().begin())
            ; pos != n.specializes().end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level(n.level+1) << "templates =" << std::endl;

        for ( net_type::template_vec_type::const_iterator pos
                (n.templates().begin())
            ; pos != n.templates().end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level(n.level+1) << "functions =" << std::endl;

        for ( net_type::function_vec_type::const_iterator pos
                (n.functions().begin())
            ; pos != n.functions().end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level(n.level+1) << "places =" << std::endl;

        for ( place_vec_type::const_iterator pos (n.places().begin())
            ; pos != n.places().end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level(n.level+1) << "transitions =" << std::endl;

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
