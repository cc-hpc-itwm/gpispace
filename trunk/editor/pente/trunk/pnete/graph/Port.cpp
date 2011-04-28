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
      _dragStart(0.0, 0.0),
      _dragging(false),
      _highlighted(false),
      _length(Style::portDefaultWidth())
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
      }
      
      void Port::mousePressEvent(QGraphicsSceneMouseEvent* event)
      {
        switch(Style::portHit(this, event->pos()))
        {
          case Style::MAIN:
            _dragging = true;
            _dragStart = event->pos();
            event->setAccepted(true);
            break;
            
          case Style::TAIL:
          default:
            event->setAccepted(createPendingConnectionIfPossible());
            break;
        }
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
        if(newLocation.x() <= 0.0)
        {
          _orientation = WEST;
        }
        else if(newLocation.x() > parentSize.width())
        {
          _orientation = EAST;
        }
        else if(newLocation.y() <= 0.0)
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
            newLocation.setX(0.0);
            break;
          case EAST:
            newLocation.setX(parentSize.width());
            break;
          case NORTH:
            newLocation.setY(0.0);
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
          const qreal minimumDistance = boundingRect().height() / 2.0 + 1.0;
          newLocation.setX(std::max(std::min(parentSize.width(), newLocation.x()), 0.0));
          newLocation.setY(std::max(std::min(parentSize.height() - minimumDistance, newLocation.y()), minimumDistance));
        }
        else
        {
          const qreal minimumDistance = boundingRect().width() / 2.0 + 1.0;
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
        return otherPort 
               && otherPort->dataType() == dataType() 
               && otherPort->direction() != direction() 
               && otherPort->parentItem() != parentItem();
      }
      
      bool Port::canConnectIn(eDirection thatDirection) const
      {
        return thatDirection == direction();
      }
    }
  }
}
