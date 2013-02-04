// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/place_map.hpp>

#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/style/association.hpp>

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
          boost::optional<const Qt::PenStyle&> pen_style (const base_item* item)
          {
            static Qt::PenStyle why_is_the_return_value_a_reference
              (Qt::DotLine);

            return why_is_the_return_value_a_reference;
          }
        }

        place_map::place_map ( port_item* port
                             , place_item* place
                             , const data::handle::place_map& handle
                             )
            : association (port, place)
            , _handle (handle)
        {
          _style.push<Qt::PenStyle> ("border_style", mode::NORMAL, pen_style);

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
