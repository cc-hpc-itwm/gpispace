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

#pragma once

#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>
#include <xml/parse/util/unique.hpp>

#include <util-generic/hash/std/tuple.hpp>
#include <fhg/util/xml.fwd.hpp>

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
        typedef std::pair<std::string, std::string> unique_key_type;

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

      typedef std::unordered_map< std::string
                                , we::place_id_type
                                > place_map_map_type;

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , place_map_type const& p
                  );
      }
    }
  }
}
