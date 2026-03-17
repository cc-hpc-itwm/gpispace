// Copyright (C) 2012-2016,2018,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/net.hpp>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/type/specialize.hpp>
#include <gspc/xml/parse/util/weparse.hpp>

#include <gspc/we/type/net.fwd.hpp>
#include <gspc/we/type/place.hpp>
#include <gspc/we/type/shared.hpp>

#include <gspc/we/expr/eval/context.hpp>

#include <gspc/we/type/signature/resolve.hpp>

#include <gspc/we/type/value/name.hpp>

#include <gspc/util/remove_prefix.hpp>
#include <optional>



    namespace gspc::xml::parse::type
    {
      net_type::net_type ( util::position_type const& pod
                         , functions_type functions
                         , places_type places
                         , specializes_type specializes
                         , templates_type templates
                         , transitions_type transitions
                         , structs_type structs_
                         , ::gspc::we::type::property::type properties
                         )
        : with_position_of_definition (pod)
        , _functions (std::move (functions))
        , _places (std::move (places))
        , _specializes (std::move (specializes))
        , _templates (std::move (templates))
        , _transitions (std::move (transitions))
        , structs (std::move (structs_))
        , _properties (std::move (properties))
      {}

      ::gspc::we::type::property::type const& net_type::properties() const
      {
        return _properties;
      }

      // ***************************************************************** //

      net_type::functions_type const& net_type::functions() const
      {
        return _functions;
      }
      net_type::places_type const& net_type::places() const
      {
        return _places;
      }
      net_type::specializes_type const& net_type::specializes() const
      {
        return _specializes;
      }
      net_type::templates_type const& net_type::templates() const
      {
        return _templates;
      }
      net_type::transitions_type const& net_type::transitions() const
      {
        return _transitions;
      }
      net_type::transitions_type& net_type::transitions()
      {
        return _transitions;
      }

      // ***************************************************************** //

      //! \todo The logic should be like this: Just when a function is
      //! requested by some transition it is lookup up recursively up
      //! the tree and on each level, it is checked
      //! 1. if there is such a function known, then take it
      //! 2. if there is a specialize, then
      //!    2.1 search for the template and do the specialization
      //! So this would be a lazy specialization.
      //! The shadowing could be done by going further up the tree
      std::optional<std::reference_wrapper<tmpl_type const>>
      net_type::get_template (std::string const& name) const
      {
        //! \todo IMPLEMENT THE LOOKUP FOR TEMPLATES IN parent FUNCTION
        return templates().get (name);
      }

      // ***************************************************************** //

      void net_type::type_map_apply ( type::type_map_type const& outer_map
                                    , type::type_map_type & inner_map
                                    )
      {
        for (auto& inner : inner_map)
        {
          const type::type_map_type::const_iterator
            outer (outer_map.find (inner.second));

          if (outer != outer_map.end())
          {
            inner.second = outer->second;
          }
        }
      }

      void net_type::specialize ( type::type_map_type const& map
                                , type::type_get_type const& get
                                , gspc::xml::parse::structure_type_util::set_type const& known_structs
                                , state::type & state
                                )
      {
        namespace st = gspc::xml::parse::structure_type_util;

        for (specialize_type const& specialize : _specializes)
        {
          ::std::optional<std::reference_wrapper<tmpl_type const>> tmpl
            (get_template (specialize.use));

          if (not tmpl)
          {
            throw error::unknown_template (specialize, *this);
          }

          //! \todo generate a new function, with a state.next_id ->
          //! needs lazy specialization

          auto type_map (specialize.type_map);
          type_map_apply (map, type_map);

          function_type specialized_function
            (tmpl->get().function().with_name (specialize.name()));

          specialized_function.specialize
            ( type_map
            , specialize.type_get
            , st::join (known_structs, st::make (structs, state))
            , state
            );

          split_structs ( known_structs
                        , specialized_function.structs
                        , structs
                        , specialize.type_get
                        , state
                        );

          //! \todo remove this, for now it is safe to not check for
          //! duplicates since we check for duplicate specializes
          struct ignore_exception
          {
            ignore_exception (function_type const&, function_type const&) {}
          };
          _functions.push<ignore_exception> (specialized_function);
        }


        for (function_type& function : _functions)
        {
          function.specialize
            ( map
            , get
            , st::join (known_structs, st::make (structs, state))
            , state
            );

          split_structs ( known_structs
                        , function.structs
                        , structs
                        , get
                        , state
                        );
        }

        for (transition_type& transition : _transitions)
        {
          transition.specialize
            ( map
            , get
            , st::join (known_structs, st::make (structs, state))
            , state
            );

          split_structs ( known_structs
                        , transition.structs
                        , structs
                        , get
                        , state
                        );
        }

        auto const places (std::move (_places));
        for (place_type const& place : places)
        {
          _places.push<error::duplicate_place> (place.specialized (map, state));
        }

        for (structure_type& s : structs)
        {
          s.specialize (map);
        }
      }

      // ***************************************************************** //

      namespace
      {
        bool is_valid_generator_type (std::string const& type)
        {
          return type == "int"
              || type == "long"
              || type == "unsigned int"
              || type == "unsigned long"
              || type == "char"
              || type == "string"
              || type == "bigint";
        }
      }

      void net_type::type_check (state::type const& state) const
      {
        for (auto const& place : places())
        {
          if ( auto cleanup_place_name
               { ::gspc::we::type::shared::cleanup_place (place.type())
               }
             )
          {
            if (auto const cleanup_place {places().get (*cleanup_place_name)})
            {
              // For shared types, the cleanup place must be marked as sink
              // The actual value type compatibility is checked at runtime
              if (!cleanup_place->get().is_shared_sink())
              {
                throw error::shared_place_not_marked_as_sink
                  (place, *cleanup_place_name, cleanup_place->get());
              }
            }
            else
            {
              throw error::shared_place_does_not_exist
                (place, *cleanup_place_name);
            }
          }

          // Validate generator places
          if (place.is_generator())
          {
            // Generator places must not have tokens defined in XML
            if (!place.tokens.empty())
            {
              throw error::generator_place_with_tokens (place);
            }

            // Generator places must not have put_token="true"
            if (place.put_token().value_or (false))
            {
              throw error::generator_place_with_put_token (place);
            }

            // Generator places must not have shared_sink="true"
            if (place.is_shared_sink())
            {
              throw error::generator_place_with_shared_sink (place);
            }

            // Generator places must have one of the supported types
            if (!is_valid_generator_type (place.type()))
            {
              throw error::generator_place_invalid_type (place);
            }
          }
        }

        for (auto const& transition : transitions())
        {
          // Check for forbidden connections to generator places
          for (auto const& connection : transition.connections())
          {
            if ( auto place {places().get (connection.place())}
               ; place && place->get().is_generator()
               )
            {
              if ( std::holds_alternative<::gspc::we::edge::PT_READ>
                     ( connection.direction()
                     )
                 )
              {
                throw error::generator_place_with_read_connection
                  {place->get(), transition};
              }

              if ( std::holds_alternative<::gspc::we::edge::TP>
                     ( connection.direction()
                     )
                 )
              {
                throw error::generator_place_with_incoming_connection
                  {place->get(), transition};
              }

              if ( std::holds_alternative<::gspc::we::edge::TP_MANY>
                     ( connection.direction()
                     )
                 )
              {
                throw error::generator_place_with_incoming_connection
                  {place->get(), transition};
              }
            }
          }

          transition.type_check (state, *this);
        }

        for (function_type const& function : functions())
        {
          function.type_check (state);
        }
      }

      void net_type::resolve_function_use_recursive
        (std::unordered_map<std::string, function_type const&> known)
      {
        for (function_type const& function : functions())
        {
          if (!!function.name())
          {
            known.emplace (*function.name(), function);
          }
        }

        for (transition_type& transition : _transitions)
        {
          transition.resolve_function_use_recursive (known);
        }
        for (tmpl_type& templat : _templates)
        {
          templat.resolve_function_use_recursive (known);
        }
        for (function_type& function : _functions)
        {
          function.resolve_function_use_recursive (known);
        }
      }

      void net_type::resolve_types_recursive
        (std::unordered_map<std::string, gspc::pnet::type::signature::signature_type> known)
      {
        auto&& resolve
          ( [this, &known] (std::string name)
          -> std::optional<gspc::pnet::type::signature::signature_type>
            {
              auto const known_it (known.find (name));
              if (known_it != known.end())
              {
                return known_it->second;
              }

              auto const local_it
                ( std::find_if ( structs.begin(), structs.end()
                               , [&name] (structure_type const& struc)
                                 {
                                   return struc.name() == name;
                                 }
                               )
                );
              if (local_it != structs.end())
              {
                return gspc::pnet::type::signature::signature_type
                  (local_it->signature());
              }

              return {};
            }
          );

        for (structure_type const& struc : structs)
        {
          known.emplace
            ( struc.name()
            , gspc::pnet::type::signature::resolve (struc.signature(), resolve)
            );
        }

        for (transition_type& transition : _transitions)
        {
          transition.resolve_types_recursive (known);
        }
        for (function_type& function : _functions)
        {
          function.resolve_types_recursive (known);
        }

        for (place_type& place : _places)
        {
          place.resolve_types_recursive (known);
        }
      }

      // ******************************************************************* //

      void net_type::set_prefix (std::string const& prefix)
      {
        //! \note We need to copy out the ids from the unique, as we
        //! modify the unique and therefore break iteration.
        auto const places (std::move (_places));
        auto const transitions (std::move (_transitions));

        for (place_type const& place : places)
        {
          _places.push<error::duplicate_place>
            (place.with_name (prefix + place.name()));
        }

        for (transition_type const& old : transitions)
        {
          _transitions.push<error::duplicate_transition>
            (old.add_prefix (prefix));
        }
      }

      void net_type::remove_prefix (std::string const& prefix)
      {
        //! \note We need to copy out the ids from the unique, as we
        //! modify the unique and therefore break iteration.
        auto const places (std::move (_places));
        auto const transitions (std::move (_transitions));

        for (place_type const& place : places)
        {
          _places.push<error::duplicate_place>
            (place.with_name (gspc::util::remove_prefix (prefix, place.name())));
        }

        for (transition_type const& old : transitions)
        {
          _transitions.push<error::duplicate_transition>
            (old.remove_prefix (prefix));
        }
      }

      // ******************************************************************* //

      std::unordered_map<std::string, ::gspc::we::place_id_type>
      net_synthesize ( ::gspc::we::type::net_type& we_net
                     , place_map_map_type const& place_map_map
                     , net_type const& net
                     , state::type const& state
                     )
      {
        using pid_of_place_type = std::unordered_map<std::string, ::gspc::we::place_id_type>;

        pid_of_place_type pid_of_place;

        for (place_type const& place : net.places())
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

            pid_of_place.emplace (place.name(), pid->second);

            const gspc::we::type::place::type place_real (we_net.places().at (pid->second));

            if (!(place_real.signature() == place.signature()))
            {
              throw error::port_tunneled_type_error
                ( place.name()
                , place.signature()
                , place_real.name()
                , place_real.signature()
                , state.file_in_progress()
                );
            }
          }
          else
          {
            ::gspc::we::type::property::type prop (place.properties());

            if (place.is_virtual())
            {
              prop.set ({"virtual"}, true);
            }

            using Generator = gspc::we::type::place::type::Generator;
            using IsGenerator = std::variant<Generator::Yes, Generator::No>;
            const ::gspc::we::place_id_type pid
              ( we_net.add_place ( gspc::we::type::place::type
                                   ( place.name()
                                   , place.signature()
                                   , place.put_token()
                                   , place.shared_sink()
                                   , prop
                                   , place.is_generator()
                                     ? IsGenerator {Generator::Yes{}}
                                     : IsGenerator {Generator::No{}}
                                   )
                                 )
              );

            pid_of_place.emplace (place.name(), pid);
          }
        }

        for (transition_type const& transition : net.transitions())
          {
            transition_synthesize
              ( transition
              , state
              , we_net
              , pid_of_place
              );
          }

        for (place_type const& place : net.places())
        {
          const ::gspc::we::place_id_type pid (pid_of_place.at (place.name()));

          for (place_type::token_type const& token : place.tokens)
          {
            we_net.put_value
              (pid, util::generic_we_parse (token, "parse token").eval_all());
          }

          if (  ::boost::empty (we_net.transition_to_place().equal_range (pid))
             && ::boost::empty (we_net.place_to_transition_consume().left.equal_range (pid))
             && ::boost::empty (we_net.place_to_transition_read().left.equal_range (pid))
             && (!place.is_virtual())
             && (!place.is_shared_sink())
             )
          {
            state.warn
              (warning::independent_place (place.name(), state.file_in_progress()));
          }
        }

        return pid_of_place;
      }

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
                  , net_type const& net
                  )
        {
          s.open ("net");

          ::gspc::we::type::property::dump::dump (s, net.properties());

          dumps (s, net.structs.begin(), net.structs.end());
          dumps (s, net.templates());
          dumps (s, net.specializes());
          dumps (s, net.functions());
          dumps (s, net.places());
          dumps (s, net.transitions());

          s.close ();
        }
      }
    }
