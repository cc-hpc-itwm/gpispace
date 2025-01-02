// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/type/connect.fwd.hpp>

#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>
#include <util-generic/hash/std/pair.hpp>

#include <we/type/net.hpp>
#include <we/type/property.hpp>

#include <string>
#include <tuple>

#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct connect_type : with_position_of_definition
      {
      public:
        //! \note          place,       port,        PT||PT_READ
        using unique_key_type = std::tuple<std::string, std::string, bool>;

        connect_type ( util::position_type const&
                     , std::string const& place
                     , std::string const& port
                     , const ::we::edge::type& direction
                     , we::type::property::type const& properties
                     = we::type::property::type()
                     );

        std::string const& place() const;
        std::string const& port() const;

        const ::we::edge::type& direction() const;

        connect_type with_place (std::string const& new_place) const;

        we::type::property::type const& properties() const;

        unique_key_type unique_key() const;

      private:
        std::string const _place;
        std::string const _port;

        ::we::edge::type const _direction;

        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, connect_type const&);
      }
    }
  }
}
