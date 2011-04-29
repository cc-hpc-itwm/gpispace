#include "Transition.hpp"
#include "Port.hpp"
#include "Style.hpp"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Transition::Transition(const QString& title, QGraphicsItem* parent)
      : QGraphicsItem(parent),
      _title(title),
      _dragStart(0,0),
      _size(160, 100),
      _highlighted(false),
      _dragging(false)
      {
        setAcceptHoverEvents(true);
      }
      
      void Transition::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
      {
        if(_dragging)
        {
          _dragging = false;
          event->setAccepted(true);
        }
      }
      
      void Transition::mousePressEvent(QGraphicsSceneMouseEvent * event)
      {
        //! \todo resize grap or move?
        event->setAccepted(true);
        _dragStart = event->pos();
        _dragging = true;
      }
      
      void Transition::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
      {
        if(_dragging)
        {
          QPointF oldLocation = pos();
          setPos(Style::snapToRaster(oldLocation + event->pos() - _dragStart));
          
          // do not move, when now colliding with a different transition
          //! \todo move this elsewhere? make this nicer, allow movement to left and right, if collision on bottom.
          //! \todo move to the nearest possible position.
          foreach(QGraphicsItem* collidingItem, collidingItems())
          {
            if(qgraphicsitem_cast<Transition*>(collidingItem))
            {
              setPos(oldLocation);
              break;
            }
          }
          scene()->update();
        }
      }
      
      void Transition::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
      {
        _highlighted = false;
        update(boundingRect());
      }
      
      void Transition::hoverEnterEvent(QGraphicsSceneHoverEvent*)
      {
        _highlighted = true;
        update(boundingRect());
      }
      
      QPainterPath Transition::shape() const
      {
        return Style::transitionShape(_size);
      }
      
      QRectF Transition::boundingRect() const
      {
        return Style::transitionBoundingRect(_size);
      }
      
      void Transition::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
      {
        Style::transitionPaint(painter, this);
      }
      
      const QString& Transition::title() const
      {
        return _title;
      }
      
      bool Transition::highlighted() const
      {
        return _highlighted;
      }
      
      void Transition::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
      {
        QMenu* menu = new QMenu();
        menu->addAction(tr("Delete Transition"));
        menu->popup(event->screenPos());
      
        connect(menu, SIGNAL(triggered(QAction *)), this, SLOT(deleteTriggered(QAction *)));
      }
      
      void Transition::deleteTriggered(QAction *)
      {
        foreach(QGraphicsItem* child, childItems())
        {
          Port* port = qgraphicsitem_cast<Port*>(child);
          if(port)
          {
            port->deleteConnection();
          }
        } 
        scene()->removeItem(this);
      }
      
      void Transition::repositionChildrenAndResize()
      {
        qreal positionIn, positionOut, positionAny;
        positionIn = positionOut = positionAny = Style::portDefaultHeight();
        
        foreach(QGraphicsItem* child, childItems())
        {
          Port* port = qgraphicsitem_cast<Port*>(child);
          if(port)
          {
            if(port->direction() == ConnectableItem::IN)
            {
              port->setOrientation(ConnectableItem::WEST);
              port->setPos(Style::snapToRaster(QPointF(0.0, positionIn)));
              positionIn += Style::portDefaultHeight() + 10.0;
            }
            else if(port->direction() == ConnectableItem::OUT)
            {
              port->setOrientation(ConnectableItem::EAST);
              port->setPos(Style::snapToRaster(QPointF(boundingRect().width(), positionOut)));
              positionOut += Style::portDefaultHeight() + 10.0;
            }
            else
            {
              port->setOrientation(ConnectableItem::NORTH);
              port->setPos(Style::snapToRaster(QPointF(positionAny, 0.0)));
              positionAny += Style::portDefaultHeight() + 10.0;
            }
          }
        }
        positionIn -= 10.0;
        positionOut -= 10.0;
        positionAny -= 10.0;
        
        _size = QSizeF(std::max(_size.width(), positionAny), std::max(_size.height(), std::max(positionOut, positionIn)));
      }
    }
  }
}
