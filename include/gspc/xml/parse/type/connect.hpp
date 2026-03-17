// Copyright (C) 2010-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/type/connect.fwd.hpp>

#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/util/xml.fwd.hpp>
#include <gspc/util/hash/std/pair.hpp>

#include <gspc/we/type/net.hpp>
#include <gspc/we/type/property.hpp>

#include <string>
#include <tuple>




    namespace gspc::xml::parse::type
    {
      struct connect_type : with_position_of_definition
      {
      public:
        //! \note          place,       port,        PT||PT_READ
        using unique_key_type = std::tuple<std::string, std::string, bool>;

        connect_type ( util::position_type const&
                     , std::string const& place
                     , std::string const& port
                     , const ::gspc::we::edge::type& direction
                     , ::gspc::we::type::property::type const& properties
                     = ::gspc::we::type::property::type()
                     );

        std::string const& place() const;
        std::string const& port() const;

        const ::gspc::we::edge::type& direction() const;

        connect_type with_place (std::string const& new_place) const;

        ::gspc::we::type::property::type const& properties() const;

        unique_key_type unique_key() const;

      private:
        std::string const _place;
        std::string const _port;

        ::gspc::we::edge::type const _direction;

        ::gspc::we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream&, connect_type const&);
      }
    }
