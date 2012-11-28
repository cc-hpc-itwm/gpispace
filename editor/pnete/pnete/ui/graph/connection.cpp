// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connection.hpp>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/style/association.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

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
          , const boost::optional<data::handle::connect>& handle
          , bool read
          )
            : association (start, end)
            , _handle (handle)
            , _read (read)
        { }

        const bool& connection_item::read() const
        {
          return _read;
        }
        const bool& connection_item::read (const bool& read_)
        {
          return _read = read_;
        }

        QPainterPath connection_item::shape () const
        {
          return association::shape();
        }

        void connection_item::paint ( QPainter* painter
                                    , const QStyleOptionGraphicsItem* option
                                    , QWidget* widget
                                    )
        {
          association::paint (painter, option, widget);
        }
      }
    }
  }
}
