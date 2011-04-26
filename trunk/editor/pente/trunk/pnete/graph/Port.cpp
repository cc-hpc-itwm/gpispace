#include "Port.hpp"
#include "Transition.hpp"
#include "Scene.hpp"
#include "Connection.hpp"
#include "Style.hpp"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Port::Port(Transition* parent, eDirection direction, const QString& title, const QString& dataType)
      : ConnectableItem(direction == OUT ? EAST : WEST, direction, parent),
      _title(title),
      _dataType(dataType),
      _dragStart(0.0f, 0.0f),
      _dragging(false),
      _highlighted(false),
      _length(60.0)
      {
        setAcceptHoverEvents(true);
      }
      
      void Port::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
      {
        if(_dragging)
        {
          _dragging = false;
          event->setAccepted(true);
        }
        else
        {
          // the port starting the connection will receive this if dragged. if clicked, the correct one does.
          QList<QGraphicsItem*> itemsBelow = scene()->items(event->scenePos());
          for(QList<QGraphicsItem *>::iterator it = itemsBelow.begin(); it != itemsBelow.end(); ++it)
          {
            Port* portBelow = qgraphicsitem_cast<Port*>(*it);
            if(portBelow)
            {
              if(portBelow->connectPendingConnection())
              {
                event->setAccepted(true);
                scene()->update();
                break;
              }
            }
          }
        }
      }
      
      bool Port::connectPendingConnection()
      {
        Scene* sceneObject = qobject_cast<Scene*>(scene());
        if(_connection || !sceneObject->isConnectionLooking())
        {
          return false;
        }
        
        if(sceneObject->isConnectionLookingForStart() && _direction == OUT)
        {
          sceneObject->addStartToConnection(this);
        }
        else if(sceneObject->isConnectionLookingForEnd() && _direction == IN)
        {
          sceneObject->addEndToConnection(this);
        }
        
        return true;
      }
      
      bool Port::createPendingConnectionIfPossible()
      {
        //! \todo move this to connectable item, add "direction-any."<
        Scene* sceneObject = qobject_cast<Scene*>(scene());
      
        if(sceneObject->isConnectionLooking())
        {
          return false;
        }
        
        // create new connection
        if(!_connection)
        {
          ConnectableItem* from = _direction == OUT ? this : NULL;
          ConnectableItem* to = _direction == IN ? this : NULL;
          sceneObject->addConnection(from, to);
        }
        // if we have a connection, detach from our port.
        else
        {
          sceneObject->setNewConnection(_connection);
          if(_direction == OUT)
          {
            _connection->setStart(NULL);
          }
          else
          {
            _connection->setEnd(NULL);
          }
        }
        
        return true;
      }
      
      void Port::mousePressEvent(QGraphicsSceneMouseEvent* event)
      {
        //! \todo Do not do this by modifiers but via areas.
        if(event->modifiers() & Qt::ControlModifier)
        {
          _dragging = true;
          _dragStart = event->pos();
        }
        else
        {
          createPendingConnectionIfPossible();
        }
        event->setAccepted(true);
      }
      
      void Port::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
      {
        if(!_dragging)
        {
          return;
        }
        
        QPointF oldLocation = pos();
        eOrientation oldOrientation = _orientation;
        
        QPointF newLocation = pos() + event->pos() - _dragStart;
        
        QSizeF parentSize = parentItem()->boundingRect().size();
        QPointF parentBottomRightPoint = parentItem()->boundingRect().bottomRight();
        
        // decide orientation
        if(newLocation.x() <= 0.0f)
        {
          _orientation = WEST;
        }
        else if(newLocation.x() > parentSize.width())
        {
          _orientation = EAST;
        }
        else if(newLocation.y() <= 0.0f)
        {
          _orientation = NORTH;
        }
        else if(newLocation.y() > parentSize.height())
        {
          _orientation = SOUTH;
        }
        else // somewhere in the middle.
        {
          QPointF diffMax = newLocation - parentBottomRightPoint;
          QPointF distMin = QPointF(newLocation.x() * newLocation.x(), newLocation.y() * newLocation.y());
          QPointF distMax = QPointF(diffMax.x() * diffMax.x(), diffMax.y() * diffMax.y());
          QPointF minimumDistance = QPointF(std::min(distMin.x(), distMax.x()), std::min(distMin.y(), distMax.y()));
          eOrientation horizontal = distMin.x() < distMax.x() ? WEST : EAST; 
          eOrientation vertical = distMin.y() < distMax.y() ? NORTH : SOUTH ; 
          _orientation = minimumDistance.x() < minimumDistance.y() ? horizontal : vertical;
        }
        
        // snap to specific edge
        switch(_orientation)
        {
          case WEST:
            newLocation.setX(0.0f);
            break;
          case EAST:
            newLocation.setX(parentSize.width());
            break;
          case NORTH:
            newLocation.setY(0.0f);
            break;
          case SOUTH:
            newLocation.setY(parentSize.height());
            break;
          case ANYORIENTATION:
            // you're fucked.
            break;
        }
          
        //! \todo Loop over all ports and check if a minimum distance is given.
        
        if(_orientation == WEST || _orientation == EAST)
        {
          const qreal minimumDistance = boundingRect().height() / 2 + 1;
          newLocation.setX(std::max(std::min(parentSize.width(), newLocation.x()), 0.0));
          newLocation.setY(std::max(std::min(parentSize.height() - minimumDistance, newLocation.y()), minimumDistance));
        }
        else
        {
          const qreal minimumDistance = boundingRect().width() / 2 + 1;
          newLocation.setX(std::max(std::min(parentSize.width() - minimumDistance, newLocation.x()), minimumDistance));
          newLocation.setY(std::max(std::min(parentSize.height(), newLocation.y()), 0.0));
        }
        
        setPos(Style::snapToRaster(newLocation));
        
        // do not move, when now colliding with a different port
        QList<QGraphicsItem *> collidingPorts = collidingItems();
        for(QList<QGraphicsItem *>::iterator it = collidingPorts.begin(); it != collidingPorts.end(); ++it)
        {
          //! \todo only if on this transition?
          if(qgraphicsitem_cast<Port*>(*it))
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
      
      const QString& Port::title() const
      {
        return _title;
      }
      const QString& Port::dataType() const
      {
        return _dataType;
      }
    }
  }
}
