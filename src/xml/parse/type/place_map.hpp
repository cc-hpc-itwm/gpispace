#pragma once

#include <xml/parse/type/with_position_of_definition.hpp>
#include <xml/parse/util/position.fwd.hpp>
#include <xml/parse/util/unique.hpp>

#include <util-generic/hash/std/tuple.hpp>
#include <fhg/util/xml.fwd.hpp>

#include <we/type/id.hpp> //we::place_id_type
#include <we/type/property.hpp>

#include <string>
#include <unordered_map>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct place_map_type : with_position_of_definition
      {
      public:
        typedef std::pair<std::string, std::string> unique_key_type;

        place_map_type ( const util::position_type&
                       , const std::string & _place_virtual
                       , const std::string & _place_real
                       , const we::type::property::type& properties
                       );

        const std::string& place_virtual() const;
        const std::string& place_real() const;

        place_map_type with_place_real (std::string const&) const;

        const we::type::property::type& properties() const;

        unique_key_type unique_key() const;

      private:
        std::string const _place_virtual;
        std::string const _place_real;

        we::type::property::type _properties;
      };

      typedef std::unordered_map< std::string
                                , we::place_id_type
                                > place_map_map_type;

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const place_map_type & p
                  );
      }
    }
  }
}
