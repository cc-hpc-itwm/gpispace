// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/state.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/type_map_type.hpp>
#include <xml/parse/util/position.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/property.hpp>
#include <we/type/signature.hpp>

#include <list>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct place_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::string;

        using token_type = std::string;

        place_type ( util::position_type const&
                   , std::string const& name
                   , std::string const& type
                   , ::boost::optional<bool> is_virtual
                   , ::boost::optional<bool> put_token
                   , std::list<token_type> tokens = {}
                   , we::type::property::type properties = {}
                   , ::boost::optional<pnet::type::signature::signature_type> = ::boost::none
                   );

        std::string const& name() const;
        std::string const& type() const;

        place_type with_name (std::string const& new_name) const;

        pnet::type::signature::signature_type signature() const;
        void resolve_types_recursive
          (std::unordered_map<std::string, pnet::type::signature::signature_type> known);

        void push_token (token_type const& t);

        place_type specialized ( type::type_map_type const& map_in
                               , state::type const&
                               ) const;

        ::boost::optional<bool> const& get_is_virtual() const;
        bool is_virtual() const;
        ::boost::optional<bool> const& put_token() const
        {
          return _put_token;
        }

        we::type::property::type const& properties() const;
        we::type::property::type& properties();

        unique_key_type const& unique_key() const;

      private:
        ::boost::optional<bool> _is_virtual;
        ::boost::optional<bool> _put_token;

        std::string const _name;
        std::string _type;
        ::boost::optional<pnet::type::signature::signature_type> _signature;

        //! \todo All these should be private with accessors.
      public:
        std::list<token_type> tokens;

      private:
        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream & s, place_type const& p);
      }
    }
  }
}
