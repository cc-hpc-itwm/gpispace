// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <xml/parse/type/net.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/util/weparse.hpp>

#include <we/type/net.fwd.hpp>
#include <we/type/place.hpp>

#include <we/expr/eval/context.hpp>

#include <we/type/signature/resolve.hpp>

#include <fhg/util/remove_prefix.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      net_type::net_type ( util::position_type const& pod
                         , functions_type functions
                         , places_type places
                         , specializes_type specializes
                         , templates_type templates
                         , transitions_type transitions
                         , structs_type structs_
                         , we::type::property::type properties
                         )
        : with_position_of_definition (pod)
        , _functions (std::move (functions))
        , _places (std::move (places))
        , _specializes (std::move (specializes))
        , _templates (std::move (templates))
        , _transitions (std::move (transitions))
        , structs (std::move (structs_))
        , contains_a_module_call()
        , _properties (std::move (properties))
      {}

      we::type::property::type const& net_type::properties() const
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
      boost::optional<tmpl_type const&>
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

      void net_type::specialize ( type::type_map_type const& map
                                , type::type_get_type const& get
                                , xml::parse::structure_type_util::set_type const& known_structs
                                , state::type & state
                                )
      {
        namespace st = xml::parse::structure_type_util;

        for (specialize_type const& specialize : _specializes)
        {
          boost::optional<tmpl_type const&> tmpl
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
            (tmpl->function().with_name (specialize.name()));

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

      void net_type::type_check (state::type const& state) const
      {
        for (transition_type const& trans : transitions())
        {
          trans.type_check (state, *this);
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
        (std::unordered_map<std::string, pnet::type::signature::signature_type> known)
      {
        auto&& resolve
          ( [this, &known] (std::string name)
          -> boost::optional<pnet::type::signature::signature_type>
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
                return pnet::type::signature::signature_type
                  (local_it->signature());
              }

              return boost::none;
            }
          );

        for (structure_type const& struc : structs)
        {
          known.emplace
            ( struc.name()
            , pnet::type::signature::resolve (struc.signature(), resolve)
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
            (place.with_name (fhg::util::remove_prefix (prefix, place.name())));
        }

        for (transition_type const& old : transitions)
        {
          _transitions.push<error::duplicate_transition>
            (old.remove_prefix (prefix));
        }
      }

      // ******************************************************************* //

      std::unordered_map<std::string, we::place_id_type>
      net_synthesize ( we::type::net_type& we_net
                     , place_map_map_type const& place_map_map
                     , net_type const& net
                     , state::type const& state
                     )
      {
        typedef std::unordered_map< std::string
                                  , we::place_id_type
                                  > pid_of_place_type;

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

            const place::type place_real (we_net.places().at (pid->second));

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
            we::type::property::type prop (place.properties());

            if (place.is_virtual())
            {
              prop.set ({"virtual"}, true);
            }

            const we::place_id_type pid
              ( we_net.add_place ( place::type
                                   ( place.name()
                                   , place.signature()
                                   , place.put_token()
                                   , prop
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
          const we::place_id_type pid (pid_of_place.at (place.name()));

          for (place_type::token_type const& token : place.tokens)
          {
            we_net.put_value
              (pid, util::generic_we_parse (token, "parse token").eval_all());
          }

          if (  boost::empty (we_net.transition_to_place().equal_range (pid))
             && boost::empty (we_net.place_to_transition_consume().left.equal_range (pid))
             && boost::empty (we_net.place_to_transition_read().left.equal_range (pid))
             && (!place.is_virtual())
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
        void dump ( ::fhg::util::xml::xmlstream & s
                  , net_type const& net
                  )
        {
          s.open ("net");

          ::we::type::property::dump::dump (s, net.properties());

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
  }
}
