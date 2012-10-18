// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/place.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>

#include <pnete/ui/graph/style/raster.hpp>

#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/transition.hpp>

#include <QPainter>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        place_item::place_item
          ( const data::handle::place& handle
          , boost::optional< ::xml::parse::type::type_map_type&> type_map
          , base_item* parent
          )
            : connectable_item ( connectable::direction::BOTH
                               , type_map
                               , parent
                               , NULL //&handle().prop
                               )
            , _handle (handle)
            , _content()
        {
          refresh_content();
        }

        const data::handle::place& place_item::handle() const
        {
          return _handle;
        }

        const std::string& place_item::we_type() const
        {
          return connectable_item::we_type (_handle().type);
        }

        std::string place_item::name() const
        {
          return _handle().name();
        }

        void place_item::refresh_content()
        {
          _content.setText ( QString::fromStdString (name())
                           + " :: "
                           + QString::fromStdString (we_type())
                           );
        }

        const QStaticText& place_item::content() const
        {
          return _content;
        }
        QSizeF place_item::content_size() const
        {
          return content().size();
        }
        QPointF place_item::content_pos() const
        {
          const QSizeF half_size (content_size() / 2.0);

          return QPointF (-half_size.width(), -half_size.height());
        }

        QPainterPath place_item::shape () const
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

        void place_item::paint ( QPainter* painter
                               , const QStyleOptionGraphicsItem* option
                               , QWidget* widget
                               )
        {
          style::draw_shape (this, painter);

          painter->drawStaticText (content_pos(), content());
        }

        void place_item::setPos (const QPointF& new_position)
        {
          const QPointF old_position (pos());

          base_item::setPos (new_position);

          foreach (QGraphicsItem* collidingItem, collidingItems())
          {
            if (  qgraphicsitem_cast<place_item*> (collidingItem)
               || qgraphicsitem_cast<transition_item*> (collidingItem)
               || qgraphicsitem_cast<top_level_port_item*> (collidingItem)
               )
            {
              base_item::setPos (old_position);

              return;
            }
          }
        }

//           void place_item::mouseMoveEvent (QGraphicsSceneMouseEvent* event)
//           {
//             if (mode() == style::mode::DRAG)
//               {
//                 setPos (style::raster::snap (pos() + event->pos() - _drag_start));
//                 event->accept();
//                 scene()->update();
//               }
//             else
//               {
//                 connectable_item::mouseMoveEvent (event);
//               }
//           }
//           void place_item::mousePressEvent (QGraphicsSceneMouseEvent* event)
//           {
//             if (event->modifiers() == Qt::ControlModifier)
//               {
//                 mode_push (style::mode::DRAG);
//                 _drag_start = event->pos();
//                 event->accept();
//                 return;
//               }

//             connectable_item::mousePressEvent (event);
//           }
      }
    }
  }
}

