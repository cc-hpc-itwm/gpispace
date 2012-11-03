// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/net.hpp>

#include <xml/parse/type/specialize.hpp>
#include <xml/parse/id/mapper.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      function_with_mapping_type::function_with_mapping_type
        ( function_type& function
        , boost::optional<type_map_type&> type_map
        )
          : _function (function)
          , _type_map (type_map)
      { }

      function_type& function_with_mapping_type::function()
      {
        return _function;
      }
      boost::optional<type_map_type&> function_with_mapping_type::type_map()
      {
        return _type_map;
      }

      net_type::net_type ( ID_CONS_PARAM(net)
                         , const id::function& parent
                         , const boost::filesystem::path& path
                         )
        : ID_INITIALIZE()
        , _parent (parent)
        , _path (path)
      {
        _id_mapper->put (_id, *this);
      }

      bool net_type::has_parent() const
      {
        return _parent;
      }

      boost::optional<const function_type&> net_type::parent() const
      {
        return id_mapper()->get (_parent);
      }
      boost::optional<function_type&> net_type::parent()
      {
        return id_mapper()->get_ref (_parent);
      }

      const boost::filesystem::path& net_type::path () const
      {
        return _path;
      }

      // ***************************************************************** //

#ifdef BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND
      net_type & net_type::operator= (net_type const &rhs)
      {
        if (this != &rhs)
        {
          _places = rhs._places;
          _transitions = rhs._transitions;
          _functions = rhs._functions;
          _templates = rhs._templates;
          _specializes = rhs._specializes;
          _id = rhs._id;
          _parent = rhs._parent;

          contains_a_module_call = rhs.contains_a_module_call;
          structs = rhs.structs;
          path = rhs.path;
          prop = rhs.prop;
          structs_resolved = rhs.structs_resolved;
        }
        return *this;
      }
