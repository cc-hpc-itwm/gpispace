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
        namespace place
        {
          item::item
          ( place_type& place
          , boost::optional< ::xml::parse::type::type_map_type&> type_map
          , item* parent
          )
            : connectable::item ( connectable::direction::BOTH
                                , type_map
                                , parent
                                , &place.prop
                                )
            , _place (place)
            , _content()
            , _dragging (false)
            , _name (tr ("<<a place>>"))
          {
            refresh_content();
          }

          const place_type& item::place () const
          {
            return _place;
          }

          const std::string& item::we_type() const
          {
            return connectable::item::we_type (place().type);
          }

          const QString& item::name (const QString& name_)
          {
            _name = name_;
            refresh_content();
            return _name;
          }
          const QString& item::name() const
          {
            return _name;
          }

          void item::refresh_content()
          {
            _content.setText (_name + " :: " + QString::fromStdString (we_type()));
          }

          QRectF item::boundingRect() const
          {
            const QSizeF half_size (_content.size() / 2.0);
            const QPointF pos (-half_size.width(), -half_size.height());
            return QRectF (pos, _content.size());
          }
          void item::paint ( QPainter* painter
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

          void item::mouseMoveEvent (QGraphicsSceneMouseEvent* event)
          {
            if (!_dragging)
              {
                connectable::item::mouseMoveEvent (event);
                return;
              }

            setPos (style::raster::snap (pos() + event->pos() - _drag_start));
            event->accept();
            scene()->update();
          }
          void item::mousePressEvent (QGraphicsSceneMouseEvent* event)
          {
            if (event->modifiers() == Qt::ControlModifier)
              {
                _dragging = true;
                _drag_start = event->pos();
                event->accept();
                return;
              }

            connectable::item::mousePressEvent (event);
          }

          void item::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
          {
            if (!_dragging)
              {
                connectable::item::mouseReleaseEvent (event);
                return;
              }

            _dragging = false;
            event->accept();
          }
        }
      }
    }
  }
}
