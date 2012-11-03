// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/net.hpp>

#include <xml/parse/type/specialize.hpp>
#include <xml/parse/id/mapper.hpp>

#include <fhg/util/remove_prefix.hpp>

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
        , _places (id_mapper)
        , _transitions (id_mapper)
        , _specializes (id_mapper)
        , _templates (id_mapper)
        , _functions (id_mapper)
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

      const net_type::places_type& net_type::places() const
      {
        return _places;
      }

      const net_type::transitions_type& net_type::transitions() const
      {
        return _transitions;
      }

      const net_type::specializes_type& net_type::specializes() const
      {
        return _specializes;
      }

      const net_type::templates_type& net_type::templates() const
      {
        return _templates;
      }

      const net_type::functions_type& net_type::functions() const
      {
        return _functions;
      }

      boost::optional<const id::ref::function&>
      net_type::get_function (const std::string& name) const
      {
        boost::optional<const id::ref::function&>
          id_function (functions().get (name));

        if (id_function)
          {
            return id_function;
          }
        else if (has_parent())
          {
            return parent()->get_function (name);
          }

        return boost::none;
      }

      //! \todo The logic should be like this: Just when a function is
      //! requested by some transition it is lookup up recursively up
      //! the tree and on each level, it is checked
      //! 1. if there is such a function known, then take it
      //! 2. if there is a specialize, then
      //!    2.1 search for the template and do the specialization
      //! So this would be a lazy specialization.
      //! The shadowing could be done by going further up the tree
      boost::optional<const id::ref::tmpl&>
      net_type::get_template (const std::string & name) const
      {
        boost::optional<const id::ref::tmpl&>
          id_tmpl (templates().get (name));

        if (id_tmpl)
          {
            return id_tmpl;
          }
        else if (has_parent())
          {
            std::cerr << "IMPLEMENT THE LOOKUP FOR TEMPLATES IN FUNCTION" << std::endl;
            // return parent()->get_template (name);
          }

        return boost::none;
      }

      // ***************************************************************** //

      const id::ref::place&
      net_type::push_place (const id::ref::place& id)
      {
        const id::ref::place& id_old (_places.push (id));

        if (not (id_old == id))
        {
          throw error::duplicate_place (id_mapper()->get (id)->name(), path());
        }

        return id;
      }

      const id::ref::transition&
      net_type::push_transition (const id::ref::transition& id)
      {
        const id::ref::transition& id_old (_transitions.push (id));

        if (not (id_old == id))
        {
          throw error::duplicate_transition<transition_type>
            (*id_mapper()->get (id), *id_mapper()->get (id_old));
        }

        return id;
      }

      const id::ref::specialize&
      net_type::push_specialize (const id::ref::specialize& id)
      {
        const id::ref::specialize& id_old (_specializes.push (id));

        if (not (id_old == id))
        {
          boost::optional<const specialize_type&>
            spec (id_mapper()->get (id));

          throw error::duplicate_specialize (spec->name(), spec->path);
        }

        return id;
      }

      const id::ref::tmpl&
      net_type::push_template (const id::ref::tmpl& id)
      {
        const id::ref::tmpl& id_old (_templates.push (id));

        if (not (id_old == id))
          {
            throw error::duplicate_template<tmpl_type>
              (*id_mapper()->get (id), *id_mapper()->get (id_old));
          }

        return id;
      }


      const id::ref::function&
      net_type::push_function (const id::ref::function& id)
      {
        const id::ref::function& id_old (_functions.push (id));

        if (not (id_old == id))
          {
            throw error::duplicate_function<function_type>
              (*id_mapper()->get (id), *id_mapper()->get (id_old));
          }

        return id;
      }

      // ***************************************************************** //

      void net_type::clear_places (void)
      {
        _places.clear();
      }

      void net_type::clear_transitions (void)
      {
        _transitions.clear();
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

        BOOST_FOREACH( const id::ref::specialize& id_specialize
                     , specializes().ids()
                     )
        {
          boost::optional<specialize_type&>
            specialize (id_mapper()->get_ref (id_specialize));

          boost::optional<const id::ref::tmpl&> id_tmpl
            (get_template (specialize->use));

          if (not id_tmpl)
          {
            throw error::unknown_template (specialize->use, path());
          }

          //! \todo generate a new function, with a state.next_id and
          //! the parent that is the transition that requires the
          //! specialization -> needs lazy specialization

          type_map_apply (map, specialize->type_map);

          id_mapper()->get_ref (*id_tmpl)->specialize
            ( specialize->type_map
            , specialize->type_get
            , st::join (known_structs, st::make (structs), state)
            , state
            );

          split_structs ( known_structs
                        , id_mapper()->get_ref (*id_tmpl)->function()->structs
                        , structs
                        , specialize->type_get
                        , state
                        );

          id_mapper()->get_ref (*id_tmpl)->function()->name (specialize->name());

          push_function ( id::ref::function
                          ( id_mapper()->get_ref (*id_tmpl)->function()->id()
                          , id_mapper()
                          )
                        );
        }

        _specializes.clear();


        BOOST_FOREACH ( const id::ref::function& id_function
                      , functions().ids()
                      )
        {
          id_mapper()->get_ref (id_function)->specialize
            ( map
            , get
            , st::join (known_structs, st::make (structs), state)
            , state
            );

          split_structs ( known_structs
                        , id_mapper()->get_ref (id_function)->structs
                        , structs
                        , get
                        , state
                        );
        }

        BOOST_FOREACH ( const id::ref::transition& id_transition
                      , transitions().ids()
                      )
        {
          boost::optional<transition_type&>
            transition (id_mapper()->get_ref (id_transition));

          transition->specialize
            ( map
            , get
            , st::join (known_structs, st::make (structs), state)
            , state
            );

          split_structs ( known_structs
                        , transition->structs
                        , structs
                        , get
                        , state
                        );
        }

        BOOST_FOREACH (const id::ref::place& id_place, places().ids())
        {
          boost::optional<place_type&> place (id_mapper()->get_ref (id_place));

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

        BOOST_FOREACH ( const id::ref::function& id_function
                      , functions().ids()
                      )
        {
          id_mapper()->get_ref (id_function)
            ->resolve (structs_resolved, state, st::forbidden_type());
        }

        BOOST_FOREACH ( const id::ref::transition& id_transition
                      , transitions().ids()
                      )
        {
          id_mapper()->get_ref(id_transition)
            ->resolve (structs_resolved, state, st::forbidden_type());
        }

        BOOST_FOREACH(id::ref::place id_place, places().ids())
        {
          id_mapper()->get_ref (id_place)->sig
            = type_of_place (*id_mapper()->get (id_place));
          id_mapper()->get_ref (id_place)->translate (path(), state);
        }
      }

      // ***************************************************************** //

      void net_type::sanity_check (const state::type & state, const function_type& outerfun) const
      {
        BOOST_FOREACH ( const id::ref::transition& id_transition
                      , transitions().ids()
                      )
        {
          id_mapper()->get (id_transition)->sanity_check (state);
        }

        BOOST_FOREACH ( const id::ref::function& id_function
                      , functions().ids()
                      )
        {
          id_mapper()->get (id_function)->sanity_check (state);
        }

        BOOST_FOREACH (const id::ref::place& id_place, places().ids())
        {
          boost::optional<const place_type&>
            place (id_mapper()->get (id_place));

          if (place->is_virtual() && !outerfun.is_known_tunnel (place->name()))
          {
            state.warn
              ( warning::virtual_place_not_tunneled ( place->name()
                                                    , outerfun.path
                                                    )
              );
          }
        }
      }

      // ***************************************************************** //

      void net_type::type_check (const state::type & state) const
      {
        BOOST_FOREACH ( const id::ref::transition& id_transition
                      , transitions().ids()
                      )
        {
          id_mapper()->get (id_transition)->type_check (*this, state);
        }

        BOOST_FOREACH ( const id::ref::function& id_function
                      , functions().ids()
                      )
        {
          id_mapper()->get (id_function)->type_check (state);
        }
      }

      // ******************************************************************* //

      void net_type::set_prefix (const std::string & prefix)
      {
        BOOST_FOREACH(id::ref::place id_place, places().ids())
        {
          id_mapper()->get_ref (id_place)
            ->name (prefix + id_mapper()->get(id_place)->name());
        }

        BOOST_FOREACH ( const id::ref::transition& id_transition
                      , transitions().ids()
                      )
        {
          boost::optional<transition_type&>
            transition (id_mapper()->get_ref (id_transition));

          transition->name (prefix + transition->name());

          BOOST_FOREACH (connect_type& connection, transition->in())
            {
              connection.place (prefix + connection.place());
            }

          BOOST_FOREACH (connect_type& connection, transition->read())
            {
              connection.place (prefix + connection.place());
            }

          BOOST_FOREACH (connect_type& connection, transition->out())
            {
              connection.place (prefix + connection.place());
            }

          BOOST_FOREACH (place_map_type& place_map, transition->place_map())
            {
              place_map.place_real = prefix + place_map.place_real;
            }
        }
      }

      void net_type::remove_prefix (const std::string & prefix)
      {
        BOOST_FOREACH (const id::ref::place& id_place, places().ids())
        {
          id_mapper()->get_ref (id_place)
            ->name ( fhg::util::remove_prefix
                     ( prefix
                     , id_mapper()->get (id_place)->name()
                     )
                   );
        }

        BOOST_FOREACH ( const id::ref::transition& id_transition
                      , transitions().ids()
                      )
        {
          boost::optional<transition_type&>
            transition (id_mapper()->get_ref (id_transition));

          transition->name
            (fhg::util::remove_prefix (prefix, transition->name()));

          for ( connections_type::iterator
                  connection (transition->in().begin())
              ; connection != transition->in().end()
              ; ++connection
              )
          {
            connection->place
              (fhg::util::remove_prefix (prefix, connection->place()));
          }

          for ( connections_type::iterator
                  connection (transition->read().begin())
              ; connection != transition->read().end()
              ; ++connection
              )
          {
            connection->place
              (fhg::util::remove_prefix (prefix, connection->place()));
          }

          for ( connections_type::iterator
                  connection (transition->out().begin())
              ; connection != transition->out().end()
              ; ++connection
              )
          {
            connection->place
              (fhg::util::remove_prefix (prefix, connection->place()));
          }

          for ( place_maps_type::iterator
                  place_map (transition->place_map().begin())
              ; place_map != transition->place_map().end()
              ; ++place_map
              )
          {
            place_map->place_real =
              fhg::util::remove_prefix (prefix, place_map->place_real);
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

        BOOST_FOREACH (const id::ref::place& id_place, net.places().ids())
            {
              boost::optional<const place_type&>
                place (net.id_mapper()->get (id_place));

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

                  pid_of_place.insert (std::make_pair ( place->name()
                                                      , pid->second
                                                      )
                                      );

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

                  pid_of_place.insert (std::make_pair (place->name(), pid));
                }
            }

        BOOST_FOREACH ( const id::ref::transition& id_transition
                      , net.transitions().ids()
                      )
          {
            transition_synthesize
              ( id_transition.id()
              , state
              , net
              , we_net
              , pid_of_place
              , e
              );
          }

        BOOST_FOREACH (const id::ref::place& id_place, net.places().ids())
          {
            boost::optional<const place_type&>
              place (net.id_mapper()->get (id_place));

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
          dumps (s, net.templates().ids(), net.id_mapper());
          dumps (s, net.specializes().ids(), net.id_mapper());
          dumps (s, net.functions().ids(), net.id_mapper());
          dumps (s, net.places().ids(), net.id_mapper());
          dumps (s, net.transitions().ids(), net.id_mapper());

          s.close ();
        }
      } // namespace dump
    }
  }
}
