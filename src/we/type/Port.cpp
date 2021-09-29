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

#include <we/type/Port.hpp>

#include <fhg/util/boost/variant.hpp>

namespace we
{
  namespace type
  {
    namespace port
    {
      namespace direction
      {
        std::ostream& operator<< (std::ostream& os, In const&)
        {
          return os << "in";
        }
        std::ostream& operator<< (std::ostream& os, Out const&)
        {
          return os << "out";
        }
        std::ostream& operator<< (std::ostream& os, Tunnel const&)
        {
          return os << "tunnel";
        }

        bool operator== (In const&, In const&)
        {
          return true;
        }
        bool operator== (Out const&, Out const&)
        {
          return true;
        }
        bool operator== (Tunnel const&, Tunnel const&)
        {
          return true;
        }
      }
    }

    Port::Port()
      : _name("default")
      , _direction (port::direction::In{})
      , _associated_place (boost::none)
    {}

    Port::Port
      ( std::string const& name
      , PortDirection direction
      , pnet::type::signature::signature_type const& signature
      , we::type::property::type prop
      )
        : _name (name)
        , _direction (direction)
        , _signature (signature)
        , _associated_place (boost::none)
        , _properties (prop)
    {}

    Port::Port
      ( std::string const& name
      , PortDirection direction
      , pnet::type::signature::signature_type const& signature
      , we::place_id_type const& place_id
      , we::type::property::type prop
      )
        : _name (name)
        , _direction (direction)
        , _signature (signature)
        , _associated_place (place_id)
        , _properties (prop)
    {}

    std::string const& Port::name() const
    {
      return _name;
    }

    PortDirection Port::direction() const
    {
      return _direction;
    }
    pnet::type::signature::signature_type const& Port::signature() const
    {
      return _signature;
    }
    boost::optional<we::place_id_type> const& Port::associated_place() const
    {
      return _associated_place;
    }
    we::type::property::type const& Port::property() const
    {
      return _properties;
    }

    bool Port::is_input() const
    {
      return fhg::util::boost::is_of_type<port::direction::In> (_direction);
    }
    bool Port::is_output() const
    {
      return fhg::util::boost::is_of_type<port::direction::Out> (_direction);
    }
    bool Port::is_tunnel() const
    {
      return fhg::util::boost::is_of_type<port::direction::Tunnel> (_direction);
    }
  }
}
