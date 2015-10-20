// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <xml/parse/id/generic.hpp>
#include <xml/parse/type/transition.fwd.hpp>
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
        ID_SIGNATURES(place_map);
        PARENT_SIGNATURES(transition);

      public:
        typedef std::pair<std::string, std::string> unique_key_type;

        place_map_type ( ID_CONS_PARAM(place_map)
                       , PARENT_CONS_PARAM(transition)
                       , const util::position_type&
                       , const std::string & _place_virtual
                       , const std::string & _place_real
                       , const we::type::property::type& properties
                       );

        const std::string& place_virtual() const;
        const std::string& place_real() const;
        const std::string& place_real (const std::string&);

      private:
        friend struct transition_type;
        const std::string& place_real_impl (const std::string&);

      public:
        boost::optional<const id::ref::place&> resolved_virtual_place() const;
        boost::optional<const id::ref::port&> resolved_tunnel_port() const;
        boost::optional<const id::ref::place&> resolved_real_place() const;

        const we::type::property::type& properties() const;
        we::type::property::type& properties();

        unique_key_type unique_key() const;

        id::ref::place_map clone
          ( const boost::optional<parent_id_type>& parent = boost::none
          , const boost::optional<id::mapper*>& mapper = boost::none
          ) const;

      private:
        std::string _place_virtual;
        std::string _place_real;

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
