// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/place_map.hpp>

#include <xml/parse/type/net.hpp>
#include <xml/parse/type/transition.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      place_map_type::place_map_type ( const util::position_type& pod
                                     , const std::string & place_virtual
                                     , const std::string & place_real
                                     , const we::type::property::type& prop
                                     )
        : with_position_of_definition (pod)
        , _place_virtual (place_virtual)
        , _place_real (place_real)
        , _properties (prop)
      {}

      const std::string& place_map_type::place_virtual() const
      {
        return _place_virtual;
      }
      const std::string& place_map_type::place_real() const
      {
        return _place_real;
      }

      place_map_type place_map_type::with_place_real (std::string const& place_real) const
      {
        return place_map_type ( position_of_definition()
                              , _place_virtual
                              , place_real
                              , _properties
                              );
      }

      const we::type::property::type& place_map_type::properties() const
      {
        return _properties;
      }

      place_map_type::unique_key_type place_map_type::unique_key() const
      {
        return std::make_pair (place_virtual(), place_real());
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const place_map_type & p
                  )
        {
          s.open ("place-map");
          s.attr ("virtual", p.place_virtual());
          s.attr ("real", p.place_real());

          ::we::type::property::dump::dump (s, p.properties());

          s.close ();
        }
      }
    }
  }
}
