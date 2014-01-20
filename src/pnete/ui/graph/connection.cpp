// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/style/isc13.hpp>

#include <util/qt/cast.hpp>

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
          boost::optional<Qt::PenStyle> pen_style (const base_item* item)
          {
            if ( fhg::util::qt::throwing_qobject_cast<const connection_item*>
                 (item)->handle().is_read()
               )
            {
              return Qt::DashLine;
            }

            return boost::none;
          }

          boost::optional<QBrush> color_if_type
            (const base_item* item, const QBrush& c, const QString& type)
          {
            return boost::make_optional
              ( fhg::util::qt::throwing_qobject_cast<const connection_item*>
                (item)->handle().resolved_port().type() == type
              , c
              );
          }
        }

        connection_item::connection_item
          ( connectable_item* start
          , connectable_item* end
          , const data::handle::connect& handle
          )
            : association (start, end)
            , _handle (handle)
        {
          _style.push<Qt::PenStyle> ("border_style", pen_style);

          style::isc13::add_colors_for_types (&_style, color_if_type);

          handle.connect_to_change_mgr
            (this, "connection_direction_changed", "data::handle::connect");

          handle.connect_to_change_mgr
            (this, "connection_removed", "data::handle::connect");
        }

        const data::handle::connect& connection_item::handle() const
        {
          return _handle;
        }

        void connection_item::connection_removed
          (const data::handle::connect& changed)
        {
          if (changed == handle())
          {
            scene()->removeItem (this);
            deleteLater();
          }
        }

        void connection_item::connection_direction_changed
          (const data::handle::connect& changed_handle)
        {
          if (changed_handle == handle())
          {
            clear_style_cache();
            update();
          }
        }
      }
    }
  }
}
