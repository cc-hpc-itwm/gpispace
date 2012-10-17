// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_MAP_HPP
#define _XML_PARSE_TYPE_PLACE_MAP_HPP

#include <xml/parse/util/id_type.hpp>
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
      public:
        std::string place_virtual;
        std::string place_real;
        std::string name;
        we::type::property::type prop;

      private:
        ::fhg::xml::parse::util::id_type _id;

      public:
        place_map_type ( const std::string & _place_virtual
                       , const std::string & _place_real
                       , const ::fhg::xml::parse::util::id_type& id
                       );

        const ::fhg::xml::parse::util::id_type& id() const;

        bool is_same (const place_map_type& other) const;
      };

      typedef xml::util::unique<place_map_type>::elements_type place_maps_type;
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
