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

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/net.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      connect_type::connect_type ( util::position_type const& pod
                                 , std::string const& place
                                 , std::string const& port
                                 , const ::we::edge::type& direction
                                 , we::type::property::type const& properties
                                 )
        : with_position_of_definition (pod)
        , _place (place)
        , _port (port)
        , _direction (direction)
        , _properties (properties)
      {}

      std::string const& connect_type::place() const
      {
        return _place;
      }
      std::string const& connect_type::port() const
      {
        return _port;
      }

      const ::we::edge::type& connect_type::direction() const
      {
        return _direction;
      }

      connect_type connect_type::with_place (std::string const& place) const
      {
        return connect_type ( position_of_definition()
                            , place
                            , _port
                            , _direction
                            , _properties
                            );
      }

      we::type::property::type const& connect_type::properties() const
      {
        return _properties;
      }

      connect_type::unique_key_type connect_type::unique_key() const
      {
        return std::make_tuple
          (_place, _port, we::edge::is_PT (_direction));
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, connect_type const& c)
        {
          s.open ("connect-" + we::edge::enum_to_string (c.direction()));
          s.attr ("port", c.port());
          s.attr ("place", c.place());

          ::we::type::property::dump::dump (s, c.properties());

          s.close ();
        }
      }
    }
  }
}
