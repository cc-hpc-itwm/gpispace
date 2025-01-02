// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>
#include <xml/parse/util/unique.hpp>

#include <fhg/util/xml.fwd.hpp>
#include <util-generic/hash/std/tuple.hpp>

#include <we/type/id.hpp> //we::place_id_type
#include <we/type/property.hpp>

#include <string>
#include <unordered_map>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct place_map_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::pair<std::string, std::string>;

        place_map_type ( util::position_type const&
                       , std::string const& _place_virtual
                       , std::string const& _place_real
                       , we::type::property::type const& properties
                       );

        std::string const& place_virtual() const;
        std::string const& place_real() const;

        place_map_type with_place_real (std::string const&) const;

        we::type::property::type const& properties() const;

        unique_key_type unique_key() const;

      private:
        std::string const _place_virtual;
        std::string const _place_real;

        we::type::property::type _properties;
      };

      using place_map_map_type =
        std::unordered_map<std::string, we::place_id_type>;

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , place_map_type const& p
                  );
      }
    }
  }
}
