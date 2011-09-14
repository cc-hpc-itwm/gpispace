#include "GraphPort.hpp"
#include "GraphTransition.hpp"
#include "GraphScene.hpp"
#include "GraphConnection.hpp"
#include "GraphStyle.hpp"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <QStaticText>
#include <QDebug>
#include <QMenu>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Port::Port ( Transition* parent
                 , eDirection direction
                 , const QString& title
                 , const QString& dataType
                 , bool notConnectable
                 )
      : ConnectableItem (direction == OUT ? EAST : WEST, direction, parent)
      , _title (title)
      , _dataType (dataType)
      , _dragStart (0.0, 0.0)
      , _dragging (false)
      , _highlighted (false)
      , _notConnectable (notConnectable)
      , _length (Style::portDefaultWidth())
      , _menu_context()
      {
        setAcceptHoverEvents (true);
        //! \todo verbose name
        setToolTip(dataType);

        _length = std::max(_length, QStaticText(_title).size().width() + Style::portCapLength() + 5.0);

        init_menu_context();
      }

      void Port::init_menu_context()
      {
        QAction* action_set_type = _menu_context.addAction(tr("Set type"));
        connect (action_set_type, SIGNAL(triggered()), SLOT(slot_set_type()));

        _menu_context.addSeparator();

        QAction* action_delete = _menu_context.addAction(tr("Delete"));
        connect (action_delete, SIGNAL(triggered()), SLOT(slot_delete()));
      }

      void Port::slot_set_type ()
      {
        qDebug() << "Port::slot_set_type()";
      }

      void Port::slot_delete ()
      {
        deleteConnection();
        scene()->removeItem(this);
      }

      void Port::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
      {
        QGraphicsItem::contextMenuEvent (event);

        if (!event->isAccepted ())
        {
          _menu_context.popup (event->screenPos ());
          event->accept ();
        }
      }

      void Port::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
      {
        if (_dragging)
        {
          _dragging = false;
          event->accept ();
        }
        else
        {
          event->ignore ();
        }
      }

      void Port::mousePressEvent(QGraphicsSceneMouseEvent* event)
      {
        if (!(event->buttons() & Qt::RightButton))
        {
          switch (Style::portHit (this, event->pos ()))
          {
          case Style::MAIN:
            _dragging = true;
            _dragStart = event->pos ();
            event->accept ();
            break;

          case Style::TAIL:
          default:
            //! \note Maybe not needed and checked somewhere else.
            if (!notConnectable ())
            {
              event->setAccepted (createPendingConnectionIfPossible ());
            }
            else
            {
              event->ignore();
            }
            break;
          }
        }
      }

      QPointF Port::snapToEdge(const QPointF& position, eOrientation edge) const
      {
        QSizeF parentSize = parentItem()->boundingRect().size();

        QPointF newPosition = position;

        switch(_orientation)
        {
          case WEST:
            newPosition.setX(0.0);
            break;
          case EAST:
            newPosition.setX(parentSize.width());
            break;
          case NORTH:
            newPosition.setY(0.0);
            break;
          case SOUTH:
            newPosition.setY(parentSize.height());
            break;
          case ANYORIENTATION:
            // you're fucked.
            break;
        }

        return newPosition;
      }

      ConnectableItem::eOrientation Port::getNearestEdge(const QPointF& position) const
      {
        QSizeF parentSize = parentItem()->boundingRect().size();
        QPointF parentBottomRightPoint = parentItem()->boundingRect().bottomRight();

        eOrientation orientation;

        if(position.x() <= 0.0)
        {
          orientation = WEST;
        }
        else if(position.x() > parentSize.width())
        {
          orientation = EAST;
        }
        else if(position.y() <= 0.0)
        {
          orientation = NORTH;
        }
        else if(position.y() > parentSize.height())
        {
          orientation = SOUTH;
        }
        else
        {
          QPointF diffMax = position - parentBottomRightPoint;
          QPointF distMin = QPointF(position.x() * position.x(), position.y() * position.y());
          QPointF distMax = QPointF(diffMax.x() * diffMax.x(), diffMax.y() * diffMax.y());
          QPointF minimumDistance = QPointF(std::min(distMin.x(), distMax.x()), std::min(distMin.y(), distMax.y()));
          eOrientation horizontal = distMin.x() < distMax.x() ? WEST : EAST;
          eOrientation vertical = distMin.y() < distMax.y() ? NORTH : SOUTH ;
          orientation = minimumDistance.x() < minimumDistance.y() ? horizontal : vertical;
        }
        return orientation;
      }

      QPointF Port::checkForMinimumDistance(const QPointF& position) const
      {
        QSizeF parentSize = parentItem()->boundingRect().size();

        QPointF newPosition = position;

        if(_orientation == WEST || _orientation == EAST)
        {
          const qreal minimumDistance = boundingRect().height() / 2.0 + 1.0;    // hardcoded constant
          newPosition.setX(std::max(std::min(parentSize.width(), newPosition.x()), 0.0));
          newPosition.setY(std::max(std::min(parentSize.height() - minimumDistance, newPosition.y()), minimumDistance));
        }
        else
        {
          const qreal minimumDistance = boundingRect().width() / 2.0 + 1.0;     // hardcoded constant
          newPosition.setX(std::max(std::min(parentSize.width() - minimumDistance, newPosition.x()), minimumDistance));
          newPosition.setY(std::max(std::min(parentSize.height(), newPosition.y()), 0.0));
        }

        return newPosition;
      }

      void Port::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
      {
        if(!_dragging)
        {
          event->ignore();
          return;
        }
        event->accept();

        QPointF oldLocation = pos();
        eOrientation oldOrientation = _orientation;

        QPointF newLocation = pos() + event->pos() - _dragStart;

        _orientation = getNearestEdge(newLocation);
        newLocation = snapToEdge(newLocation, _orientation);
        newLocation = checkForMinimumDistance(newLocation);

        setPos(Style::snapToRaster(newLocation));

        // do not move, when now colliding with a different port
        foreach(QGraphicsItem* collidingItem, collidingItems())
        {
          if(qgraphicsitem_cast<Port*>(collidingItem) && collidingItem->parentItem() == parentItem())
          {
            _orientation = oldOrientation;
            setPos(oldLocation);
            break;
          }
        }
        scene()->update();
      }

      void Port::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
      {
        _highlighted = false;
        update(boundingRect());
      }
      void Port::hoverEnterEvent(QGraphicsSceneHoverEvent *)
      {
        _highlighted = true;
        update(boundingRect());
      }

      QPainterPath Port::shape() const
      {
        return Style::portShape(this);
      }

      QRectF Port::boundingRect() const
      {
        return Style::portBoundingRect(this);
      }

      void Port::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
      {
        Style::portPaint(painter, this);
      }

      const bool& Port::highlighted() const
      {
        return _highlighted;
      }

      const qreal& Port::length() const
      {
        return _length;
      }

      const bool& Port::notConnectable() const
      {
        return _notConnectable;
      }

      const QString& Port::title() const
      {
        return _title;
      }
      const QString& Port::dataType() const
      {
        return _dataType;
      }

      void Port::deleteConnection()
      {
        if(_connection)
        {
          Connection* backup = _connection;
          backup->setStart(NULL);
          backup->setEnd(NULL);
          delete backup;
          scene()->update();
        }
      }

      bool Port::canConnectTo(ConnectableItem* other) const
      {
        Port* otherPort = qgraphicsitem_cast<Port*>(other);
        return !notConnectable() && otherPort
               && otherPort->dataType() == dataType()
               && otherPort->direction() != direction()
               && otherPort->parentItem() != parentItem();
      }

      bool Port::canConnectIn(eDirection thatDirection) const
      {
        return !notConnectable() && thatDirection == direction();
      }
    }
  }
}
