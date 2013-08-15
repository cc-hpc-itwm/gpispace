// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/net.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/id/mapper.hpp>
#include <xml/parse/util/weparse.hpp>

#include <we/type/net.fwd.hpp>
#include <we/type/place.hpp>
#include <we/type/error.hpp>

#include <we/expr/eval/context.hpp>

#include <we2/require_type.hpp>

#include <fhg/util/remove_prefix.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      net_type::net_type ( ID_CONS_PARAM(net)
                         , PARENT_CONS_PARAM(function)
                         , const util::position_type& pod
                         )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
      {
        _id_mapper->put (_id, *this);
      }

      net_type::net_type ( ID_CONS_PARAM(net)
                         , PARENT_CONS_PARAM(function)
                         , const util::position_type& pod
                         , const functions_type& functions
                         , const places_type& places
                         , const specializes_type& specializes
                         , const templates_type& templates
                         , const transitions_type& transitions
                         , const structs_type& structs
                         , const bool& contains_a_module_call
                         , const we::type::property::type& properties
                         )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _functions (functions, _id)
        , _places (places, _id)
        , _specializes (specializes, _id)
        , _templates (templates, _id)
        , _transitions (transitions, _id)
        , structs (structs)
        , contains_a_module_call (contains_a_module_call)
        , _properties (properties)
      {
        _id_mapper->put (_id, *this);
      }

      const we::type::property::type& net_type::properties() const
      {
        return _properties;
      }
      we::type::property::type& net_type::properties()
      {
        return _properties;
      }

      // ***************************************************************** //

      const net_type::functions_type& net_type::functions() const
      {
        return _functions;
      }
      const net_type::places_type& net_type::places() const
      {
        return _places;
      }
      const net_type::specializes_type& net_type::specializes() const
      {
        return _specializes;
      }
      const net_type::templates_type& net_type::templates() const
      {
        return _templates;
      }
      const net_type::transitions_type& net_type::transitions() const
      {
        return _transitions;
      }

      // ***************************************************************** //

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

        if (id_old != id)
        {
          throw error::duplicate_place (id_old, id);
        }

        id.get_ref().parent (_id);

        return id;
      }

      const id::ref::specialize&
      net_type::push_specialize (const id::ref::specialize& id)
      {
        const id::ref::specialize& id_old (_specializes.push (id));

        if (id_old != id)
        {
          throw error::duplicate_specialize (id_old, id);
        }

        id.get_ref().parent (_id);

        return id;
      }

      const id::ref::tmpl&
      net_type::push_template (const id::ref::tmpl& id)
      {
        const id::ref::tmpl& id_old (_templates.push (id));

        if (id_old != id)
          {
            throw error::duplicate_template (id_old, id);
          }

        id.get_ref().parent (_id);

        return id;
      }

      const id::ref::transition&
      net_type::push_transition (const id::ref::transition& id)
      {
        const id::ref::transition& id_old (_transitions.push (id));

        if (id_old != id)
        {
          throw error::duplicate_transition (id_old, id);
        }

        id.get_ref().parent (_id);

        return id;
      }

      // ***************************************************************** //

      bool net_type::has_function (const std::string& name) const
      {
        return _functions.has (name);
      }
      bool net_type::has_place (const std::string& name) const
      {
        return _places.has (name);
      }
      bool net_type::has_specialize (const std::string& name) const
      {
        return _specializes.has (name);
      }
      bool net_type::has_template (const std::string& name) const
      {
        return _templates.has (name);
      }
      bool net_type::has_transition (const std::string& name) const
      {
        return _transitions.has (name);
      }

      // ***************************************************************** //

      void net_type::erase_function (const id::ref::function& id)
      {
        _functions.erase (id);
      }
      void net_type::erase_place (const id::ref::place& id)
      {
        _places.erase (id);
      }
      void net_type::erase_specialize (const id::ref::specialize& id)
      {
        _specializes.erase (id);
      }
      void net_type::erase_template (const id::ref::tmpl& id)
      {
        _templates.erase (id);
      }
      void net_type::erase_transition (const id::ref::transition& id)
      {
        _transitions.erase (id);
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

      void net_type::rename ( const id::ref::function& function
                            , const std::string& name
                            )
      {
        if (function.get().name() == name)
        {
          return;
        }

        if (has_function (name))
        {
          throw std::runtime_error
            ("tried renaming function, but function with given name exists");
        }

        _functions.erase (function);
        function.get_ref().name_impl (name);
        _functions.push (function);
      }

      void net_type::rename ( const id::ref::place& place
                            , const std::string& name
                            )
      {
        if (place.get().name() == name)
        {
          return;
        }

        if (has_place (name))
        {
          throw std::runtime_error
            ("tried renaming place, but place with given name exists");
        }

        _places.erase (place);
        place.get_ref().name_impl (name);
        _places.push (place);
      }

      void net_type::rename ( const id::ref::specialize& specialize
                            , const std::string& name
                            )
      {
        if (specialize.get().name() == name)
        {
          return;
        }

        if (has_specialize (name))
        {
          throw std::runtime_error
            ("tried renaming specialize, but specialize with given name exists");
        }

        _specializes.erase (specialize);
        specialize.get_ref().name_impl (name);
        _specializes.push (specialize);
      }

      void net_type::rename ( const id::ref::tmpl& tmpl
                            , const std::string& name
                            )
      {
        if (tmpl.get().name() == name)
        {
          return;
        }

        if (has_template (name))
        {
          throw std::runtime_error
            ("tried renaming template, but template with given name exists");
        }

        _templates.erase (tmpl);
        tmpl.get_ref().name_impl (name);
        _templates.push (tmpl);
      }

      void net_type::rename ( const id::ref::transition& transition
                            , const std::string& name
                            )
      {
        if (transition.get().name() == name)
        {
          return;
        }

        if (has_transition (name))
        {
          throw std::runtime_error
            ("tried renaming transition, but transition with given name exists");
        }

        _transitions.erase (transition);
        transition.get_ref().name_impl (name);
        _transitions.push (transition);
      }

      // ***************************************************************** //

      boost::optional<signature::type>
      net_type::signature (const std::string& type) const
      {
        const structs_type::const_iterator pos
          ( std::find_if ( structs.begin()
                         , structs.end()
                         , boost::bind ( parse::structure_type_util::struct_by_name
                                       , type
                                       , _1
                                       )
                         )
          );

        if (pos != structs.end())
        {
          return signature::type
            ( pos->resolve_with_fun (boost::bind (&net_type::signature, *this, _1))
            , pos->name()
            );
        }

        if (has_parent())
        {
          return parent()->signature (type);
        }

        return boost::none;
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
                                , const xml::parse::structure_type_util::set_type & known_structs
                                , state::type & state
                                )
      {
        namespace st = xml::parse::structure_type_util;

        BOOST_FOREACH (specialize_type& specialize, specializes().values())
        {
          boost::optional<const id::ref::tmpl&> id_tmpl
            (get_template (specialize.use));

          if (not id_tmpl)
          {
            throw error::unknown_template
              (specialize.make_reference_id(), make_reference_id());
          }

          //! \todo generate a new function, with a state.next_id and
          //! the parent that is the transition that requires the
          //! specialization -> needs lazy specialization

          type_map_apply (map, specialize.type_map);

          const id::ref::function specialized_function
            (id_tmpl->get().function().get().clone (boost::none));

          specialized_function.get_ref().specialize
            ( specialize.type_map
            , specialize.type_get
            , st::join (known_structs, st::make (structs), state)
            , state
            );

          split_structs ( known_structs
                        , specialized_function.get_ref().structs
                        , structs
                        , specialize.type_get
                        , state
                        );

          specialized_function.get_ref().name (specialize.name());

          //! \todo remove this, for now it is safe to not check for
          //! duplicates since we check for duplicate specializes
          _functions.push (specialized_function);

          specialized_function.get_ref().parent (_id);
        }

        _specializes.clear();


        BOOST_FOREACH (function_type& function, functions().values())
        {
          function.specialize
            ( map
            , get
            , st::join (known_structs, st::make (structs), state)
            , state
            );

          split_structs ( known_structs
                        , function.structs
                        , structs
                        , get
                        , state
                        );
        }

        BOOST_FOREACH (transition_type& transition, transitions().values())
        {
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

        BOOST_FOREACH (place_type& place, places().values())
        {
          place.specialize (map, state);
        }

        BOOST_FOREACH (structure_type& s, structs)
        {
          s.specialize (map);
        }
      }

      // ***************************************************************** //

      void net_type::sanity_check (const state::type & state) const
      {
        assert (has_parent());

        BOOST_FOREACH (const transition_type& transition, transitions().values())
        {
          transition.sanity_check (state);
        }

        BOOST_FOREACH (const function_type& function, functions().values())
        {
          function.sanity_check (state);
        }

        const function_type& outer_function (*parent());

        BOOST_FOREACH (const place_type& place, places().values())
        {
          if ( place.is_virtual()
            && !outer_function.is_known_tunnel (place.name())
             )
          {
            state.warn
              ( warning::virtual_place_not_tunneled ( place.name()
                                                    , outer_function.position_of_definition().path()
                                                    )
              );
          }
        }
      }

      // ***************************************************************** //

      void net_type::type_check (const state::type & state) const
      {
        BOOST_FOREACH (const transition_type& trans, transitions().values())
        {
          trans.type_check (state);
        }

        BOOST_FOREACH (const function_type& function, functions().values())
        {
          function.type_check (state);
        }
      }

      // ******************************************************************* //

      void net_type::set_prefix (const std::string & prefix)
      {
        //! \note We need to copy out the ids from the unique, as we
        //! modify the unique and therefore break iteration.
        const boost::unordered_set<id::ref::place> place_ids (places().ids());
        const boost::unordered_set<id::ref::transition> transition_ids
          (transitions().ids());

        BOOST_FOREACH (const id::ref::place& place, place_ids)
        {
          place.get_ref().name (prefix + place.get().name());
        }

        BOOST_FOREACH (const id::ref::transition& id, transition_ids)
        {
          transition_type& transition (id.get_ref());
          const boost::unordered_set<id::ref::connect> connect_ids
            (transition.connections().ids());
          const boost::unordered_set<id::ref::place_map> place_map_ids
            (transition.place_map().ids());

          transition.name (prefix + transition.name());

          BOOST_FOREACH (const id::ref::connect& conn, connect_ids)
          {
            conn.get_ref().place (prefix + conn.get().place());
          }

          BOOST_FOREACH (const id::ref::place_map& pm, place_map_ids)
          {
            pm.get_ref().place_real (prefix + pm.get().place_real());
          }
        }
      }

      void net_type::remove_prefix (const std::string & prefix)
      {
        //! \note We need to copy out the ids from the unique, as we
        //! modify the unique and therefore break iteration.
        const boost::unordered_set<id::ref::place> place_ids (places().ids());
        const boost::unordered_set<id::ref::transition> transition_ids
          (transitions().ids());

        BOOST_FOREACH (const id::ref::place& place, place_ids)
        {
          place.get_ref().name
            (fhg::util::remove_prefix (prefix, place.get().name()));
        }

        BOOST_FOREACH (const id::ref::transition& id, transition_ids)
        {
          transition_type& transition (id.get_ref());
          const boost::unordered_set<id::ref::connect> connect_ids
            (transition.connections().ids());
          const boost::unordered_set<id::ref::place_map> place_map_ids
            (transition.place_map().ids());

          transition.name
            (fhg::util::remove_prefix (prefix, transition.name()));

          BOOST_FOREACH (const id::ref::connect& conn, connect_ids)
          {
            conn.get_ref().place
              (fhg::util::remove_prefix (prefix, conn.get().place()));
          }

          BOOST_FOREACH (const id::ref::place_map& pm, place_map_ids)
          {
            pm.get_ref().place_real
              (fhg::util::remove_prefix (prefix, pm.get().place_real()));
          }
        }
      }

      id::ref::net net_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return net_type
          ( new_id
          , new_mapper
          , parent
          , _position_of_definition
          , _functions.clone (function_type::make_parent (new_id), new_mapper)
          , _places.clone (new_id, new_mapper)
          , _specializes.clone (new_id, new_mapper)
          , _templates.clone (new_id, new_mapper)
          , _transitions.clone (new_id, new_mapper)
          , structs
          , contains_a_module_call
          , _properties
          ).make_reference_id();
      }

      // ******************************************************************* //

      boost::unordered_map<std::string, petri_net::place_id_type>
      net_synthesize ( petri_net::net& we_net
                     , const place_map_map_type & place_map_map
                     , const net_type& net
                     , const state::type & state
                     )
      {
        typedef boost::unordered_map< std::string
                                    , petri_net::place_id_type
                                    > pid_of_place_type;

        pid_of_place_type pid_of_place;

        BOOST_FOREACH (const place_type& place, net.places().values())
        {
          if (!state.synthesize_virtual_places() && place.is_virtual())
          {
            const pid_of_place_type::const_iterator pid
              (place_map_map.find (place.name()));

            if (pid == place_map_map.end())
            {
              throw error::no_map_for_virtual_place
                (place.name(), state.file_in_progress());
            }

            pid_of_place.insert (std::make_pair (place.name(), pid->second));

            const place::type place_real (we_net.get_place (pid->second));

            if (!(place_real.signature() == place.signature_or_throw()))
            {
              throw error::port_tunneled_type_error
                ( place.name()
                , place.signature_or_throw()
                , place_real.name()
                , place_real.signature()
                , state.file_in_progress()
                );
            }
          }
          else
          {
            we::type::property::type prop (place.properties());

            if (place.is_virtual())
            {
              prop.set ("virtual", "true");
            }

            const petri_net::place_id_type pid
              ( we_net.add_place ( place::type
                                   ( place.name()
                                   , place.signature_or_throw()
                                   , prop
                                   )
                                 )
              );

            pid_of_place.insert (std::make_pair (place.name(), pid));
          }
        }

        BOOST_FOREACH ( const id::ref::transition& id_transition
                      , net.transitions().ids()
                      )
          {
            transition_synthesize
              ( id_transition
              , state
              , we_net
              , pid_of_place
              );
          }

        BOOST_FOREACH (const place_type& place, net.places().values())
        {
          const petri_net::place_id_type pid (pid_of_place.at (place.name()));

          BOOST_FOREACH (const place_type::token_type& token, place.tokens)
          {
            we_net.put_value
              ( pid
              , pnet::require_type_relaxed
                ( util::generic_we_parse (token, "parse token").eval_all2()
                , place.signature_or_throw()
                , ""
                )
              );
          }

          if (  (we_net.in_to_place (pid).size() == 0)
             && (we_net.out_of_place (pid).size() == 0)
             && (!place.is_virtual())
             )
          {
            state.warn
              (warning::independent_place (place.name(), state.file_in_progress()));
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

          ::we::type::property::dump::dump (s, net.properties());

          dumps (s, net.structs.begin(), net.structs.end());
          dumps (s, net.templates().values());
          dumps (s, net.specializes().values());
          dumps (s, net.functions().values());
          dumps (s, net.places().values());
          dumps (s, net.transitions().values());

          s.close ();
        }
      } // namespace dump
    }
  }
}
