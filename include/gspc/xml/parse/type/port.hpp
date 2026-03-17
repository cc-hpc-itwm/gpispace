// Copyright (C) 2010-2016,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

#include <gspc/xml/parse/error.hpp>
#include <gspc/xml/parse/state.fwd.hpp>
#include <gspc/xml/parse/type/function.fwd.hpp>
#include <gspc/xml/parse/type/net.fwd.hpp>
#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/type_map_type.hpp>

#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/util/xml.fwd.hpp>

#include <gspc/we/type/Port.hpp>
#include <gspc/we/type/property.hpp>
#include <gspc/we/type/signature.hpp>

#include <filesystem>
#include <functional>
#include <optional>
#include <string>



    namespace gspc::xml::parse::type
    {
      struct port_type : with_position_of_definition
      {
      public:
        struct unique_key_type
        {
          unique_key_type (std::string, ::gspc::we::type::PortDirection);

          friend bool operator==
            (unique_key_type const&, unique_key_type const&);

          std::size_t hash_value() const;

        private:
          std::string _name;
          ::gspc::we::type::PortDirection _port_direction;
        };

        port_type ( util::position_type const&
                  , std::string const& name
                  , std::string const& _type
                  , std::optional<std::string> const& _place
                  , ::gspc::we::type::PortDirection const& direction
                  , ::gspc::we::type::property::type const& properties
                  = ::gspc::we::type::property::type()
                  , std::optional<gspc::pnet::type::signature::signature_type> = std::nullopt
                  );

        port_type specialized ( type::type_map_type const& map_in
                              , state::type const&
                              ) const;

        void type_check ( std::filesystem::path const&
                        , state::type const&
                        , function_type const& parent
                        ) const;

        std::string const& name() const;

        std::string const& type() const;

        gspc::pnet::type::signature::signature_type signature() const;
        void resolve_types_recursive
          (std::unordered_map<std::string, gspc::pnet::type::signature::signature_type> known);

        ::gspc::we::type::PortDirection const& direction() const;

        std::optional<std::reference_wrapper<place_type const>> resolved_place
          (net_type const& parent) const;

        ::gspc::we::type::property::type const& properties() const;

        unique_key_type unique_key() const;

        bool is_input() const;
        bool is_output() const;
        bool is_tunnel() const;

      private:
        std::string const _name;
        std::string _type;
        std::optional<gspc::pnet::type::signature::signature_type> _signature;

        //! \todo All these should be private with accessors.
      public:
        std::optional<std::string> place;

      private:
        ::gspc::we::type::PortDirection const _direction;
        ::gspc::we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream&, port_type const&);
      }
    }



namespace std
{
  template<>
    struct hash<::gspc::xml::parse::type::port_type::unique_key_type>
  {
    std::size_t operator()
      (::gspc::xml::parse::type::port_type::unique_key_type const& uk) const
    {
      return uk.hash_value();
    }
  };
}
