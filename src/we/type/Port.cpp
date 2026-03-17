// Copyright (C) 2012,2014,2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/Port.hpp>

#include <variant>


  namespace gspc::we::type
  {

      namespace port::direction
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


    Port::Port()
      : _name("default")
      , _direction (port::direction::In{})
      , _associated_place (std::nullopt)
    {}

    Port::Port
      ( std::string const& name
      , PortDirection direction
      , pnet::type::signature::signature_type const& signature
      , property::type prop
      )
        : _name (name)
        , _direction (direction)
        , _signature (signature)
        , _associated_place (std::nullopt)
        , _properties (prop)
    {}

    Port::Port
      ( std::string const& name
      , PortDirection direction
      , pnet::type::signature::signature_type const& signature
      , we::place_id_type const& place_id
      , property::type prop
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
    std::optional<we::place_id_type> const& Port::associated_place() const
    {
      return _associated_place;
    }
    property::type const& Port::property() const
    {
      return _properties;
    }

    std::ostream& operator<<
      ( std::ostream& os
      , PortDirection const& direction
      )
    {
      return std::visit
        ( [&] (auto const& d) -> std::ostream&
          {
            return os << d;
          }
        , direction
        );
    }

    bool Port::is_input() const
    {
      return std::holds_alternative<port::direction::In> (_direction);
    }
    bool Port::is_output() const
    {
      return std::holds_alternative<port::direction::Out> (_direction);
    }
    bool Port::is_tunnel() const
    {
      return std::holds_alternative<port::direction::Tunnel> (_direction);
    }
  }
