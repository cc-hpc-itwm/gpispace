#include <pnete/ui/graph/port.hpp>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <QStaticText>
#include <QDebug>
#include <QMenu>

#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/style.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        port::port
          ( DIRECTION direction
          , boost::optional< ::xml::parse::type::type_map_type&> type_map
          , transition* parent
          )
            : connectable_item (direction, type_map, parent)
            , _name ("<<port>>")
            , _orientation (direction == OUT ? EAST : WEST)
            , _dragging (false)
            , _drag_start (0.0, 0.0)
            , _highlighted (false)
            , _length (style::portDefaultWidth())
            , _menu_context()
        {
          setAcceptHoverEvents (true);
          //! \todo verbose name

          refresh_tooltip();
          connect (this, SIGNAL (we_type_changed()), SLOT (refresh_tooltip()));

          _length = qMax( _length
                        , QStaticText(_name).size().width()
                        + style::portCapLength()
                        + 5.0
                        );

          init_menu_context();
        }

        void port::init_menu_context()
        {
          QAction* action_set_type (_menu_context.addAction(tr("Set type")));
          connect (action_set_type, SIGNAL(triggered()), SLOT(slot_set_type()));

          _menu_context.addSeparator();

          QAction* action_delete (_menu_context.addAction(tr("Delete")));
          connect (action_delete, SIGNAL(triggered()), SLOT(slot_delete()));
        }

        void port::slot_set_type ()
        {
          qDebug() << "port::slot_set_type()";
        }

        void port::slot_delete ()
        {
          //delete_connection();
          //scene()->removeItem (this);
        }

        void port::contextMenuEvent (QGraphicsSceneContextMenuEvent* event)
        {
          item::contextMenuEvent (event);

          if (!event->isAccepted())
          {
            _menu_context.popup (event->screenPos());
            event->accept();
          }
        }

        void port::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
        {
          if (!_dragging)
          {
            connectable_item::mouseReleaseEvent (event);
            return;
          }

          _dragging = false;
          event->accept();
        }

        void port::mousePressEvent (QGraphicsSceneMouseEvent* event)
        {
          if (event->buttons() & Qt::RightButton)
          {
            event->ignore();
            return;
          }

          if (event->modifiers() == Qt::ControlModifier)
          {
            _dragging = true;
            _drag_start = event->pos();
            event->accept();
            return;
          }

          //! \note Only allow one connection on ports.
          if (!_connections.isEmpty())
          {
            event->ignore();
            return;
          }

          connectable_item::mousePressEvent (event);
        }

        bool port::is_connectable_with (const connectable_item* item) const
        {
          //! \note Only allow one connection on ports.
          return _connections.isEmpty()
              && connectable_item::is_connectable_with (item);
        }

        QPointF port::fitting_position (QPointF position)
        {
          if (!parentItem())
          {
            return style::snapToRaster (position);
          }

          const QPointF minimum_distance ( boundingRect().width() / 2.0 + 1.0    // hardcoded constants
                                         , boundingRect().height() / 2.0 + 1.0   // hardcoded constants
                                         );

          const QRectF bounding (parentItem()->boundingRect());

          qreal to_left (position.x() - bounding.left());
          qreal to_right (position.x() - bounding.right());
          qreal to_top (position.y() - bounding.top());
          qreal to_bottom (position.y() - bounding.bottom());
          to_left *= to_left;
          to_right *= to_right;
          to_top *= to_top;
          to_bottom *= to_bottom;

          _orientation = ( qMin (to_top, to_bottom) < qMin (to_left, to_right)
                         ? ( to_top < to_bottom
                           ? NORTH
                           : SOUTH
                           )
                         : ( to_left < to_right
                           ? WEST
                           : EAST
                           )
                         );

          if(_orientation == WEST || _orientation == EAST)
          {
            position.setX ( _orientation == WEST
                          ? bounding.left()
                          : bounding.right()
                          );
            position.setY ( qBound ( bounding.top() + minimum_distance.y()
                                   , position.y()
                                   , bounding.bottom() - minimum_distance.y()
                                   )
                          );
          }
          else
          {
            position.setX ( qBound ( bounding.left() + minimum_distance.x()
                                   , position.x()
                                   , bounding.right() - minimum_distance.x()
                                   )
                          );
            position.setY ( _orientation == NORTH
                          ? bounding.top()
                          : bounding.bottom()
                          );
          }

          return style::snapToRaster (position);
        }

        void port::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
        {
          if(!_dragging)
          {
            connectable_item::mouseMoveEvent (event);
            return;
          }

          const QPointF old_location (pos());
          const ORIENTATION old_orientation (_orientation);
          const QPointF new_location (pos() + event->pos() - _drag_start);

          setPos (fitting_position (new_location));

          // do not move, when now colliding with a different port
          foreach (QGraphicsItem* collidingItem, collidingItems())
          {
            if ( qgraphicsitem_cast<port*>(collidingItem)
              && collidingItem->parentItem() == parentItem()
               )
            {
              _orientation = old_orientation;
              setPos (old_location);
              event->ignore();
              return;
            }
          }
          event->accept();
          scene()->update (boundingRect().translated (old_location));
          scene()->update (boundingRect().translated (pos()));
        }

        void port::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
        {
          _highlighted = false;
          update (boundingRect());
        }
        void port::hoverEnterEvent(QGraphicsSceneHoverEvent *)
        {
          _highlighted = true;
          update (boundingRect());
        }

        QPainterPath port::shape() const
        {
          return style::portShape (this);
        }

        QRectF port::boundingRect() const
        {
          return style::portBoundingRect (this);
        }

        void port::paint ( QPainter *painter
                         , const QStyleOptionGraphicsItem *
                         , QWidget *
                         )
        {
          style::portPaint (painter, this);
        }

        const bool& port::highlighted() const
        {
          return _highlighted;
        }

        const qreal& port::length() const
        {
          return _length;
        }

        const QString& port::name() const
        {
          return _name;
        }
        const QString& port::name (const QString& name_)
        {
          return _name = name_;
        }

        const port::ORIENTATION& port::orientation() const
        {
          return _orientation;
        }
        const port::ORIENTATION&
        port::orientation (const ORIENTATION& orientation_)
        {
          return _orientation = orientation_;
        }

        // void port::delete_connection()
        // {
        //   if(_connection)
        //   {
        //     class connection* const backup (_connection);
        //     const QRectF area (backup->boundingRect());
        //     backup->start (NULL);
        //     backup->end (NULL);
        //     delete backup;
        //     scene()->update (area);
        //   }
        // }

        void port::refresh_tooltip()
        {
          setToolTip (_name + " :: " + we_type());
        }
      }
    }
  }
}
