// Copyright (C) 2010-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

#include <gspc/xml/parse/type/net.fwd.hpp>

#include <gspc/xml/parse/type/function.hpp>
#include <gspc/xml/parse/type/place.hpp>
#include <gspc/xml/parse/type/place_map.hpp>
#include <gspc/xml/parse/type/specialize.hpp>
#include <gspc/xml/parse/type/template.hpp>
#include <gspc/xml/parse/type/transition.hpp>
#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/type_map_type.hpp>
#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/xml/parse/util/unique.hpp>

#include <gspc/xml/parse/type/dumps.hpp>

#include <gspc/we/type/id.hpp>



    namespace gspc::xml::parse::type
    {
      struct net_type : with_position_of_definition
      {
      public:
        using functions_type = fhgpnet::util::unique<function_type>;
        using places_type = fhgpnet::util::unique<place_type>;
        using specializes_type = fhgpnet::util::unique<specialize_type>;
        using templates_type = fhgpnet::util::unique<tmpl_type>;
        using transitions_type = fhgpnet::util::unique<transition_type>;

        net_type ( util::position_type const&
                 , functions_type
                 , places_type
                 , specializes_type
                 , templates_type
                 , transitions_type
                 , structs_type
                 , ::gspc::we::type::property::type
                 );

        ::gspc::we::type::property::type const& properties() const;

        // ***************************************************************** //

        functions_type const& functions() const;
        places_type const& places() const;
        specializes_type const& specializes() const;
        templates_type const& templates() const;
        transitions_type const& transitions() const;
        //! \note find_module_calls needs to modify transitions when diving
        transitions_type& transitions();

        // ***************************************************************** //

        std::optional<std::reference_wrapper<tmpl_type const>>
          get_template (std::string const& name) const;

        // ***************************************************************** //

        void type_map_apply ( type::type_map_type const& outer_map
                            , type::type_map_type & inner_map
                            );

        void specialize ( type::type_map_type const& map
                        , type::type_get_type const& get
                        , gspc::xml::parse::structure_type_util::set_type const& known_structs
                        , state::type & state
                        );

        // ***************************************************************** //

        void type_check (state::type const& state) const;

        void resolve_function_use_recursive
          (std::unordered_map<std::string, function_type const&> known);
        void resolve_types_recursive
          (std::unordered_map<std::string, gspc::pnet::type::signature::signature_type> known);

        void set_prefix (std::string const& prefix);
        void remove_prefix (std::string const& prefix);

      private:
        functions_type _functions;
        places_type _places;
        specializes_type _specializes;
        templates_type _templates;
        transitions_type _transitions;

        //! \todo Everything below should be private with accessors.
      public:
        structs_type structs;
        bool contains_a_module_call{};

      private:
        ::gspc::we::type::property::type _properties;
      };

      // ******************************************************************* //

      std::unordered_map<std::string, ::gspc::we::place_id_type>
      net_synthesize ( ::gspc::we::type::net_type& we_net
                     , place_map_map_type const& place_map_map
                     , net_type const& net
                     , state::type const& state
                     );

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
                  , net_type const& net
                  );
      }
    }
