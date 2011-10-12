// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/place.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>

#include <pnete/ui/graph/style/raster.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        place::place
          ( boost::optional< ::xml::parse::type::type_map_type&> type_map
          , item* parent
          )
            : connectable_item (BOTH, type_map, parent)
            , _content()
            , _dragging (false)
            , _name (tr ("<<a place>>"))
        {
          refresh_content();
          connect ( this
                  , SIGNAL (we_type_changed())
                  , SLOT (refresh_content())
                  );
        }

        const QString& place::name (const QString& name_)
        {
          _name = name_;
          refresh_content();
          return _name;
        }
        const QString& place::name() const
        {
          return _name;
        }

        void place::refresh_content()
        {
          _content.setText (_name + " :: " + we_type());
        }

        QRectF place::boundingRect() const
        {
          const QSizeF half_size (_content.size() / 2.0);
          const QPointF pos (-half_size.width(), -half_size.height());
          return QRectF (pos, _content.size());
        }
        void place::paint ( QPainter* painter
                          , const QStyleOptionGraphicsItem* option
                          , QWidget* widget
                          )
        {
          const QSizeF half_size (_content.size() / 2.0);
          const QPointF pos (-half_size.width(), -half_size.height());
          painter->drawStaticText (pos, _content);
          painter->drawRoundedRect ( QRectF
                                     ( pos - QPointF (2.0, 2.0)
                                     , _content.size() + QSizeF (4.0, 4.0)
                                     )
                                   , 5.0
                                   , 5.0
                                   );
        }

        void place::mouseMoveEvent (QGraphicsSceneMouseEvent* event)
        {
          if (!_dragging)
          {
            connectable_item::mouseMoveEvent (event);
            return;
          }

          setPos (style::raster::snap (pos() + event->pos() - _drag_start));
          event->accept();
          scene()->update();
        }
        void place::mousePressEvent (QGraphicsSceneMouseEvent* event)
        {
          if (event->modifiers() == Qt::ControlModifier)
          {
            _dragging = true;
            _drag_start = event->pos();
            event->accept();
            return;
          }

          connectable_item::mousePressEvent (event);
        }

        void place::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
        {
          if (!_dragging)
          {
            connectable_item::mouseReleaseEvent (event);
            return;
          }

          _dragging = false;
          event->accept();
        }
      }
    }
  }
}
