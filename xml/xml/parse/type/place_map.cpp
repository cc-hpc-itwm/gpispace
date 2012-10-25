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
      place_map_type::place_map_type ( const std::string & _place_virtual
                                     , const std::string & _place_real
                                     , const id::place_map& id
                                     , const id::transition& parent
                                     , id::mapper* id_mapper
                                     )
        : _name (_place_virtual + " <-> " + _place_real)
        , place_virtual (_place_virtual)
        , place_real (_place_real)
        , _id (id)
        , _parent (parent)
        , _id_mapper (id_mapper)
      {
        _id_mapper->put (_id, *this);
      }

      const id::place_map& place_map_type::id() const
      {
        return _id;
      }

      const id::transition& place_map_type::parent() const
      {
        return _parent;
      }

      const std::string& place_map_type::name() const
      {
        return _name;
      }

      bool place_map_type::is_same (const place_map_type& other) const
      {
        return id() == other.id() && parent() == other.parent();
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

