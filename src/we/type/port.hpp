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

#include <we/type/id.hpp>
#include <we/type/property.hpp>
#include <we/type/signature.hpp>
#include <we/type/signature/serialize.hpp>

#include <util-generic/cxx14/enum_hash.hpp>

#include <boost/optional.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/optional.hpp>

#include <iosfwd>
#include <limits>
#include <string>

namespace we
{
  namespace type
  {
    enum PortDirection
      { PORT_IN
      , PORT_OUT
      , PORT_TUNNEL
      };

    std::string enum_to_string (const PortDirection&);

    struct port_t
    {
    public:
      port_t ()
        : _name("default")
        , _direction(PORT_IN)
        , _associated_place(boost::none)
      {}

      port_t ( const std::string & name
             , PortDirection direction
             , const pnet::type::signature::signature_type& signature
             , const we::type::property::type prop
             = we::type::property::type()
             )
        : _name(name)
        , _direction(direction)
        , _signature(signature)
        , _associated_place(boost::none)
        , _properties(prop)
      {}

      port_t ( const std::string & name
             , PortDirection direction
             , const pnet::type::signature::signature_type& signature
             , const we::place_id_type& place_id
             , const we::type::property::type prop
             = we::type::property::type()
             )
        : _name(name)
        , _direction(direction)
        , _signature(signature)
        , _associated_place(place_id)
        , _properties(prop)
      {}

      const std::string & name() const { return _name; }

      PortDirection direction() const { return _direction; }
      const pnet::type::signature::signature_type& signature() const { return _signature; }
      const boost::optional<we::place_id_type>& associated_place() const { return _associated_place; }
      const we::type::property::type & property() const { return _properties; }

      bool is_input() const { return _direction == PORT_IN; }
      bool is_output() const { return _direction == PORT_OUT; }
      bool is_tunnel() const { return _direction == PORT_TUNNEL; }

    private:
      std::string _name;
      PortDirection _direction;
      pnet::type::signature::signature_type _signature;
      boost::optional<we::place_id_type> _associated_place;
      we::type::property::type _properties;

      friend class boost::serialization::access;
      template<typename Archive>
      void serialize (Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP(_name);
        ar & BOOST_SERIALIZATION_NVP(_direction);
        ar & BOOST_SERIALIZATION_NVP(_signature);
        ar & BOOST_SERIALIZATION_NVP(_associated_place);
        ar & BOOST_SERIALIZATION_NVP(_properties);
      }
    };
  }
}
