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
        place_map_type ( ID_CONS_PARAM(place_map)
                       , PARENT_CONS_PARAM(transition)
                       , const std::string & _place_virtual
                       , const std::string & _place_real
                       );

        std::string name() const;

      public:
        std::string place_virtual;
        std::string place_real;
        we::type::property::type prop;
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
