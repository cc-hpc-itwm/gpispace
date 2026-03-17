// Copyright (C) 2014-2016,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/type/function.fwd.hpp>
#include <gspc/xml/parse/type/with_position_of_definition.hpp>

#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/we/type/property.hpp>

#include <gspc/util/xml.fwd.hpp>

#include <string>
#include <optional>



    namespace gspc::xml::parse::type
    {
      struct memory_buffer_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::string;

        memory_buffer_type ( util::position_type const&
                           , std::string const& name
                           , std::string const& size
                           , std::string const& alignment
                           , std::optional<bool> const& read_only
                           , ::gspc::we::type::property::type const& properties
                           );
        memory_buffer_type (memory_buffer_type const&) = default;
        memory_buffer_type (memory_buffer_type&&) = default;
        memory_buffer_type& operator= (memory_buffer_type const&) = delete;
        memory_buffer_type& operator= (memory_buffer_type&&) = delete;
        ~memory_buffer_type() = default;

        std::string const& name() const;
        std::string const& size() const;
        std::string const& alignment() const;
        std::optional<bool> const& read_only() const;

        ::gspc::we::type::property::type const& properties() const;

        unique_key_type unique_key() const;

      private:
        std::string const _name;
        std::string _size;
        std::string _alignment;
        std::optional<bool> _read_only;
        ::gspc::we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream&, memory_buffer_type const&);
      }
    }