#endif // BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND

      // ***************************************************************** //

      bool net_type::has_place (const std::string& name) const
      {
        return _places.is_element (name);
      }

      boost::optional<place_type> net_type::get_place (const std::string & name) const
      {
        return _places.copy_by_key (name);
      }

      boost::optional<place_type> net_type::place_by_id (const id::place& id) const
      {
        return _places.copy_by_id (id);
      }

      bool net_type::has_transition (const std::string& name) const
      {
        return _by_name_transition.find (name) != _by_name_transition.end();
      }

      const boost::unordered_set<id::ref::transition>&
      net_type::ids_transition() const
      {
        return _ids_transition;
      }
      boost::unordered_set<id::ref::transition>&
      net_type::ids_transition()
      {
        return _ids_transition;
      }

      boost::optional<function_type> net_type::get_function (const std::string & name) const
      {
        boost::optional<function_type> mf
          (_functions.copy_by_key (maybe_string_type(name)));

        if (mf)
          {
            return mf;
          }
        else if (has_parent())
          {
            return parent()->get_function (name);
          }

        return boost::none;
      }

      boost::optional<tmpl_type> net_type::get_template (const std::string & name) const
      {
        return _templates.copy_by_key (name);
      }

      // ***************************************************************** //

      function_with_mapping_type net_type::get_function
        (const std::string & name)
      {
        boost::optional<function_type&>
          fun (_functions.ref_by_key (maybe_string_type (name)));

        if (fun)
        {
          return function_with_mapping_type (*fun);
        }

        boost::optional<specialize_type &> spec
          (_specializes.ref_by_key (name));

        if (spec)
        {
          boost::optional<tmpl_type&>
            spec_fun (_templates.ref_by_key (spec->use));

          if (spec_fun)
          {
            return function_with_mapping_type
              ( *(*spec_fun).function()
              , spec->type_map
              );
          }
        }

        throw std::runtime_error ("STRANGE: function " + name + " unknown");
      }

      // ***************************************************************** //

      net_type::places_type & net_type::places (void)
      {
        return _places;
      }
      const net_type::places_type & net_type::places (void) const
      {
        return _places;
      }

      const functions_type & net_type::functions (void) const
      {
        return _functions.elements();
      }

      const specializes_type & net_type::specializes (void) const
      {
        return _specializes.elements();
      }

      const templates_type & net_type::templates (void) const
      {
        return _templates.elements();
      }

      // ***************************************************************** //

      void net_type::push_place (const place_type & p)
      {
        boost::optional<place_type&> place (_places.push (p));

        if (!place)
        {
          throw error::duplicate_place (p.name(), path());
        }
      }

      void net_type::erase_place (const place_type& t)
      {
        _places.erase (t);
      }

      const id::ref::transition&
      net_type::push_transition (const id::ref::transition& id_transition)
      {
        const transition_type& transition (*id_mapper()->get (id_transition));

        const boost::unordered_map<std::string,id::ref::transition>::const_iterator
          pos (_by_name_transition.find (transition.name()));

        if (pos != _by_name_transition.end())
        {
          throw error::duplicate_transition<transition_type>
            (transition, *id_mapper()->get (pos->second));
        }

        _ids_transition.insert (id_transition);
        _by_name_transition.insert (std::make_pair ( transition.name()
                                                   , id_transition
                                                   )
                                   );

        return id_transition;
      }

      void net_type::push_function (const function_type & f)
      {
        xml::util::unique<function_type,id::function>::push_return_type fun
          (_functions.push_and_get_old_value (f));

        if (!fun.first)
        {
          throw error::duplicate_function<function_type>
            (f, *fun.second);
        }
      }

      void net_type::push_template (const tmpl_type & t)
      {
        xml::util::unique<tmpl_type,id::tmpl>::push_return_type templ
          (_templates.push_and_get_old_value (t));

        if (!templ.first)
        {
          throw error::duplicate_template<tmpl_type>
            (t, *templ.second);
        }
      }

      void net_type::push_specialize ( const specialize_type & s
                                     , const state::type & state
                                     )
      {
        if (!_specializes.push (s))
        {
          throw error::duplicate_specialize ( s.name()
                                            , state.file_in_progress()
                                            );
        }
      }

      // ***************************************************************** //

      void net_type::clear_places (void)
      {
        _places.clear();
      }

      void net_type::clear_transitions (void)
      {
        _ids_transition.clear();
        _by_name_transition.clear();
      }

      // ***************************************************************** //

      signature::type net_type::type_of_place (const place_type & place) const
      {
        if (literal::valid_name (place.type))
        {
          return signature::type (place.type);
        }

        const xml::parse::structure_type::set_type::const_iterator sig
          (structs_resolved.find (place.type));

        if (sig == structs_resolved.end())
        {
          throw error::place_type_unknown (place.name(), place.type, path());
        }

        return signature::type (sig->second.signature(), sig->second.name());
      }

      // ***************************************************************** //

      void net_type::type_map_apply ( const type::type_map_type & outer_map
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

      void net_type::specialize ( const type::type_map_type & map
                                , const type::type_get_type & get
                                , const xml::parse::structure_type::set_type & known_structs
                                , state::type & state
                                )
      {
        namespace st = xml::parse::structure_type;

        for ( specializes_type::iterator
                specialize (_specializes.elements().begin())
            ; specialize != _specializes.elements().end()
            ; ++specialize
            )
        {
          boost::optional<tmpl_type> tmpl
            (get_template (specialize->use));

          if (!tmpl)
          {
            throw error::unknown_template (specialize->use, path());
          }

          //! \todo generate a new function, with a state.next_id and
          //! the parent that is the transition that requires the
          //! specialization -> needs lazy specialization

          type_map_apply (map, specialize->type_map);

          tmpl->specialize
            ( specialize->type_map
            , specialize->type_get
            , st::join (known_structs, st::make (structs), state)
            , state
            );

          split_structs ( known_structs
                        , tmpl->function()->structs
                        , structs
                        , specialize->type_get
                        , state
                        );

          tmpl->function()->name (specialize->name());

          push_function (*tmpl->function());
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

        for ( boost::unordered_set<id::ref::transition>::iterator
                id_transition (ids_transition().begin())
            ; id_transition != ids_transition().end()
            ; ++id_transition
            )
        {
          transition_type& transition (*id_mapper()->get_ref (*id_transition));

          transition.specialize
            ( map
            , get
            , st::join (known_structs, st::make (structs), state)
            , state
            );

          split_structs ( known_structs
                        , transition.structs
                        , structs
                        , get
                        , state
                        );
        }

        for ( places_type::iterator place (places().begin())
            ; place != places().end()
            ; ++place
            )
        {
          place->specialize (map, state);
        }

        specialize_structs (map, structs, state);
      }

      // ***************************************************************** //

      void net_type::resolve ( const state::type & state
                             , const xml::parse::structure_type::forbidden_type & forbidden
                             )
      {
        resolve (xml::parse::structure_type::set_type(), state, forbidden);
      }

      void net_type::resolve ( const xml::parse::structure_type::set_type & global
                             , const state::type & state
                             , const xml::parse::structure_type::forbidden_type & forbidden
                             )
      {
        namespace st = xml::parse::structure_type;

        structs_resolved =
          st::join (global, st::make (structs), forbidden, state);

        for ( st::set_type::iterator pos (structs_resolved.begin())
            ; pos != structs_resolved.end()
            ; ++pos
            )
        {
          boost::apply_visitor
            ( st::resolve (structs_resolved, pos->second.path())
            , pos->second.signature()
            );
        }

        for ( functions_type::iterator fun (_functions.elements().begin())
            ; fun != _functions.elements().end()
            ; ++fun
            )
        {
          fun->resolve (structs_resolved, state, st::forbidden_type());
        }

        for ( boost::unordered_set<id::ref::transition>::iterator
                id_transition (ids_transition().begin())
            ; id_transition != ids_transition().end()
            ; ++id_transition
            )
        {
          id_mapper()->get_ref(*id_transition)
            ->resolve (structs_resolved, state, st::forbidden_type());
        }

        for ( places_type::iterator place (places().begin())
            ; place != places().end()
            ; ++place
            )
        {
          place->sig = type_of_place (*place);
          place->translate (path(), state);
        }
      }

      // ***************************************************************** //

      void net_type::sanity_check (const state::type & state, const function_type& outerfun) const
      {
        for ( boost::unordered_set<id::ref::transition>::const_iterator
                id_transition (ids_transition().begin())
            ; id_transition != ids_transition().end()
            ; ++id_transition
            )
        {
          id_mapper()->get (*id_transition)->sanity_check (state);
        }

        for ( functions_type::const_iterator fun (functions().begin())
            ; fun != functions().end()
            ; ++fun
            )
        {
          fun->sanity_check (state);
        }

        BOOST_FOREACH (const place_type& place, places())
        {
          if (place.is_virtual() && !outerfun.is_known_tunnel (place.name()))
          {
            state.warn
              ( warning::virtual_place_not_tunneled ( place.name()
                                                    , outerfun.path
                                                    )
              );
          }
        }
      }

      // ***************************************************************** //

      void net_type::type_check (const state::type & state) const
      {
        for ( boost::unordered_set<id::ref::transition>::const_iterator
                id_transition (ids_transition().begin())
            ; id_transition != ids_transition().end()
            ; ++id_transition
            )
        {
          id_mapper()->get (*id_transition)->type_check (*this, state);
        }

        for ( functions_type::const_iterator fun (functions().begin())
            ; fun != functions().end()
            ; ++fun
            )
        {
          fun->type_check (state);
        }
      }

      // ******************************************************************* //

      void net_type::set_prefix (const std::string & prefix)
      {
        for ( places_type::iterator place (places().begin())
            ; place != places().end()
            ; ++place
            )
        {
          place->name (prefix + place->name());
        }

        for ( boost::unordered_set<id::ref::transition>::iterator
                id_transition (ids_transition().begin())
            ; id_transition != ids_transition().end()
            ; ++id_transition
            )
        {
          boost::optional<transition_type&>
            transition (id_mapper()->get_ref (*id_transition));

          transition->name (prefix + transition->name());

          for ( connections_type::iterator
                  connection (transition->in().begin())
              ; connection != transition->in().end()
              ; ++connection
              )
          {
            connection->place = prefix + connection->place;
          }

          for ( connections_type::iterator
                  connection (transition->read().begin())
              ; connection != transition->read().end()
              ; ++connection
              )
          {
            connection->place = prefix + connection->place;
          }

          for ( connections_type::iterator
                  connection (transition->out().begin())
              ; connection != transition->out().end()
              ; ++connection
              )
          {
            connection->place = prefix + connection->place;
          }

          for ( place_maps_type::iterator
                  place_map (transition->place_map().begin())
              ; place_map != transition->place_map().end()
              ; ++place_map
              )
          {
            place_map->place_real = prefix + place_map->place_real;
          }
        }
      }

      //! \note Does not assert that the names begin with prefix, but
      //! only trims them.
      void net_type::remove_prefix (const std::string & prefix)
      {
        const std::string::size_type prefix_length (prefix.size());

        for ( places_type::iterator place (places().begin())
            ; place != places().end()
            ; ++place
            )
        {
          place->name (place->name().substr (prefix_length));
        }

        for ( boost::unordered_set<id::ref::transition>::iterator
                id_transition (ids_transition().begin())
            ; id_transition != ids_transition().end()
            ; ++id_transition
            )
        {
          boost::optional<transition_type&>
            transition (id_mapper()->get_ref (*id_transition));

          transition->name (transition->name().substr (prefix_length));

          for ( connections_type::iterator
                  connection (transition->in().begin())
              ; connection != transition->in().end()
              ; ++connection
              )
          {
            connection->place = connection->place.substr (prefix_length);
          }

          for ( connections_type::iterator
                  connection (transition->read().begin())
              ; connection != transition->read().end()
              ; ++connection
              )
          {
            connection->place = connection->place.substr (prefix_length);
          }

          for ( connections_type::iterator
                  connection (transition->out().begin())
              ; connection != transition->out().end()
              ; ++connection
              )
          {
            connection->place = connection->place.substr (prefix_length);
          }

          for ( place_maps_type::iterator
                  place_map (transition->place_map().begin())
              ; place_map != transition->place_map().end()
              ; ++place_map
              )
          {
            place_map->place_real =
              place_map->place_real.substr (prefix_length);
          }
        }
      }

      // ******************************************************************* //

      boost::unordered_map< std::string
                          , we::activity_t::transition_type::pid_t
                          >
      net_synthesize ( we::activity_t::transition_type::net_type & we_net
                     , const place_map_map_type & place_map_map
                     , const net_type& net
                     , const state::type & state
                     , we::activity_t::transition_type::edge_type & e
                     )
      {
        typedef we::activity_t::transition_type we_transition_type;

        typedef we_transition_type::place_type we_place_type;
        typedef we_transition_type::edge_type we_edge_type;

        typedef we_transition_type::pid_t pid_t;

        typedef boost::unordered_map<std::string, pid_t> pid_of_place_type;

        pid_of_place_type pid_of_place;

        for ( net_type::places_type::const_iterator place (net.places().begin())
            ; place != net.places().end()
            ; ++place
            )
            {
              const signature::type type (net.type_of_place (*place));

              if (!state.synthesize_virtual_places() && place->is_virtual())
                {
                  // try to find a mapping
                  const place_map_map_type::const_iterator pid
                    (place_map_map.find (place->name()));

                  if (pid == place_map_map.end())
                    {
                      throw error::no_map_for_virtual_place
                        (place->name(), state.file_in_progress());
                    }

                  pid_of_place[place->name()] = pid->second;

                  const we_place_type place_real
                    (we_net.get_place (pid->second));

                  if (not (place_real.signature() == place->sig))
                    {
                      throw error::port_tunneled_type_error
                        ( place->name()
                        , place->sig
                        , place_real.name()
                        , place_real.signature()
                        , state.file_in_progress()
                        );
                    }
                }
              else
                {
                  we::type::property::type prop (place->prop);

                  if (place->is_virtual())
                    {
                      prop.set ("virtual", "true");
                    }

                  const pid_t pid
                    ( we_net.add_place ( we_place_type ( place->name()
                                                       , type
                                                       , prop
                                                       )
                                       )
                    );

                  pid_of_place[place->name()] = pid;
                }
            }

        for ( boost::unordered_set<id::ref::transition>::const_iterator
                id_transition (net.ids_transition().begin())
            ; id_transition != net.ids_transition().end()
            ; ++id_transition
            )
          {
            transition_synthesize
              ( *state.id_mapper()->get (*id_transition)
              , state
              , net
              , we_net
              , pid_of_place
              , e
              );
          }

          for ( net_type::places_type::const_iterator place (net.places().begin())
              ; place != net.places().end()
              ; ++place
              )
            {
              const pid_t pid (pid_of_place.at (place->name()));

              for ( values_type::const_iterator val (place->values.begin())
                  ; val != place->values.end()
                  ; ++val
                  )
                {
                  token::put (we_net, pid, *val);
                }

              if (  (we_net.in_to_place (pid).size() == 0)
                 && (we_net.out_of_place (pid).size() == 0)
                 && (!place->is_virtual())
                 )
                {
                  state.warn
                    ( warning::independent_place ( place->name()
                                                 , state.file_in_progress()
                                                 )
                    )
                    ;
                }
            }

          return pid_of_place;
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const net_type & net
                  )
        {
          s.open ("net");

          ::we::type::property::dump::dump (s, net.prop);

          dumps (s, net.structs.begin(), net.structs.end());
          dumps (s, net.templates().begin(), net.templates().end());
          dumps (s, net.specializes().begin(), net.specializes().end());
          dumps (s, net.functions().begin(), net.functions().end());
          dumps (s, net.places().begin(), net.places().end());

          dumps (s, net.ids_transition(), net.id_mapper());

          s.close ();
        }
      } // namespace dump
    }
  }
}
