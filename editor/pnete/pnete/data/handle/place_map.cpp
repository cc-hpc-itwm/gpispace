// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/place_map.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/place.hpp>

#include <xml/parse/type/place_map.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        place_map::place_map ( const place_map_meta_base::id_type& id
                             , internal_type* document
                             )
          : place_map_meta_base (id, document)
        { }

        void place_map::remove (const QObject* sender) const
        {
          change_manager().remove_place_map (sender, *this);
        }

        void place_map::set_property
          ( const QObject* sender
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& val
          ) const
        {
          change_manager().set_property (sender, *this, key, val);
        }

        place place_map::resolved_real_place() const
        {
          return place (*get().resolved_real_place(), document());
        }
      }
    }
  }
}
