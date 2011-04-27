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
      _size(150, 100),
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
          QList<QGraphicsItem *> collidingPorts = collidingItems();
          for(QList<QGraphicsItem *>::iterator it = collidingPorts.begin(); it != collidingPorts.end(); ++it)
          {
            if(qgraphicsitem_cast<Transition*>(*it))
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
    }
  }
}
