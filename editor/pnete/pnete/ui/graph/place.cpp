// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/place.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>

#include <pnete/ui/graph/style/raster.hpp>

#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/transition.hpp>

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
          , graph::item* parent
          )
            : connectable::item ( connectable::direction::BOTH
                                , type_map
                                , parent
                                , &place.prop
                                )
            , _place (place)
            , _content()
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

          const std::string& item::name() const
          {
            return place().name;
          }

          void item::refresh_content()
          {
            _content.setText ( QString::fromStdString (name())
                             + " :: "
                             + QString::fromStdString (we_type())
                             );
          }

          const QStaticText& item::content() const
          {
            return _content;
          }
          QSizeF item::content_size() const
          {
            return content().size();
          }
          QPointF item::content_pos() const
          {
            const QSizeF half_size (content_size() / 2.0);

            return QPointF (-half_size.width(), -half_size.height());
          }

          QPainterPath item::shape () const
          {
            QPainterPath path;
            const qreal d (3.0);

            path.addRoundRect ( QRectF
                                ( content_pos() - QPointF (d, d)
                                , content_size() + QSizeF (2*d, 2*d)
                                )
                              , 2*d
                              , 2*d
                              );

            return path;
          }

          void item::paint ( QPainter* painter
                            , const QStyleOptionGraphicsItem* option
                            , QWidget* widget
                            )
          {
            style::draw_shape (this, painter);

            painter->drawStaticText (content_pos(), content());
          }

          void item::setPos (const QPointF& new_position)
          {
            const QPointF old_position (pos());

            graph::item::setPos (new_position);

            foreach (QGraphicsItem* collidingItem, collidingItems())
              {
                if (  qgraphicsitem_cast<item*> (collidingItem)
                   || qgraphicsitem_cast<transition::item*> (collidingItem)
                   || qgraphicsitem_cast<port::top_level::item*> (collidingItem)
                   )
                  {
                    graph::item::setPos (old_position);

                    return;
                  }
              }
          }

//           void item::mouseMoveEvent (QGraphicsSceneMouseEvent* event)
//           {
//             if (mode() == style::mode::DRAG)
//               {
//                 setPos (style::raster::snap (pos() + event->pos() - _drag_start));
//                 event->accept();
//                 scene()->update();
//               }
//             else
//               {
//                 connectable::item::mouseMoveEvent (event);
//               }
//           }
//           void item::mousePressEvent (QGraphicsSceneMouseEvent* event)
//           {
//             if (event->modifiers() == Qt::ControlModifier)
//               {
//                 mode_push (style::mode::DRAG);
//                 _drag_start = event->pos();
//                 event->accept();
//                 return;
//               }

//             connectable::item::mousePressEvent (event);
//           }
        }
      }
    }
  }
}
