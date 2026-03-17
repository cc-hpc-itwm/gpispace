// Copyright (C) 2010-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/util/position.fwd.hpp>
#include <gspc/xml/parse/util/unique.hpp>

#include <gspc/util/xml.fwd.hpp>
#include <gspc/util/hash/std/tuple.hpp>

#include <gspc/we/type/id.hpp> //::gspc::we::place_id_type
#include <gspc/we/type/property.hpp>

#include <string>
#include <unordered_map>



    namespace gspc::xml::parse::type
    {
      struct place_map_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::pair<std::string, std::string>;

        place_map_type ( util::position_type const&
                       , std::string const& _place_virtual
                       , std::string const& _place_real
                       , ::gspc::we::type::property::type const& properties
                       );

        std::string const& place_virtual() const;
        std::string const& place_real() const;

        place_map_type with_place_real (std::string const&) const;

        ::gspc::we::type::property::type const& properties() const;

        unique_key_type unique_key() const;

      private:
        std::string const _place_virtual;
        std::string const _place_real;

        ::gspc::we::type::property::type _properties;
      };

      using place_map_map_type =
        std::unordered_map<std::string, ::gspc::we::place_id_type>;

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
                  , place_map_type const& p
                  );
      }
    }
