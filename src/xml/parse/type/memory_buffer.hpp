// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/with_position_of_definition.hpp>

#include <xml/parse/util/position.fwd.hpp>

#include <we/type/property.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct memory_buffer_type : with_position_of_definition
      {
      public:
        using unique_key_type = std::string;

        memory_buffer_type ( util::position_type const&
                           , std::string const& name
                           , std::string const& size
                           , std::string const& alignment
                           , ::boost::optional<bool> const& read_only
                           , we::type::property::type const& properties
                           );
        memory_buffer_type (memory_buffer_type const&) = default;
        memory_buffer_type (memory_buffer_type&&) = default;
        memory_buffer_type& operator= (memory_buffer_type const&) = delete;
        memory_buffer_type& operator= (memory_buffer_type&&) = delete;
        ~memory_buffer_type() = default;

        std::string const& name() const;
        std::string const& size() const;
        std::string const& alignment() const;
        ::boost::optional<bool> const& read_only() const;

        we::type::property::type const& properties() const;

        unique_key_type unique_key() const;

      private:
        std::string const _name;
        std::string _size;
        std::string _alignment;
        ::boost::optional<bool> _read_only;
        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, memory_buffer_type const&);
      }
    }
  }
}
