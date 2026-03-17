// Copyright (C) 2010-2013,2015,2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/id.hpp>
#include <gspc/we/type/property.hpp>
#include <gspc/we/type/signature.hpp>

#include <iosfwd>
#include <optional>
#include <string>
#include <variant>


  namespace gspc::we::type
  {

      namespace port::direction
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


    using PortDirection = std::variant
       < port::direction::In
       , port::direction::Out
       , port::direction::Tunnel
       >;

    std::ostream& operator<< (std::ostream&, PortDirection const&);

    struct Port
    {
    public:
      Port();
      Port ( std::string const& name
             , PortDirection
             , pnet::type::signature::signature_type const&
              , property::type
             );
      Port ( std::string const& name
             , PortDirection
             , pnet::type::signature::signature_type const&
             , we::place_id_type const&
              , property::type
             );

      std::string const& name() const;

      PortDirection direction() const;
      pnet::type::signature::signature_type const& signature() const;
      std::optional<we::place_id_type> const& associated_place() const;
      property::type const& property() const;

      bool is_input() const;
      bool is_output() const;
      bool is_tunnel() const;

    private:
      std::string _name;
      PortDirection _direction;
      pnet::type::signature::signature_type _signature;
      std::optional<we::place_id_type> _associated_place;
      property::type _properties;

      friend class ::boost::serialization::access;
      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };
  }


#include <gspc/we/type/Port.ipp>
