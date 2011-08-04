#include "GraphTransition.hpp"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QDebug>
#include <QToolButton>
#include <QGraphicsProxyWidget>
#include <QPushButton>

#include "GraphPort.hpp"
#include "GraphStyle.hpp"

#include "ui/PopoverWidgetButton.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Transition::Transition(const QString& title, const data::Transition& producedFrom, QGraphicsItem* parent)
      : QGraphicsItem(parent),
      _title(title),
      _dragStart(0,0),
      _size(160, 100),                                                          // hardcoded constant
      _highlighted(false),
      _dragging(false),
      _producedFrom(producedFrom)
      {
        new TransitionCogWheelButton(this);
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
        qreal positionIn, positionOut, positionAny, positionParam;
        positionIn = positionOut = positionAny = Style::portDefaultHeight();
        positionParam = boundingRect().width() - Style::portDefaultHeight();

        const qreal padding = 10.0;                                             // hardcoded constant

        foreach(QGraphicsItem* child, childItems())
        {
          Port* port = qgraphicsitem_cast<Port*>(child);
          if(port)
          {
            if(port->notConnectable())
            {
              port->setOrientation(ConnectableItem::SOUTH);
              port->setPos(Style::snapToRaster(QPointF(positionParam, boundingRect().height())));
              positionParam -= Style::portDefaultHeight() + padding;
            }
            else if(port->direction() == ConnectableItem::IN)
            {
              port->setOrientation(ConnectableItem::WEST);
              port->setPos(Style::snapToRaster(QPointF(0.0, positionIn)));
              positionIn += Style::portDefaultHeight() + padding;
            }
            else if(port->direction() == ConnectableItem::OUT)
            {
              port->setOrientation(ConnectableItem::EAST);
              port->setPos(Style::snapToRaster(QPointF(boundingRect().width(), positionOut)));
              positionOut += Style::portDefaultHeight() + padding;
            }
            else
            {
              port->setOrientation(ConnectableItem::NORTH);
              port->setPos(Style::snapToRaster(QPointF(positionAny, 0.0)));
              positionAny += Style::portDefaultHeight() + padding;
            }
          }
        }
        positionIn -= padding;
        positionOut -= padding;
        positionAny -= padding;
        positionParam += padding;
        
        _size = QSizeF(std::max(_size.width(), positionAny), std::max(_size.height(), std::max(positionOut, positionIn)));
        
        // is this correct?
        if(positionParam < 0.0)
        {
          _size.setWidth(_size.width() - positionParam);
          repositionChildrenAndResize();
        }
      }
      
      const data::Transition& Transition::producedFrom() const
      {
        return _producedFrom;
      }
    }
  }
}
