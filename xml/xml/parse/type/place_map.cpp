// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/place_map.hpp>

#include <xml/parse/id/mapper.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      place_map_type::place_map_type ( ID_CONS_PARAM(place_map)
                                     , PARENT_CONS_PARAM(transition)
                                     , const std::string & _place_virtual
                                     , const std::string & _place_real
                                     )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _name (_place_virtual + " <-> " + _place_real)
        , place_virtual (_place_virtual)
        , place_real (_place_real)
      {
        _id_mapper->put (_id, *this);
      }

      const std::string& place_map_type::name() const
      {
        return _name;
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const place_map_type & p
                  )
        {
          s.open ("place-map");
          s.attr ("virtual", p.place_virtual);
          s.attr ("real", p.place_real);

          ::we::type::property::dump::dump (s, p.prop);

          s.close ();
        }
      }
    }
  }
}

