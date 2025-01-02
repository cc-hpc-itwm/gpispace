// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/eureka.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/require.hpp>
#include <xml/parse/type/response.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/use.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <we/type/net.fwd.hpp>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct transition_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::string;

        using connections_type = fhg::pnet::util::unique<connect_type>;
        using responses_type = fhg::pnet::util::unique<response_type>;
        using eurekas_type = fhg::pnet::util::unique<eureka_type>;
        using place_maps_type = fhg::pnet::util::unique<place_map_type>;

        using function_or_use_type = std::variant<function_type, use_type>;

        transition_type ( util::position_type const&
                        , function_or_use_type const&
                        , std::string const& name
                        , connections_type const& connections
                        , responses_type const&
                        , eurekas_type const&
                        , place_maps_type const& place_map
                        , structs_type const& structs
                        , conditions_type const&
                        , requirements_type const& requirements
                        , ::boost::optional<we::priority_type> const& priority
                        , ::boost::optional<bool> const& finline
                        , we::type::property::type const& properties
                        );
        transition_type add_prefix (std::string const&) const;
        transition_type remove_prefix (std::string const&) const;

        function_or_use_type const& function_or_use() const;
        function_or_use_type& function_or_use();

        function_type const& resolved_function() const;

        std::string const& name() const;

        connections_type const& connections() const;
        responses_type const& responses() const;
        eurekas_type const& eurekas() const;
        place_maps_type const& place_map() const;

        // ***************************************************************** //

        conditions_type const& conditions() const;

        // ***************************************************************** //

        void resolve ( state::type const& state
                     , xml::parse::structure_type_util::forbidden_type const& forbidden
                     );
        void resolve ( xml::parse::structure_type_util::set_type const& global
                     , state::type const& state
                     , xml::parse::structure_type_util::forbidden_type const& forbidden
                     );

        // ***************************************************************** //

        void specialize ( type::type_map_type const& map
                        , type::type_get_type const& get
                        , xml::parse::structure_type_util::set_type const& known_structs
                        , state::type & state
                        );

        // ***************************************************************** //

        void type_check (response_type const&, state::type const&) const;
        void type_check (connect_type const&, state::type const&, net_type const& parent) const;
        void type_check (state::type const& state, net_type const& parent) const;
        void type_check (eureka_type const&, state::type const&) const;

        void resolve_function_use_recursive
          (std::unordered_map<std::string, function_type const&> known);
        void resolve_types_recursive
          (std::unordered_map<std::string, pnet::type::signature::signature_type> known);

        we::type::property::type const& properties() const;
        we::type::property::type& properties();

        unique_key_type const& unique_key() const;

      private:
        function_or_use_type _function_or_use;

        std::string const _name;

        //! \note renaming for prefix
        friend struct net_type;
        connections_type _connections;
        responses_type _responses;
        eurekas_type _eurekas;
        place_maps_type _place_map;

        bool is_connect_tp_many (we::edge::type, std::string const&) const;
        //! \todo All below should be private with accessors.
      public:
        structs_type structs;

      private:
        conditions_type _conditions;

      public:
        requirements_type requirements;

        ::boost::optional<we::priority_type> priority;
        ::boost::optional<bool> finline;

      private:
        we::type::property::type _properties;
      };

      // ******************************************************************* //

      void transition_synthesize
        ( transition_type const&
        , state::type const& state
        , we::type::net_type& we_net
        , place_map_map_type const& pids
        );

      // ******************************************************************* //

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , transition_type const& t
                  );
      }
    }
  }
}
