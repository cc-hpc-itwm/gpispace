// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/place_map.hpp>

#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/style/association.hpp>
#include <pnete/ui/graph/style/isc13.hpp>

#include <util/qt/cast.hpp>

#include <xml/parse/type/port.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace
        {
          boost::optional<QColor> color_if_type
            (const base_item* item, const QColor& c, const QString& type)
          {
            return boost::make_optional
              ( fhg::util::qt::throwing_qobject_cast<const place_map*>
                (item)->handle().tunnel_port().type() == type
              , c
              );
          }
        }


        place_map::place_map ( port_item* port
                             , place_item* place
                             , const data::handle::place_map& handle
                             )
            : association (port, place)
            , _handle (handle)
        {
          style::isc13::add_colors_for_types (&_style, color_if_type);

          handle.connect_to_change_mgr
            (this, "place_map_removed", "data::handle::place_map");
        }

        const data::handle::place_map& place_map::handle() const
        {
          return _handle;
        }

        QPainterPath place_map::shape() const
        {
          return style::association::shape_no_cap (all_points());
        }

        void place_map::place_map_removed
          (const data::handle::place_map& changed)
        {
          if (changed == handle())
          {
            scene()->removeItem (this);
            deleteLater();
          }
        }
      }
    }
  }
}
