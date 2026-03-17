// Copyright (C) 2010-2013,2015-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/state.fwd.hpp>
#include <gspc/xml/parse/type/net.fwd.hpp>
#include <gspc/xml/parse/type/with_position_of_definition.hpp>
#include <gspc/xml/parse/type_map_type.hpp>
#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/util/xml.fwd.hpp>

#include <gspc/we/type/property.hpp>
#include <gspc/we/type/signature.hpp>

#include <list>
#include <string>

#include <filesystem>
#include <optional>



    namespace gspc::xml::parse::type
    {
      struct place_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::string;

        using token_type = std::string;

        place_type ( util::position_type const&
                   , std::string const& name
                   , std::string const& type
                   , std::optional<bool> is_virtual
                   , std::optional<bool> put_token
                   , std::optional<bool> shared_sink = std::nullopt
                   , std::optional<bool> generator = std::nullopt
                   , std::list<token_type> tokens = {}
                   , ::gspc::we::type::property::type properties = {}
                   , std::optional<gspc::pnet::type::signature::signature_type> = std::nullopt
                   );

        std::string const& name() const;
        std::string const& type() const;

        place_type with_name (std::string const& new_name) const;

        gspc::pnet::type::signature::signature_type signature() const;
        void resolve_types_recursive
          (std::unordered_map<std::string, gspc::pnet::type::signature::signature_type> known);

        void push_token (token_type const& t);

        place_type specialized ( type::type_map_type const& map_in
                               , state::type const&
                               ) const;

        std::optional<bool> const& get_is_virtual() const;
        bool is_virtual() const;
        std::optional<bool> const& put_token() const
        {
          return _put_token;
        }
        std::optional<bool> const& shared_sink() const
        {
          return _shared_sink;
        }
        bool is_shared_sink() const
        {
          return _shared_sink.value_or (false);
        }
        std::optional<bool> const& generator() const
        {
          return _generator;
        }
        bool is_generator() const
        {
          return _generator.value_or (false);
        }

        ::gspc::we::type::property::type const& properties() const;
        ::gspc::we::type::property::type& properties();

        unique_key_type const& unique_key() const;

      private:
        std::optional<bool> _is_virtual;
        std::optional<bool> _put_token;
        std::optional<bool> _shared_sink;
        std::optional<bool> _generator;

        std::string const _name;
        std::string _type;
        std::optional<gspc::pnet::type::signature::signature_type> _signature;

        //! \todo All these should be private with accessors.
      public:
        std::list<token_type> tokens;

      private:
        ::gspc::we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream & s, place_type const& p);
      }
    }
