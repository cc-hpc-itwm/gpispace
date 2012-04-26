// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_NET_HPP
#define _XML_PARSE_TYPE_NET_HPP

#include <xml/parse/types.hpp>
#include <xml/parse/state.hpp>

#include <xml/parse/util/unique.hpp>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>

#include <fhg/util/maybe.hpp>

#include <we/type/literal/valid_name.hpp>
#include <we/type/property.hpp>

#include <iostream>

#include <set>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      class function_with_mapping_type
      {
      private:
        function_type& _function;
        boost::optional<type_map_type&> _type_map;

      public:
        explicit function_with_mapping_type
        ( function_type& function
        , boost::optional<type_map_type&> type_map = boost::none
        )
          : _function (function)
          , _type_map (type_map)
        {}

        function_type& function() { return _function; }
        boost::optional<type_map_type&> type_map() { return _type_map; }
      };

      struct net_type
      {
      private:
        typedef boost::optional<std::string> maybe_string_type;

        xml::util::unique<place_type> _places;
        xml::util::unique<transition_type> _transitions;
        xml::util::unique<function_type,maybe_string_type, ::fhg::util::optional_hash<std::string> > _functions;
        xml::util::unique<function_type,maybe_string_type, ::fhg::util::optional_hash<std::string> > _templates;
        xml::util::unique<specialize_type> _specializes;

      public:
        typedef xml::util::unique<place_type>::elements_type places_type;
        typedef xml::util::unique<function_type>::elements_type templates_type;
        typedef xml::util::unique<transition_type>::elements_type transitions_type;
        typedef xml::util::unique<specialize_type>::elements_type specializes_type;

        bool contains_a_module_call;
        structs_type structs;

        boost::filesystem::path path;

        we::type::property::type prop;

        xml::parse::struct_t::set_type structs_resolved;

        // ***************************************************************** //

        bool get_place (const std::string & name, place_type & place) const
        {
          return _places.by_key (name, place);
        }

        bool get_function (const std::string & name, function_type & fun) const
        {
          return _functions.by_key (maybe_string_type(name), fun);
        }

        bool get_template (const std::string & name, function_type & tmpl) const
        {
          return _templates.by_key (maybe_string_type(name), tmpl);
        }

        // ***************************************************************** //

        function_with_mapping_type get_function (const std::string & name)
        {
          boost::optional<function_type&>
            fun (_functions.by_key (maybe_string_type (name)));

          if (fun)
            {
              return function_with_mapping_type (*fun);
            }

          boost::optional<specialize_type &> spec (_specializes.by_key (name));

          if (spec)
            {
              boost::optional<function_type&>
                spec_fun (_templates.by_key (maybe_string_type (spec->use)));

              if (spec_fun)
                {
                  return function_with_mapping_type (*spec_fun, spec->type_map);
                }
            }

          throw std::runtime_error ("function " + name + " unknown");
        }

        // ***************************************************************** //

        const places_type & places (void) const
        {
          return _places.elements();
        }

        const transitions_type & transitions (void) const
        {
          return _transitions.elements();
        }
        transitions_type & transitions (void)
        {
          return _transitions.elements();
        }

        const functions_type & functions (void) const
        {
          return _functions.elements();
        }

        const specializes_type & specializes (void) const
        {
          return _specializes.elements();
        }

        const templates_type & templates (void) const
        {
          return _templates.elements();
        }

        // ***************************************************************** //

        place_type& push_place (const place_type & p)
        {
          place_type old;
          boost::optional<place_type&> place (_places.push (p, old));

          if (!place)
            {
              throw error::duplicate_place (p.name, path);
            }

          return *place;
        }

        transition_type& push_transition (const transition_type & t)
        {
          transition_type old;
          boost::optional<transition_type&> trans (_transitions.push (t, old));

          if (!trans)
            {
              throw error::duplicate_transition<transition_type> (t, old);
            }

          return *trans;
        }

        void erase_transition (const transition_type& t)
        {
          _transitions.erase (t);
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

        void clear_places (void)
        {
          _places.clear();
        }

        void clear_transitions (void)
        {
          _transitions.clear();
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
                        , const xml::parse::struct_t::set_type & known_structs
                        , const state::type & state
                        )
        {
          namespace st = xml::parse::struct_t;

          for ( specializes_type::iterator
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

              tmpl.specialize
                ( specialize->type_map
                , specialize->type_get
                , st::join (known_structs, st::make (structs), state)
                , state
                );

              split_structs ( known_structs
                            , tmpl.structs
                            , structs
                            , specialize->type_get
                            , state
                            );

              tmpl.was_template = true;

              push_function (tmpl);
            }

          _specializes.clear();

          for ( functions_type::iterator fun (_functions.elements().begin())
              ; fun != _functions.elements().end()
              ; ++fun
              )
            {
              fun->specialize
                ( map
                , get
                , st::join (known_structs, st::make (structs), state)
                , state
                );

              split_structs ( known_structs
                            , fun->structs
                            , structs
                            , get
                            , state
                            );
            }

          for ( transitions_type::iterator
                  trans (_transitions.elements().begin())
              ; trans != _transitions.elements().end()
              ; ++trans
              )
            {
              trans->specialize
                ( map
                , get
                , st::join (known_structs, st::make (structs), state)
                , state
                );

              split_structs ( known_structs
                            , trans->structs
                            , structs
                            , get
                            , state
                            );
            }

          for ( places_type::iterator place (_places.elements().begin())
              ; place != _places.elements().end()
              ; ++place
              )
            {
              place->specialize (map, state);
            }

          specialize_structs (map, structs, state);
        }

        // ***************************************************************** //

        void distribute_function ( const state::type& state
                                 , const functions_type& functions_above
                                 )
        {
          functions_type funs (functions());

          BOOST_FOREACH (function_type& fun, funs)
            {
              fun.distribute_function (state, functions());
            }

          BOOST_FOREACH (const function_type& fun, functions_above)
            {
              function_type fun_local;

              if (!_functions.push (fun, fun_local))
                {
                  state.warn ( warning::shadow_function ( fun.name
                                                        , fun.path
                                                        , fun_local.path
                                                        )
                             );
                }
            }

          BOOST_FOREACH (transition_type& transition, transitions())
            {
              transition.distribute_function (state, functions());
            }
        }

        // ***************************************************************** //

        void resolve ( const state::type & state
                     , const xml::parse::struct_t::forbidden_type & forbidden
                     )
        {
          resolve (xml::parse::struct_t::set_type(), state, forbidden);
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

          for ( functions_type::iterator fun (_functions.elements().begin())
              ; fun != _functions.elements().end()
              ; ++fun
              )
            {
              fun->resolve (structs_resolved, state, st::forbidden_type());
            }

          for ( transitions_type::iterator
                  trans (_transitions.elements().begin())
              ; trans != _transitions.elements().end()
              ; ++trans
              )
            {
              trans->resolve (structs_resolved, state, st::forbidden_type());
            }

          for ( places_type::iterator place (_places.elements().begin())
              ; place != _places.elements().end()
              ; ++place
              )
            {
              place->sig = type_of_place (*place);
              place->translate (path, state);
            }
        }

        // ***************************************************************** //

        void sanity_check (const state::type & state) const
        {
          for ( transitions_type::const_iterator trans (transitions().begin())
              ; trans != transitions().end()
              ; ++trans
              )
            {
              trans->sanity_check (state);
            }

          for ( functions_type::const_iterator fun (functions().begin())
              ; fun != functions().end()
              ; ++fun
              )
            {
              fun->sanity_check (state);
            }
        }

        // ***************************************************************** //

        void type_check (const state::type & state) const
        {
          for ( transitions_type::const_iterator trans (transitions().begin())
              ; trans != transitions().end()
              ; ++trans
              )
            {
              trans->type_check<net_type> (*this, state);
            }

          for ( functions_type::const_iterator fun (functions().begin())
              ; fun != functions().end()
              ; ++fun
              )
            {
              fun->type_check (state);
            }
        }
      };

      // ******************************************************************* //

      inline net_type
      set_prefix (const net_type & net_old, const std::string & prefix)
      {
        net_type net_new (net_old);

        net_new.clear_places();
        net_new.clear_transitions();

        for ( net_type::places_type::const_iterator
                place_old (net_old.places().begin())
            ; place_old != net_old.places().end()
            ; ++place_old
            )
          {
            place_type place_new (*place_old);

            place_new.name = prefix + place_old->name;

            net_new.push_place (place_new);
          }

        for ( net_type::transitions_type::const_iterator
                transition_old (net_old.transitions().begin())
            ; transition_old != net_old.transitions().end()
            ; ++transition_old
            )
          {
            transition_type transition_new (*transition_old);
            transition_new.name = prefix + transition_old->name;

            transition_new.clear_ports();

            for (connections_type::const_iterator
                   connect_in_old (transition_old->in().begin())
                ; connect_in_old != transition_old->in().end()
                ; ++connect_in_old
                )
              {
                connect_type connect_in_new (*connect_in_old);

                connect_in_new.place = prefix + connect_in_old->place;

                transition_new.push_in (connect_in_new);
              }

            for (connections_type::const_iterator
                   connect_read_old (transition_old->read().begin())
                ; connect_read_old != transition_old->read().end()
                ; ++connect_read_old
                )
              {
                connect_type connect_read_new (*connect_read_old);

                connect_read_new.place = prefix + connect_read_old->place;

                transition_new.push_read (connect_read_new);
              }

            for (connections_type::const_iterator
                   connect_out_old (transition_old->out().begin())
                ; connect_out_old != transition_old->out().end()
                ; ++connect_out_old
                )
              {
                connect_type connect_out_new (*connect_out_old);

                connect_out_new.place = prefix + connect_out_old->place;

                transition_new.push_out (connect_out_new);
              }

            transition_new.clear_place_map();

            for (place_maps_type::const_iterator
                   place_map_old (transition_old->place_map().begin())
                ; place_map_old != transition_old->place_map().end()
                ; ++place_map_old
                )
              {
                place_map_type place_map_new (*place_map_old);

                place_map_new.place_real = prefix + place_map_old->place_real;

                transition_new.push_place_map (place_map_new);
              }

            net_new.push_transition (transition_new);
          }

        return net_new;
      }

      // ******************************************************************* //

      template <typename Activity, typename Net, typename Fun>
      boost::unordered_map< std::string
                          , typename Activity::transition_type::pid_t
                          >
      net_synthesize ( typename Activity::transition_type::net_type & we_net
                     , const place_map_map_type & place_map_map
                     , const Net & net
                     , const state::type & state
                     , typename Activity::transition_type::edge_type & e
                     )
      {
        typedef typename Activity::transition_type we_transition_type;

        typedef typename we_transition_type::place_type we_place_type;
        typedef typename we_transition_type::edge_type we_edge_type;

        typedef typename we_transition_type::pid_t pid_t;

        typedef boost::unordered_map<std::string, pid_t> pid_of_place_type;

        pid_of_place_type pid_of_place;

        for ( places_type::const_iterator place (net.places().begin())
            ; place != net.places().end()
            ; ++place
            )
            {
              const signature::type type (net.type_of_place (*place));

              if (  !state.synthesize_virtual_places()
                 && place->is_virtual.get_value_or (false)
                 )
                {
                  // try to find a mapping
                  const place_map_map_type::const_iterator pid
                    (place_map_map.find (place->name));

                  if (pid == place_map_map.end())
                    {
                      throw error::no_map_for_virtual_place
                        (place->name, state.file_in_progress());
                    }

                  pid_of_place[place->name] = pid->second;
                }
              else
                {
                  we::type::property::type prop (place->prop);

                  if (place->is_virtual.get_value_or (false))
                    {
                      prop.set ("virtual", "true");
                    }

                  const pid_t pid
                    ( we_net.add_place ( we_place_type ( place->name
                                                       , type
                                                       , prop
                                                       )
                                       )
                    );

                  if (place->capacity)
                    {
                      we_net.set_capacity (pid, *place->capacity);
                    }

                  pid_of_place[place->name] = pid;
                }
            }

          for ( typename Net::transitions_type::const_iterator transition
                  (net.transitions().begin())
              ; transition != net.transitions().end()
              ; ++transition
              )
            {
              transition_synthesize< Activity
                                   , Net
                                   , transition_type
                                   , Fun
                                   , pid_of_place_type
                                   >
                (*transition, state, net, we_net, pid_of_place, e);
            }

          for ( places_type::const_iterator place (net.places().begin())
              ; place != net.places().end()
              ; ++place
              )
            {
              const pid_t pid (pid_of_place.at (place->name));

              for ( values_type::const_iterator val (place->values.begin())
                  ; val != place->values.end()
                  ; ++val
                  )
                {
                  token::put (we_net, pid, *val);
                }

              if (  (we_net.in_to_place (pid).size() == 0)
                 && (we_net.out_of_place (pid).size() == 0)
                 && (!place->is_virtual.get_value_or (false))
                 )
                {
                  state.warn
                    ( warning::independent_place ( place->name
                                                 , state.file_in_progress()
                                                 )
                    )
                    ;
                }
            }

          return pid_of_place;
      };
    }
  }
}

#endif
