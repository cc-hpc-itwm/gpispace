// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/type/net.fwd.hpp>

#include <xml/parse/type/function.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <xml/parse/util/unique.hpp>

#include <xml/parse/type/dumps.hpp>

#include <we/type/id.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct net_type : with_position_of_definition
      {
      public:
        using functions_type = fhg::pnet::util::unique<function_type>;
        using places_type = fhg::pnet::util::unique<place_type>;
        using specializes_type = fhg::pnet::util::unique<specialize_type>;
        using templates_type = fhg::pnet::util::unique<tmpl_type>;
        using transitions_type = fhg::pnet::util::unique<transition_type>;

        net_type ( util::position_type const&
                 , functions_type
                 , places_type
                 , specializes_type
                 , templates_type
                 , transitions_type
                 , structs_type
                 , we::type::property::type
                 );

        we::type::property::type const& properties() const;

        // ***************************************************************** //

        functions_type const& functions() const;
        places_type const& places() const;
        specializes_type const& specializes() const;
        templates_type const& templates() const;
        transitions_type const& transitions() const;
        //! \note find_module_calls needs to modify transitions when diving
        transitions_type& transitions();

        // ***************************************************************** //

        ::boost::optional<tmpl_type const&>
          get_template (std::string const& name) const;

        // ***************************************************************** //

        void type_map_apply ( type::type_map_type const& outer_map
                            , type::type_map_type & inner_map
                            );

        void specialize ( type::type_map_type const& map
                        , type::type_get_type const& get
                        , xml::parse::structure_type_util::set_type const& known_structs
                        , state::type & state
                        );

        // ***************************************************************** //

        void type_check (state::type const& state) const;

        void resolve_function_use_recursive
          (std::unordered_map<std::string, function_type const&> known);
        void resolve_types_recursive
          (std::unordered_map<std::string, pnet::type::signature::signature_type> known);

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
        we::type::property::type _properties;
      };

      // ******************************************************************* //

      std::unordered_map<std::string, we::place_id_type>
      net_synthesize ( we::type::net_type& we_net
                     , place_map_map_type const& place_map_map
                     , net_type const& net
                     , state::type const& state
                     );

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , net_type const& net
                  );
      }
    }
  }
}
