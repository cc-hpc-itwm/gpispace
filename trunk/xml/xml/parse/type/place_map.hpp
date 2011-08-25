// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_MAP_HPP
#define _XML_PARSE_TYPE_PLACE_MAP_HPP

#include <string>
#include <iostream>

#include <boost/filesystem.hpp>

#include <we/type/property.hpp>
#include <we/type/id.hpp>

#include <fhg/util/xml.hpp>

#include <xml/parse/util/unique.hpp>

namespace xml_util = ::fhg::util::xml;

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct place_map_type
      {
      public:
        std::string place_virtual;
        std::string place_real;
        std::string name;
        we::type::property::type prop;

        place_map_type ( const std::string & _place_virtual
                       , const std::string & _place_real
                       )
          : place_virtual (_place_virtual)
          , place_real (_place_real)
          , name (_place_virtual + " <-> " + _place_real)
        {}
      };

      typedef xml::util::unique<place_map_type>::elements_type place_maps_type;
      typedef boost::unordered_map<std::string, petri_net::pid_t> place_map_map_type;

      namespace dump
      {
        inline void dump (xml_util::xmlstream & s, const place_map_type & p)
        {
          s.open ("place-map");
          s.attr ("virtual", p.place_virtual);
          s.attr ("real", p.place_real);

          ::we::type::property::dump::dump (s, p.prop);

          s.close ();
        }
      } // namespace dump
    }
  }
}

#endif
