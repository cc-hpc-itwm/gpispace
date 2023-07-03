// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/memory_buffer.hpp>

#include <xml/parse/error.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      memory_buffer_type::memory_buffer_type
        ( util::position_type const& position_of_definition
        , std::string const& name
        , std::string const& size
        , std::string const& alignment
        , ::boost::optional<bool> const& read_only
        , we::type::property::type const& properties
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
      ::boost::optional<bool> const& memory_buffer_type::read_only() const
      {
        return _read_only;
      }

      we::type::property::type const& memory_buffer_type::properties() const
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
          ( ::fhg::util::xml::xmlstream& s
          , memory_buffer_type const& memory_buffer
          )
        {
          s.open ("memory-buffer");
          s.attr ("name", memory_buffer.name());
          s.attr ("read-only", memory_buffer.read_only());

          ::we::type::property::dump::dump (s, memory_buffer.properties());

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
  }
}
