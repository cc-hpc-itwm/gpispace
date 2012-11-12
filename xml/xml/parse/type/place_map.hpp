// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_MAP_HPP
#define _XML_PARSE_TYPE_PLACE_MAP_HPP

#include <xml/parse/id/generic.hpp>
#include <xml/parse/util/unique.hpp>

#include <xml/parse/type/transition.fwd.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <we/type/id.hpp> //petri_net::pid_t
#include <we/type/property.hpp>

#include <string>

#include <boost/unordered/unordered_map_fwd.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct place_map_type
      {
        ID_SIGNATURES(place_map);
        PARENT_SIGNATURES(transition);

      public:
        typedef std::pair<std::string, std::string> unique_key_type;

        place_map_type ( ID_CONS_PARAM(place_map)
                       , PARENT_CONS_PARAM(transition)
                       , const std::string & _place_virtual
                       , const std::string & _place_real
                       , const we::type::property::type& properties
                       );

        const std::string& place_virtual() const;
        const std::string& place_real() const;
        const std::string& place_real(const std::string&);
        const we::type::property::type& properties() const;

        unique_key_type unique_key() const;

        id::ref::place_map clone
          (const boost::optional<parent_id_type>& parent = boost::none) const;

      private:
        std::string _place_virtual;
        std::string _place_real;

        we::type::property::type _properties;
      };

      typedef boost::unordered_map<std::string, petri_net::pid_t> place_map_map_type;

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const place_map_type & p
                  );
      }
    }
  }
}

#endif
