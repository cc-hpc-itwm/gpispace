// Copyright (C) 2010-2016,2019-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/type/connect.hpp>
#include <gspc/xml/parse/type/eureka.hpp>
#include <gspc/xml/parse/type/function.hpp>
#include <gspc/xml/parse/type/net.fwd.hpp>
#include <gspc/xml/parse/type/place_map.hpp>
#include <gspc/xml/parse/type/require.hpp>
#include <gspc/xml/parse/type/response.hpp>
#include <gspc/xml/parse/type/struct.hpp>
#include <gspc/xml/parse/type/use.hpp>
#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/we/type/net.fwd.hpp>

#include <optional>



    namespace gspc::xml::parse::type
    {
      struct transition_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::string;

        using connections_type = fhgpnet::util::unique<connect_type>;
        using responses_type = fhgpnet::util::unique<response_type>;
        using eurekas_type = fhgpnet::util::unique<eureka_type>;
        using place_maps_type = fhgpnet::util::unique<place_map_type>;

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
                        , std::optional<::gspc::we::priority_type> const& priority
                        , std::optional<bool> const& finline
                        , std::optional<bool> const& produce_shared
                        , std::optional<bool> const& consume_shared
                        , std::optional<bool> const& passthrough_shared
                        , ::gspc::we::type::property::type const& properties
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
                     , gspc::xml::parse::structure_type_util::forbidden_type const& forbidden
                     );
        void resolve ( gspc::xml::parse::structure_type_util::set_type const& global
                     , state::type const& state
                     , gspc::xml::parse::structure_type_util::forbidden_type const& forbidden
                     );

        // ***************************************************************** //

        void specialize ( type::type_map_type const& map
                        , type::type_get_type const& get
                        , gspc::xml::parse::structure_type_util::set_type const& known_structs
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
          (std::unordered_map<std::string, gspc::pnet::type::signature::signature_type> known);

        ::gspc::we::type::property::type const& properties() const;
        ::gspc::we::type::property::type& properties();

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

        bool is_connect_tp_many (::gspc::we::edge::type, std::string const&) const;
        //! \todo All below should be private with accessors.
      public:
        structs_type structs;

      private:
        conditions_type _conditions;

      public:
        requirements_type requirements;

        std::optional<::gspc::we::priority_type> priority;
        std::optional<bool> finline;

        // Shared value tracking annotations.
        // When set, override auto-detection from port types.
        std::optional<bool> produce_shared;
        std::optional<bool> consume_shared;
        std::optional<bool> passthrough_shared;

      private:
        ::gspc::we::type::property::type _properties;
      };

      // ******************************************************************* //

      void transition_synthesize
        ( transition_type const&
        , state::type const& state
        , ::gspc::we::type::net_type& we_net
        , place_map_map_type const& pids
        );

      // ******************************************************************* //

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
                  , transition_type const& t
                  );
      }
    }
