// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_MAP_HPP
#define _XML_PARSE_TYPE_PLACE_MAP_HPP

#include <xml/parse/id/generic.hpp>
#include <xml/parse/util/unique.hpp>

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
        ID_SIGNATURES(place_map)

      public:
        place_map_type ( ID_CONS_PARAM(place_map)
                       , const std::string & _place_virtual
                       , const std::string & _place_real
                       , const id::transition& parent
                       );

        const id::transition& parent() const;

        std::string _name;

      public:
        std::string place_virtual;
        std::string place_real;
        we::type::property::type prop;

        const std::string& name() const;

      private:
        id::transition _parent;
      };

      typedef xml::util::unique<place_map_type,id::place_map>::elements_type place_maps_type;
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
