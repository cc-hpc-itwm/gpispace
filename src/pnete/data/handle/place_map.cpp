// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/place_map.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/place.hpp>
#include <pnete/data/handle/port.hpp>

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

        void place_map::remove() const
        {
          change_manager().remove_place_map (*this);
        }

        void place_map::set_property
          ( const ::we::type::property::path_type& path
          , const ::we::type::property::value_type& val
          ) const
        {
          change_manager().set_property (*this, path, val);
        }

        place place_map::resolved_real_place() const
        {
          return place (*get().resolved_real_place(), document());
        }

        port place_map::tunnel_port() const
        {
          return port (*get().resolved_tunnel_port(), document());
        }
      }
    }
  }
}
