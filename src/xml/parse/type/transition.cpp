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

#include <xml/parse/type/transition.hpp>

#include <xml/parse/state.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/use.hpp>
#include <xml/parse/util/property.hpp>
#include <xml/parse/util/weparse.hpp>

#include <xml/parse/type/dumps.hpp>

#include <we/type/value/name.hpp>
#include <we/type/Expression.hpp>
#include <we/workflow_response.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/remove_prefix.hpp>

#include <util-generic/cxx17/holds_alternative.hpp>

#include <boost/variant.hpp>

#include <xml/parse/rewrite/validprefix.hpp>

#include <algorithm>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      transition_type::transition_type
        ( util::position_type const& pod
        , function_or_use_type const& fun_or_use
        , std::string const& name
        , connections_type const& connections
        , responses_type const& responses
        , eurekas_type const& eurekas
        , place_maps_type const& place_map
        , structs_type const& structs_
        , conditions_type const& conditions
        , requirements_type const& requirements_
        , ::boost::optional<we::priority_type> const& priority_
        , ::boost::optional<bool> const& finline_
        , we::type::property::type const& properties
        )
        : with_position_of_definition (pod)
        , _function_or_use (fun_or_use)
        , _name (name)
        , _connections (connections)
        , _responses (responses)
        , _eurekas (eurekas)
        , _place_map (place_map)
        , structs (structs_)
        , _conditions (conditions)
        , requirements (requirements_)
        , priority (priority_)
        , finline (finline_)
        , _properties (properties)
      {}

      transition_type transition_type::add_prefix
        (std::string const& prefix) const
      {
        connections_type connections;

        for (connect_type const& connection : _connections)
        {
          connections.push<error::duplicate_connect>
            (connection.with_place (prefix + connection.place()));
        }

        place_maps_type place_map;

        for (place_map_type const& pm : _place_map)
        {
          place_map.push<error::duplicate_place_map>
            (pm.with_place_real (prefix + pm.place_real()));
        }

        return { position_of_definition()
               , _function_or_use
               , prefix + _name
               , connections
               , _responses
               , _eurekas
               , place_map
               , structs
               , _conditions
               , requirements
               , priority
               , finline
               , _properties
               };
      }
      transition_type transition_type::remove_prefix
        (std::string const& prefix) const
      {
        connections_type connections;

        for (connect_type const& connection : _connections)
        {
          connections.push<error::duplicate_connect>
            (connection.with_place
              (fhg::util::remove_prefix (prefix, connection.place()))
            );
        }

        place_maps_type place_map;

        for (place_map_type const& pm : _place_map)
        {
          place_map.push<error::duplicate_place_map>
            (pm.with_place_real
              (fhg::util::remove_prefix (prefix, pm.place_real()))
            );
        }

        return { position_of_definition()
               , _function_or_use
               , fhg::util::remove_prefix (prefix, _name)
               , connections
               , _responses
               , _eurekas
               , place_map
               , structs
               , _conditions
               , requirements
               , priority
               , finline
               , _properties
               };
      }

      transition_type::function_or_use_type const&
        transition_type::function_or_use() const
      {
        return _function_or_use;
      }
      transition_type::function_or_use_type&
        transition_type::function_or_use()
      {
        return _function_or_use;
      }

      function_type const& transition_type::resolved_function() const
      {
        //! \note assume post processing pass (resolve_function_use_recursive)
        return ::boost::get<function_type> (_function_or_use);
      }

      std::string const& transition_type::name() const
      {
        return _name;
      }

      transition_type::connections_type const&
        transition_type::connections() const
      {
        return _connections;
      }
      transition_type::responses_type const& transition_type::responses() const
      {
        return _responses;
      }
      transition_type::eurekas_type const& transition_type::eurekas() const
      {
        return _eurekas;
      }
      transition_type::place_maps_type const&
        transition_type::place_map() const
      {
        return _place_map;
      }

      // ***************************************************************** //

      conditions_type const& transition_type::conditions() const
      {
        return _conditions;
      }

      // ***************************************************************** //

      namespace
      {
        class transition_specialize : public ::boost::static_visitor<void>
        {
        private:
          type::type_map_type const& map;
          type::type_get_type const& get;
          xml::parse::structure_type_util::set_type const& known_structs;
          state::type & state;

        public:
          transition_specialize
            ( type::type_map_type const& _map
            , type::type_get_type const& _get
            , xml::parse::structure_type_util::set_type const& _known_structs
            , state::type & _state
            )
              : map (_map)
              , get (_get)
              , known_structs (_known_structs)
              , state (_state)
          {}

          void operator () (use_type const&) const { return; }
          void operator () (function_type& function) const
          {
            function.specialize (map, get, known_structs, state);
          }
        };
      }

      void transition_type::specialize ( type::type_map_type const& map
                                       , type::type_get_type const& get
                                       , xml::parse::structure_type_util::set_type const& known_structs
                                       , state::type & state
                                       )
      {
        ::boost::apply_visitor
          ( transition_specialize ( map
                                  , get
                                  , known_structs
                                  , state
                                  )
          , _function_or_use
          );
      }

      // ***************************************************************** //


      void transition_type::type_check ( response_type const& response
                                       , state::type const&
                                       ) const
      {
        auto const& ports (resolved_function().ports());

        if ( std::find_if
             ( std::begin (ports), std::end (ports)
             , [&response] (port_type const& port)
               {
                 return (  port.is_output()
                        && port.name() == response.port()
                        );
               }
             )
           == std::end (ports)
           )
        {
          throw error::unknown_port_in_connect_response (response);
        }

        auto const& to
          ( std::find_if
            ( std::begin (ports), std::end (ports)
            , [&response] (port_type const& port)
              {
                return (  port.is_input()
                       && port.name() == response.to()
                       );
              }
            )
          );

        if (to == std::end (ports))
        {
          throw error::unknown_to_in_connect_response (response);
        }

        if (!we::is_response_description (to->signature()))
        {
          throw error::invalid_signature_in_connect_response (response, *to);
        }
      }

      // ***************************************************************** //


      void transition_type::type_check ( eureka_type const& eureka
                                       , state::type const&
                                       ) const
      {
        auto const& ports (resolved_function().ports());

        auto const& eureka_port
          ( std::find_if
            ( std::begin (ports), std::end (ports)
            , [&eureka] (port_type const& port)
              {
                return (  port.is_output()
                       && port.name() == eureka.port()
                       );
              }
            )
          );

        if (eureka_port == std::end (ports))
        {
          throw error::connect_eureka_to_nonexistent_out_port ( *this
                                                              , eureka
                                                              );
        }
        else if (eureka_port->type() != pnet::type::value::SET())
        {
          throw error::eureka_port_type_mismatch (*this, eureka);
        }
      }

      // ***************************************************************** //

      bool transition_type::is_connect_tp_many ( we::edge::type direction
                                               , std::string const& port_type
                                               ) const
      {
        return (  fhg::util::cxx17::holds_alternative<we::edge::TP_MANY> (direction)
               && port_type == pnet::type::value::LIST()
               );
      }

      //! \todo move to connect_type
      void transition_type::type_check ( connect_type const& connect
                                       , state::type const&
                                       , net_type const& parent_net
                                       ) const
      {
        const ::boost::optional<place_type const&> place
          (parent_net.places().get (connect.place()));

        if (not place)
        {
          throw error::connect_to_nonexistent_place
            (*this, connect);
        }

        const ::boost::optional<port_type const&> port
          ( resolved_function().ports().get
              ( { connect.port()
                , ( we::edge::is_incoming (connect.direction())
                  ? we::type::PortDirection {we::type::port::direction::In{}}
                  : we::type::PortDirection {we::type::port::direction::Out{}}
                  )
                }
              )
          );

        if (not port)
        {
          throw error::connect_to_nonexistent_port
            (*this, connect);
        }

        if (  place->signature() != port->signature()
           && not is_connect_tp_many (connect.direction(), port->type())
           )
        {
          throw error::connect_type_error ( *this
                                          , connect
                                          , *port
                                          , *place
                                          );
        }
      }

      namespace
      {
        class transition_type_check : public ::boost::static_visitor<void>
        {
        private:
          state::type const& state;

        public:
          transition_type_check (state::type const& _state)
            : state (_state)
          { }

          void operator () (use_type const&) const { return; }
          void operator () (function_type const& function) const
          {
            function.type_check (state);
          }
        };
      }

      void transition_type::type_check
        (state::type const& state, net_type const& parent_net) const
      {
        for (connect_type const& connect : connections())
        {
          type_check (connect, state, parent_net);
        }
        for (response_type const& response : responses())
        {
          type_check (response, state);
        }
        for (eureka_type const& eureka : eurekas())
        {
          type_check (eureka, state);
        }

        ::boost::apply_visitor (transition_type_check (state), _function_or_use);
      }

      void transition_type::resolve_function_use_recursive
        (std::unordered_map<std::string, function_type const&> known)
      {
        if (!!::boost::get<use_type> (&_function_or_use))
        {
          std::string const name
            (::boost::get<use_type> (_function_or_use).name());
          auto const it (known.find (name));
          if (it == known.end())
          {
            throw error::unknown_function (name, *this);
          }
          _function_or_use = it->second;
        }
        else
        {
          ::boost::get<function_type> (_function_or_use)
            .resolve_function_use_recursive (known);
        }
      }

      void transition_type::resolve_types_recursive
        (std::unordered_map<std::string, pnet::type::signature::signature_type> known)
      {
        if (!!::boost::get<function_type> (&_function_or_use))
        {
          ::boost::get<function_type> (_function_or_use)
            .resolve_types_recursive (known);
        }
      }

      we::type::property::type const& transition_type::properties() const
      {
        return _properties;
      }
      we::type::property::type& transition_type::properties()
      {
        return _properties;
      }

      // ******************************************************************* //

      transition_type::unique_key_type const&
        transition_type::unique_key() const
      {
        return name();
      }

      // ******************************************************************* //

      namespace
      {
        place_map_map_type::mapped_type
        get_pid (place_map_map_type const& pid_of_place, std::string name)
        {
          const place_map_map_type::const_iterator pos (pid_of_place.find (name));

          if (pos == pid_of_place.end())
          {
            std::ostringstream s;
            s << __FILE__ << " [" << __LINE__ << "]: "
              << "missing place " << name << " in pid_of_place";
            throw error::strange (s.str());
          }

          return pos->second;
        }
      }

      void transition_synthesize
        ( transition_type const& trans
        , state::type const& state
        , we::type::net_type & we_net
        , place_map_map_type const& pids
        )
      {
        if (trans.connections().empty())
          {
            state.warn
              ( warning::independent_transition ( trans.name()
                                                , state.file_in_progress()
                                                )
              );
          }

        function_type const& fun (trans.resolved_function());

        for (port_type const& port_in : fun.ports())
        {
          if (port_in.is_input())
          {
            const ::boost::optional<port_type const&> port_out
              (fun.get_port_out (port_in.name()));

            if (  port_out
               && port_out->signature() != port_in.signature()
               )
            {
              state.warn
                ( warning::conflicting_port_types ( trans
                                                  , port_in
                                                  , *port_out
                                                  , state.file_in_progress()
                                                  )
                );
            }
          }
        }

        if (  fun.name()
           && (*fun.name() != trans.name())
           && (!rewrite::has_magic_prefix (trans.name()))
           )
        {
          state.warn (warning::overwrite_function_name_trans (trans, fun));
        }

        if (  not trans.priority // WORK HERE: make it work with prio
           && fun.is_net()
           && (
               (  !state.synthesize_virtual_places()
               && !trans.place_map().empty()
               )
               || (  !state.no_inline()
                  && trans.finline.get_value_or (false)
                  )
              )
           )
          { // unfold

            // set a prefix
            const std::string prefix (rewrite::mk_prefix (trans.name()));

            place_map_map_type place_map_map;

            for ( place_map_type const& place_map : trans.place_map())
              {
                const place_map_map_type::const_iterator pid
                  (pids.find (place_map.place_real()));

                if (pid == pids.end())
                  {
                    throw
                      error::real_place_missing ( place_map.place_virtual()
                                                , place_map.place_real()
                                                , trans.name()
                                                , state.file_in_progress()
                                                );
                  }

                place_map_map[prefix + place_map.place_virtual()] = pid->second;
              }

            //! \todo avoid copy by not modifying
            auto net (fun.net());
            net.set_prefix (prefix);

            // synthesize into this level
            const place_map_map_type pid_of_place
              (net_synthesize (we_net, place_map_map, net, state));

            net.remove_prefix (prefix);

            // go in the subnet
            const std::string cond_in
              ((fun.conditions() + trans.conditions()).flatten());

            expr::parse::parser parsed_condition_in
              ( util::we_parse ( cond_in
                               , "condition"
                               , "unfold"
                               , trans.name()
                               , trans.position_of_definition().path()
                               )
              );

            we::type::property::type properties (fun.properties());
            util::property::join (state, properties, trans.properties());

            //! \todo It seems like this should be getting the
            //! requirements of the inlined transition. Or all
            //! inlined transitions?
            we::type::Transition trans_in
              ( prefix + "IN"
              , we::type::Expression()
              , we::type::Expression (cond_in, parsed_condition_in)
              , properties
              , we::priority_type()
              , ::boost::none //! \todo eureka_id
              , {}          //! \todo preferences
              );

            {
              std::unordered_map<std::string, we::port_id_type>
                port_id_in;
              std::unordered_map<std::string, we::port_id_type>
                port_id_out;

              for (port_type const& port : fun.ports())
              {
                if (port.is_input())
                {
                  port_id_in.emplace
                    ( port.name()
                    , trans_in.add_port
                      ( we::type::Port ( port.name()
                                         , we::type::port::direction::In{}
                                         , port.signature()
                                         , port.properties()
                                         )
                      )
                    );

                  port_id_out.emplace
                    ( port.name()
                    , trans_in.add_port
                      ( we::type::Port ( port.name()
                                         , we::type::port::direction::Out{}
                                         , port.signature()
                                         , port.properties()
                                         )
                      )
                    );
                }
              }

              const we::transition_id_type tid_in
                (we_net.add_transition (trans_in));

              for (port_type const& port : fun.ports())
              {
                if (port.is_input() && port.place)
                {
                  we_net.add_connection
                    ( we::edge::TP{}
                    , tid_in
                    , get_pid (pid_of_place, prefix + *port.place)
                    , port_id_out.at (port.name())
                    , port.properties()
                    );
                }
              }

              for (connect_type const& connect : trans.connections())
              {
                if (we::edge::is_incoming (connect.direction()))
                {
                  we_net.add_connection
                    ( connect.direction()
                    , tid_in
                    , get_pid (pids, connect.place())
                    , port_id_in.at (connect.port())
                    , connect.properties()
                    );
                }
              }
            }

            // going out of the subnet
            we::type::Transition trans_out
              ( prefix + "OUT"
              , we::type::Expression()
              , ::boost::none
              , properties
              , we::priority_type()
              , ::boost::none //! \todo eureka_id
              , {}          //! \todo preferences
              );

            {
              std::unordered_map<std::string, we::port_id_type>
                port_id_in;
              std::unordered_map<std::string, we::port_id_type>
                port_id_out;

              for (port_type const& port : fun.ports())
              {
                if (port.is_output())
                {
                  port_id_in.emplace
                    ( port.name()
                    , trans_out.add_port
                      ( we::type::Port ( port.name()
                                         , we::type::port::direction::In{}
                                         , port.signature()
                                         , port.properties()
                                         )
                      )
                    );

                  port_id_out.emplace
                    ( port.name()
                    , trans_out.add_port
                      ( we::type::Port ( port.name()
                                         , we::type::port::direction::Out{}
                                         , port.signature()
                                         , port.properties()
                                         )
                      )
                    );
                }
              }

              std::size_t num_outport (0);

              const we::transition_id_type tid_out
                (we_net.add_transition (trans_out));

              for (port_type const& port : fun.ports())
              {
                if (port.is_output() && port.place)
                {
                  we_net.add_connection
                    ( we::edge::PT{}
                    , tid_out
                    , get_pid (pid_of_place, prefix + *port.place)
                    , port_id_in.at (port.name())
                    , port.properties()
                    );
                }
              }

              for (connect_type const& connect : trans.connections())
              {
                if (!we::edge::is_incoming (connect.direction()))
                {
                  we_net.add_connection
                    ( connect.direction()
                    , tid_out
                    , get_pid (pids, connect.place())
                    , port_id_out.at (connect.port())
                    , connect.properties()
                    );

                  ++num_outport;
                }
              }

              if (num_outport > 1)
              {
                if (properties.is_true ( { "pnetc"
                                         , "warning"
                                         , "inline_many_output_ports"
                                         }
                                       )
                  )
                {
                  state.warn ( warning::inline_many_output_ports
                             ( trans.name()
                             , state.file_in_progress()
                             )
                             );
                }
              }
            }

          } // unfold

        else

          { // not unfold

            std::unordered_map<std::string, we::port_id_type>
              port_id_in;
            std::unordered_map<std::string, we::port_id_type>
              port_id_out;
            std::unordered_map<we::port_id_type, std::string>
              real_place_names;

            we::type::Transition we_trans
              ( fun.synthesize ( trans.name()
                               , state
                               , port_id_in
                               , port_id_out
                               , trans.conditions()
                               , trans.properties()
                               , trans.requirements
                               , trans.priority
                               ? *trans.priority : we::priority_type()
                               , trans.place_map()
                               , real_place_names
                               )
              );

            const we::transition_id_type tid
              (we_net.add_transition (we_trans));

            for (connect_type const& connect : trans.connections())
            {
              if (we::edge::is_incoming (connect.direction()))
              {
                we_net.add_connection
                  ( connect.direction()
                  , tid
                  , get_pid (pids, connect.place())
                  , port_id_in.at (connect.port())
                  , connect.properties()
                  );
              }
              else
              {
                we_net.add_connection
                  ( connect.direction()
                  , tid
                  , get_pid (pids, connect.place())
                  , port_id_out.at (connect.port())
                  , connect.properties()
                  );
              }
            }

            for (response_type const& response : trans.responses())
            {
              we_net.add_response
                ( tid
                , port_id_out.at (response.port())
                , response.to()
                , response.properties()
                );
            }

            for (eureka_type const& eureka : trans.eurekas())
            {
              we_net.add_eureka
                ( tid
                , port_id_out.at (eureka.port())
                );
            }

            for (auto const& association : real_place_names)
            {
              we::type::property::type properties;

              properties.set ({"pnetc", "tunnel"}, we::type::property::value_type());

              we_net.add_connection
                ( we::edge::PT{}
                , tid
                , get_pid (pids, association.second)
                , association.first
                , properties
                );
            }
          } // not unfold

        return;
      }

      // ******************************************************************* //

      namespace dump
      {
        namespace
        {
          class dump_visitor : public ::boost::static_visitor<void>
          {
          private:
            ::fhg::util::xml::xmlstream & s;

          public:
            dump_visitor ( ::fhg::util::xml::xmlstream & _s)
              : s (_s)
            {}

            template<typename T>
              void operator () (T const& x) const
            {
              ::xml::parse::type::dump::dump (s, x);
            }
          };
        }

        void dump ( ::fhg::util::xml::xmlstream & s
                  , transition_type const& t
                  )
        {
          s.open ("transition");
          s.attr ("name", t.name());
          s.attr ("priority", t.priority);
          s.attr ("inline", t.finline);

          ::we::type::property::dump::dump (s, t.properties());
          ::xml::parse::type::dump::dump (s, t.requirements);

          ::boost::apply_visitor (dump_visitor (s), t.function_or_use());

          dumps (s, t.place_map());
          dumps (s, t.connections());
          dumps (s, t.responses());
          dumps (s, t.eurekas());

          for (std::string const& cond : t.conditions())
          {
            s.open ("condition");
            s.content (cond);
            s.close();
          }

          s.close ();
        }
      }
    }
  }
}
