// mirko.rahn@itwm.fraunhofer.de

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
        ( const util::position_type& position_of_definition
        , const std::string& name
        , const std::string& size
        , const boost::optional<bool>& read_only
        , const we::type::property::type& properties
        )
        : with_position_of_definition (position_of_definition)
        , _name (name)
        , _size (size)
        , _read_only (read_only)
        , _properties (properties)
      {}

      const std::string& memory_buffer_type::name() const
      {
        return _name;
      }
      const std::string& memory_buffer_type::size() const
      {
        return _size;
      }
      const boost::optional<bool>& memory_buffer_type::read_only() const
      {
        return _read_only;
      }

      const we::type::property::type& memory_buffer_type::properties() const
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
          , const memory_buffer_type& memory_buffer
          )
        {
          s.open ("memory-buffer");
          s.attr ("name", memory_buffer.name());
          s.attr ("read-only", memory_buffer.read_only());

          ::we::type::property::dump::dump (s, memory_buffer.properties());

          s.open ("size");
          s.content (memory_buffer.size());

          s.close();
          s.close();
        }
      }
    }
  }
}
