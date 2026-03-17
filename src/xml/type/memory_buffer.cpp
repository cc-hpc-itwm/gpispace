// Copyright (C) 2014,2016,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/memory_buffer.hpp>

#include <gspc/xml/parse/error.hpp>

#include <gspc/util/xml.hpp>
#include <optional>



    namespace gspc::xml::parse::type
    {
      memory_buffer_type::memory_buffer_type
        ( util::position_type const& position_of_definition
        , std::string const& name
        , std::string const& size
        , std::string const& alignment
        , std::optional<bool> const& read_only
        , ::gspc::we::type::property::type const& properties
        )
        : with_position_of_definition (position_of_definition)
        , _name (name)
        , _size (size)
        , _alignment (alignment)
        , _read_only (read_only)
        , _properties (properties)
      {}

      std::string const& memory_buffer_type::name() const
      {
        return _name;
      }
      std::string const& memory_buffer_type::size() const
      {
        return _size;
      }
      std::string const& memory_buffer_type::alignment() const
      {
        return _alignment;
      }
      std::optional<bool> const& memory_buffer_type::read_only() const
      {
        return _read_only;
      }

      ::gspc::we::type::property::type const& memory_buffer_type::properties() const
      {
        return _properties;
      }

      memory_buffer_type::unique_key_type
        memory_buffer_type::unique_key() const
      {
        return _name;
      }

      namespace dump
      {
        void dump
          ( ::gspc::util::xml::xmlstream& s
          , memory_buffer_type const& memory_buffer
          )
        {
          s.open ("memory-buffer");
          s.attr ("name", memory_buffer.name());
          s.attr ("read-only", memory_buffer.read_only());

          ::gspc::we::type::property::dump::dump (s, memory_buffer.properties());

          s.open ("size");
          s.content (memory_buffer.size());
          s.close();

          s.open ("alignment");
          s.content (memory_buffer.alignment());
          s.close();

          s.close();
        }
      }
    }
