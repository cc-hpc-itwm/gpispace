// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

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

#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/ui/graph/style/size.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace port
        {
          item::item
          ( DIRECTION direction
          , boost::optional< ::xml::parse::type::type_map_type&> type_map
          , transition::item* parent
          )
            : connectable_item (direction, type_map, parent)
            , _name ("<<port>>")
            , _orientation ( direction == OUT
                           ? orientation::EAST
                           : orientation::WEST
                           )
            , _dragging (false)
            , _drag_start (0.0, 0.0)
            , _highlighted (false)
            , _length (size::port::width())
            , _menu_context()
          {
            setAcceptHoverEvents (true);
            //! \todo verbose name

            refresh_tooltip();
            connect (this, SIGNAL (we_type_changed()), SLOT (refresh_tooltip()));

            _length = qMax( _length
                          , QStaticText(_name).size().width()
                          + size::cap::length()
                          + 5.0 // hardcoded constant
                          );

            init_menu_context();
          }

          void item::init_menu_context()
          {
            QAction* action_set_type (_menu_context.addAction(tr("Set type")));
            connect (action_set_type, SIGNAL(triggered()), SLOT(slot_set_type()));

            _menu_context.addSeparator();

            QAction* action_delete (_menu_context.addAction(tr("Delete")));
            connect (action_delete, SIGNAL(triggered()), SLOT(slot_delete()));
          }

          void item::slot_set_type ()
          {
            qDebug() << "port::slot_set_type()";
          }

          void item::slot_delete ()
          {
            //delete_connection();
            //scene()->removeItem (this);
          }

          void item::contextMenuEvent (QGraphicsSceneContextMenuEvent* event)
          {
            item::contextMenuEvent (event);

            if (!event->isAccepted())
              {
                _menu_context.popup (event->screenPos());
                event->accept();
              }
          }

          void item::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
          {
            if (!_dragging)
              {
                connectable_item::mouseReleaseEvent (event);
                return;
              }

            _dragging = false;
            event->accept();
          }

          void item::mousePressEvent (QGraphicsSceneMouseEvent* event)
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

          bool item::is_connectable_with (const connectable_item* item) const
          {
            //! \note Only allow one connection on ports.
            return _connections.isEmpty()
              && connectable_item::is_connectable_with (item);
          }

          QPointF item::fitting_position (QPointF position)
          {
            if (!parentItem())
              {
                return style::raster::snap (position);
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
                             ? orientation::NORTH
                             : orientation::SOUTH
                             )
                           : ( to_left < to_right
                             ? orientation::WEST
                             : orientation::EAST
                             )
                           );

            if(  _orientation == orientation::WEST
              || _orientation == orientation::EAST
              )
              {
                position.setX ( _orientation == orientation::WEST
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
                position.setY ( _orientation == orientation::NORTH
                              ? bounding.top()
                              : bounding.bottom()
                              );
              }

            return style::raster::snap (position);
          }

          void item::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
          {
            if(!_dragging)
              {
                connectable_item::mouseMoveEvent (event);
                return;
              }

            const QPointF old_location (pos());
            const orientation::type old_orientation (_orientation);
            const QPointF new_location (pos() + event->pos() - _drag_start);

            setPos (fitting_position (new_location));

            // do not move, when now colliding with a different port
            foreach (QGraphicsItem* collidingItem, collidingItems())
              {
                if ( qgraphicsitem_cast<item*>(collidingItem)
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

          void item::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
          {
            _highlighted = false;
            update (boundingRect());
          }
          void item::hoverEnterEvent(QGraphicsSceneHoverEvent *)
          {
            _highlighted = true;
            update (boundingRect());
          }

          const bool& item::highlighted() const
          {
            return _highlighted;
          }

          const qreal& item::length() const
          {
            return _length;
          }

          const QString& item::name() const
          {
            return _name;
          }
          const QString& item::name (const QString& name_)
          {
            return _name = name_;
          }

          const orientation::type&
          item::orientation() const
          {
            return _orientation;
          }
          const orientation::type&
          item::orientation (const orientation::type& orientation_)
          {
            return _orientation = orientation_;
          }

          // void item::delete_connection()
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

          void item::refresh_tooltip()
          {
            setToolTip (_name + " :: " + we_type());
          }
        }
      }
    }
  }
}
