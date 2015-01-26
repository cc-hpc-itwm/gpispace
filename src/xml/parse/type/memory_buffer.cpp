// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/type/memory_buffer.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/id/mapper.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      memory_buffer_type::memory_buffer_type
        ( ID_CONS_PARAM (memory_buffer)
        , PARENT_CONS_PARAM (function)
        , const util::position_type& position_of_definition
        , const std::string& name
        , const std::string& size
        , const boost::optional<bool>& read_only
        , const we::type::property::type& properties
        )
        : with_position_of_definition (position_of_definition)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _name (name)
        , _size (size)
        , _read_only (read_only)
        , _properties (properties)
      {
        _id_mapper->put (_id, *this);
      }

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

      id::ref::memory_buffer memory_buffer_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return memory_buffer_type
          ( new_id
          , new_mapper
          , parent
          , _position_of_definition
          , _name
          , _size
          , _read_only
          , _properties
          ).make_reference_id();
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
