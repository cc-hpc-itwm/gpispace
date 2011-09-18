#include "GraphTransition.hpp"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QToolButton>
#include <QGraphicsProxyWidget>
#include <QPushButton>

#include "GraphPort.hpp"
#include "GraphStyle.hpp"
#include "GraphTransitionCogWheelButton.hpp"
#include <pnete/ui/GraphConnection.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Transition::Transition(const QString& title, const data::Transition& producedFrom, QGraphicsItem* parent)
      : graph_item(parent)
      , _dragStart(0, 0)
      , _size(160, 100) // hardcoded constant
      , _highlighted(false)
      , _dragging(false)
      , _producedFrom(producedFrom)
      , _menu_context()
      , _title(title)
      {
        new TransitionCogWheelButton(this);
        setAcceptHoverEvents(true);
        setFlag (QGraphicsItem::ItemIsSelectable);
        init_menu_context();
      }

      //! \todo This is ugly and bad. Baaaaaaad.
      Transition* Transition::create_from_library_data (const QByteArray& data)
      {
        QByteArray byteArray(data);
        data::Transition transitionData;
        QDataStream stream(byteArray);
        stream >> transitionData;

        Transition* transition
            (new Transition (transitionData.name(), transitionData));
        foreach (data::Port port, transitionData.inPorts())
        {
          new Port (transition, Port::IN, port.name(), port.type(), port.notConnectable());
        }
        foreach (data::Port port, transitionData.outPorts())
        {
          new Port (transition, Port::OUT, port.name(), port.type(), false);
        }
        transition->repositionChildrenAndResize();

        return transition;
      }

      void Transition::init_menu_context()
      {
        QAction* action_add_port = _menu_context.addAction(tr("Add Port"));
        connect (action_add_port, SIGNAL(triggered()), SLOT(slot_add_port()));

        _menu_context.addSeparator();

        QAction* action_delete = _menu_context.addAction(tr("Delete"));
        connect (action_delete, SIGNAL(triggered()), SLOT(slot_delete()));
      }

      void Transition::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
      {
        if(_dragging)
        {
          _dragging = false;
          event->accept ();
        }
        else
        {
          event->ignore ();
        }
      }

      void Transition::mousePressEvent(QGraphicsSceneMouseEvent * event)
      {
        //! \todo resize grap or move?
        event->accept ();
        _dragStart = event->pos();
        _dragging = true;
      }

      void Transition::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
      {
        if(_dragging)
        {
          //! \note Hackadiddyhack.
          QHash<Connection*, QRectF> connections;
          foreach (QGraphicsItem* item, scene()->items())
          {
            Connection* conn (qgraphicsitem_cast<Connection*> (item));
            if (conn)
            {
              connections.insert (conn, conn->boundingRect());
            }
          }

          const QPointF oldLocation (pos());
          setPos(Style::snapToRaster(oldLocation + event->pos() - _dragStart));

          // do not move, when now colliding with a different transition
          //! \todo move this elsewhere? make this nicer, allow movement to left and right, if collision on bottom.
          //! \todo move to the nearest possible position.
          foreach(QGraphicsItem* collidingItem, collidingItems())
          {
            if(qgraphicsitem_cast<Transition*>(collidingItem))
            {
              setPos(oldLocation);
              event->ignore();
              return;
            }
          }
          event->accept();
          QRectF bounding_with_childs (boundingRect());
          foreach (QGraphicsItem* child, childItems())
          {
            bounding_with_childs =
                bounding_with_childs.united ( child->boundingRect()
                                            .translated (child->pos())
                                            );
          }
          scene()->update (bounding_with_childs.translated (oldLocation));
          scene()->update (bounding_with_childs.translated (pos()));

          //! \note Hackadilicous.
          for ( QHash<Connection*, QRectF>::const_iterator
                it (connections.constBegin())
              , end (connections.constEnd())
              ; it != end
              ; ++it
              )
          {
            if (it.value() != it.key()->boundingRect())
            {
              scene()->update (it.value());
              scene()->update (it.key()->boundingRect());
            }
          }
        }
        else
        {
          event->ignore ();
        }
      }

      void Transition::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
      {
        _highlighted = false;
        update (boundingRect());
      }

      void Transition::hoverEnterEvent(QGraphicsSceneHoverEvent*)
      {
        _highlighted = true;
        update (boundingRect());
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
        QGraphicsItem::contextMenuEvent(event);

        if (!event->isAccepted())
          {
            _menu_context.popup(event->screenPos());
            event->accept();
          }
      }

      void Transition::slot_delete()
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

      void Transition::slot_add_port()
      {
        qDebug() << "Transition::slot_add_port()";
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
