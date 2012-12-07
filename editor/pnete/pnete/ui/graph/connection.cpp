// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connection.hpp>

#include <QDebug>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        connection_item::connection_item
          ( connectable_item* start
          , connectable_item* end
          , const data::handle::connect& handle
          )
            : association (start, end)
            , _handle (handle)
        {
          handle.connect_to_change_mgr
            ( this
            , "connection_direction_changed"
            , "const data::handle::connect&"
            );
        }

        const data::handle::connect& connection_item::handle() const
        {
          return _handle;
        }

        QPainterPath connection_item::shape () const
        {
          return association::shape();
        }

        void connection_item::paint
          (QPainter* painter, const QStyleOptionGraphicsItem* opt, QWidget* wid)
        {
          association::paint (painter, opt, wid);
        }

        void connection_item::connection_direction_changed
          (const QObject*, const data::handle::connect& changed_handle)
        {
          if (changed_handle == handle())
          {
            qDebug()
              << "NYI: connection should somehow show if it is read or in.";
          }
        }
      }
    }
  }
}
