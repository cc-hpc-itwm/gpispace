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
        port::port ( transition* parent
                   , eDirection direction
                   )
          : connectable_item (direction == OUT ? EAST : WEST, direction, parent)
          , _name ("<<port>>")
          , _dragStart (0.0, 0.0)
          , _dragging (false)
          , _highlighted (false)
          , _length (style::portDefaultWidth())
          , _menu_context()
        {
          setAcceptHoverEvents (true);
          //! \todo verbose name

          refresh_tooltip();
          connect (this, SIGNAL (we_type_changed()), SLOT (refresh_tooltip()));

          _length = std::max(_length, QStaticText(_name).size().width() + style::portCapLength() + 5.0);

          init_menu_context();
        }

        void port::init_menu_context()
        {
          QAction* action_set_type = _menu_context.addAction(tr("Set type"));
          connect (action_set_type, SIGNAL(triggered()), SLOT(slot_set_type()));

          _menu_context.addSeparator();

          QAction* action_delete = _menu_context.addAction(tr("Delete"));
          connect (action_delete, SIGNAL(triggered()), SLOT(slot_delete()));
        }

        void port::slot_set_type ()
        {
          qDebug() << "port::slot_set_type()";
        }

        void port::slot_delete ()
        {
          delete_connection();
          scene()->removeItem(this);
        }

        void port::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
        {
          QGraphicsItem::contextMenuEvent (event);

          if (!event->isAccepted ())
          {
            _menu_context.popup (event->screenPos ());
            event->accept ();
          }
        }

        void port::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
        {
          if (_dragging)
          {
            _dragging = false;
            event->accept();
          }
          else
          {
            event->ignore();
          }
        }

        void port::mousePressEvent (QGraphicsSceneMouseEvent* event)
        {
          if (!(event->buttons() & Qt::RightButton))
          {
            switch (style::portHit (this, event->pos ()))
            {
            case style::MAIN:
              _dragging = true;
              _dragStart = event->pos();
              event->accept();
              break;

            case style::TAIL:
            default:
              //! \note Maybe not needed and checked somewhere else.
              event->setAccepted (createPendingConnectionIfPossible ());
              break;
            }
          }
          else
          {
            event->ignore();
          }
        }

        QPointF
        port::snap_to_edge (QPointF position, eOrientation edge) const
        {
          if (!parentItem())
          {
            return position;
          }

          switch (_orientation)
          {
          case WEST:
            position.setX (0.0);
            break;
          case EAST:
            position.setX (parentItem()->boundingRect().width());
            break;
          case NORTH:
            position.setY (0.0);
            break;
          case SOUTH:
            position.setY (parentItem()->boundingRect().height());
            break;
          case ANYORIENTATION:
            // you're fucked.
            break;
          }

          return position;
        }

        connectable_item::eOrientation
        port::get_nearest_edge (const QPointF& position) const
        {
          if (!parentItem())
          {
            return _orientation;
          }

          if (position.x() <= 0.0)
          {
            return WEST;
          }
          else if (position.x() > parentItem()->boundingRect().width())
          {
            return EAST;
          }
          else if (position.y() <= 0.0)
          {
            return NORTH;
          }
          else if (position.y() > parentItem()->boundingRect().height())
          {
            return SOUTH;
          }
          else
          {
            const QPointF parent_bottom_right
              (parentItem()->boundingRect().bottomRight());

            const qreal to_top (position.y());
            const qreal to_left (position.x());
            const qreal to_bottom (position.y() - parent_bottom_right.y());
            const qreal to_right (position.x() - parent_bottom_right.x());

            return ( qMin (to_top, to_left) < qMin (to_left, to_right)
                   ? ( to_top < to_bottom
                     ? NORTH
                     : SOUTH
                     )
                   : ( to_left < to_right
                     ? WEST
                     : EAST
                     )
                   );
          }
        }

        QPointF port::check_for_minimum_distance (const QPointF& position) const
        {
          if (!parentItem())
          {
            return position;
          }

          const QSizeF parentSize (parentItem()->boundingRect().size());

          if(_orientation == WEST || _orientation == EAST)
          {
            const qreal minimumDistance (boundingRect().height() / 2.0 + 1.0);    // hardcoded constant

            return QPointF ( qBound ( 0.0
                                    , position.x()
                                    , parentSize.width()
                                    )
                           , qBound ( minimumDistance
                                    , position.y()
                                    , parentSize.height() - minimumDistance
                                    )
                           );
          }
          else
          {
            const qreal minimumDistance (boundingRect().width() / 2.0 + 1.0);     // hardcoded constant

            return QPointF ( qBound ( minimumDistance
                                    , position.x()
                                    , parentSize.width() - minimumDistance
                                    )
                           , qBound ( 0.0
                                    , position.y()
                                    , parentSize.height()
                                    )
                           );
          }
        }

        void port::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
        {
          if(!_dragging)
          {
            event->ignore();
            return;
          }

          const QPointF old_location (pos());
          const eOrientation old_orientation (_orientation);
          const QPointF new_location (pos() + event->pos() - _dragStart);

          _orientation = get_nearest_edge (new_location);

          setPos ( style::snapToRaster
                   ( check_for_minimum_distance
                     (snap_to_edge (new_location, _orientation))
                   )
                 );

          // do not move, when now colliding with a different port
          foreach (QGraphicsItem* collidingItem, collidingItems())
          {
            if ( qgraphicsitem_cast<port*>(collidingItem)
              && collidingItem->parentItem() == parentItem()
               )
            {
              _orientation = old_orientation;
              setPos(old_location);
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
          return style::portShape(this);
        }

        QRectF port::boundingRect() const
        {
          return style::portBoundingRect(this);
        }

        void port::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
        {
          style::portPaint(painter, this);
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
        const QString& port::name(const QString& name_)
        {
          return _name = name_;
        }

        void port::delete_connection()
        {
          if(_connection)
          {
            class connection* const backup (_connection);
            const QRectF area (backup->boundingRect());
            backup->setStart (NULL);
            backup->setEnd (NULL);
            delete backup;
            scene()->update (area);
          }
        }

        void port::refresh_tooltip()
        {
          setToolTip (_name + " :: " + we_type());
        }
      }
    }
  }
}
