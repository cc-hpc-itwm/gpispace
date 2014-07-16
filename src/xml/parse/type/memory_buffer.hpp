// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_MEMORY_BUFFER_HPP
#define _XML_PARSE_TYPE_MEMORY_BUFFER_HPP

#include <xml/parse/id/generic.hpp>
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
        ID_SIGNATURES (memory_buffer);
        PARENT_SIGNATURES (function);

      public:
        using unique_key_type = std::string;

        memory_buffer_type ( ID_CONS_PARAM (memory_buffer)
                           , PARENT_CONS_PARAM (function)
                           , const util::position_type&
                           , const std::string& name
                           , const std::string& size
                           , const boost::optional<bool>& read_only
                           , const we::type::property::type& properties
                           );

        const std::string& name() const;
        const std::string& size() const;
        const boost::optional<bool>& read_only() const;

        const we::type::property::type& properties() const;

        unique_key_type unique_key() const;

        id::ref::memory_buffer clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        std::string _name;
        std::string _size;
        boost::optional<bool> _read_only;
        we::type::property::type _properties;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const memory_buffer_type&);
      }
    }
  }
}

#endif
