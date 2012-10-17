// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/place_map.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      place_map_type::place_map_type ( const std::string & _place_virtual
                                     , const std::string & _place_real
                                     , const ::fhg::xml::parse::util::id_type& id
                                     )
        : place_virtual (_place_virtual)
        , place_real (_place_real)
        , name (_place_virtual + " <-> " + _place_real)
        , _id (id)
      { }

      const ::fhg::xml::parse::util::id_type& place_map_type::id() const
      {
        return _id;
      }

      bool place_map_type::is_same (const place_map_type& other) const
      {
        return id() == other.id();
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

