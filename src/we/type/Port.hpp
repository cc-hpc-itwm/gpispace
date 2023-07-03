// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/id.hpp>
#include <we/type/property.hpp>
#include <we/type/signature.hpp>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <string>

namespace we
{
  namespace type
  {
    namespace port
    {
      namespace direction
      {
        struct In{};
        struct Out{};
        struct Tunnel{};

        std::ostream& operator<< (std::ostream&, In const&);
        std::ostream& operator<< (std::ostream&, Out const&);
        std::ostream& operator<< (std::ostream&, Tunnel const&);

        bool operator== (In const&, In const&);
        bool operator== (Out const&, Out const&);
        bool operator== (Tunnel const&, Tunnel const&);
      }
    }

    using PortDirection = ::boost::variant
       < port::direction::In
       , port::direction::Out
       , port::direction::Tunnel
       >;

    struct Port
    {
    public:
      Port();
      Port ( std::string const& name
             , PortDirection
             , pnet::type::signature::signature_type const&
             , we::type::property::type
             );
      Port ( std::string const& name
             , PortDirection
             , pnet::type::signature::signature_type const&
             , we::place_id_type const&
             , we::type::property::type
             );

      std::string const& name() const;

      PortDirection direction() const;
      pnet::type::signature::signature_type const& signature() const;
      ::boost::optional<we::place_id_type> const& associated_place() const;
      we::type::property::type const& property() const;

      bool is_input() const;
      bool is_output() const;
      bool is_tunnel() const;

    private:
      std::string _name;
      PortDirection _direction;
      pnet::type::signature::signature_type _signature;
      ::boost::optional<we::place_id_type> _associated_place;
      we::type::property::type _properties;

      friend class ::boost::serialization::access;
      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };
  }
}

#include <we/type/Port.ipp>
