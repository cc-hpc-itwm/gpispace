#include <pnete/ui/graph/transition.hpp>

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

#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/style.hpp>
#include <pnete/ui/graph/cogwheel_button.hpp>
#include <pnete/ui/graph/connection.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        transition::transition( transition_type & data
                              , QGraphicsItem* parent
                              )
          : item(parent)
          , _dragStart(0, 0)
          , _size(160, 100) // hardcoded constant
          , _highlighted(false)
          , _dragging(false)
          , _data (data)
          , _menu_context()
          , _name()
          , _proxy (NULL)
        {
          new cogwheel_button(this);
          setAcceptHoverEvents(true);
          setFlag (QGraphicsItem::ItemIsSelectable);
          init_menu_context();
        }

        transition::transition ( const QString& filename
                               , QGraphicsItem* parent
                               )
          : item (parent)
          , _dragStart (0, 0)
          , _size (160, 100) // hardcoded constant
          , _highlighted (false)
          , _dragging (false)
            //! \todo BIG UGLY FUCKING HACK EVIL DO NOT LOOK AT THIS BUT DELETE
          , _data(*static_cast<transition_type*> (malloc (sizeof (transition_type))))
          , _menu_context()
          , _name()
          , _proxy (NULL)
        {
          //! \todo WORK HERE, everything is missing
        }

        void transition::init_menu_context()
        {
          QAction* action_add_port = _menu_context.addAction(tr("Add Port"));
          connect (action_add_port, SIGNAL(triggered()), SLOT(slot_add_port()));

          _menu_context.addSeparator();

          QAction* action_delete = _menu_context.addAction(tr("Delete"));
          connect (action_delete, SIGNAL(triggered()), SLOT(slot_delete()));
        }

        void transition::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
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

        void transition::mousePressEvent(QGraphicsSceneMouseEvent * event)
        {
          //! \todo resize grap or move?
          event->accept ();
          _dragStart = event->pos();
          _dragging = true;
        }

        void transition::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
        {
          if(_dragging)
          {
            //! \note Hackadiddyhack.
            QHash<connection*, QRectF> connections;
            foreach (QGraphicsItem* item, scene()->items())
            {
              connection* conn (qgraphicsitem_cast<connection*> (item));
              if (conn)
              {
                connections.insert (conn, conn->boundingRect());
              }
            }

            const QPointF oldLocation (pos());
            setPos(style::snapToRaster(oldLocation + event->pos() - _dragStart));

            // do not move, when now colliding with a different transition
            //! \todo move this elsewhere? make this nicer, allow movement to left and right, if collision on bottom.
            //! \todo move to the nearest possible position.
            foreach(QGraphicsItem* collidingItem, collidingItems())
            {
              if(qgraphicsitem_cast<transition*>(collidingItem))
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
            for ( QHash<connection*, QRectF>::const_iterator
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

        void transition::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
        {
          _highlighted = false;
          update (boundingRect());
        }

        void transition::hoverEnterEvent(QGraphicsSceneHoverEvent*)
        {
          _highlighted = true;
          update (boundingRect());
        }

        QPainterPath transition::shape() const
        {
          return style::transitionShape(_size);
        }

        QRectF transition::boundingRect() const
        {
          return style::transitionBoundingRect(_size);
        }

        void transition::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
        {
          style::transitionPaint(painter, this);
        }

        const QString& transition::name() const
        {
          return _name;
        }

        const QString& transition::name(const QString& name_)
        {
          return _name = name_;
        }

        bool transition::highlighted() const
        {
          return _highlighted;
        }

        void transition::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
        {
          QGraphicsItem::contextMenuEvent(event);

          if (!event->isAccepted())
          {
            _menu_context.popup(event->screenPos());
            event->accept();
          }
        }

        void transition::slot_delete()
        {
          foreach(QGraphicsItem* child, childItems())
          {
            port* p (qgraphicsitem_cast<port*>(child));
            if (p)
            {
              p->delete_connection();
            }
          }
          scene()->removeItem(this);
        }

        void transition::slot_add_port()
        {
          qDebug() << "transition::slot_add_port()";
        }

        void transition::repositionChildrenAndResize()
        {
          qreal positionIn, positionOut, positionAny, positionParam;
          positionIn = positionOut = positionAny = style::portDefaultHeight();
          positionParam = boundingRect().width() - style::portDefaultHeight();

          const qreal padding = 10.0;                                             // hardcoded constant

          foreach(QGraphicsItem* child, childItems())
          {
            port* p = qgraphicsitem_cast<port*>(child);
            if(p)
            {
//             if(p->notConnectable())
//             {
//               p->setOrientation(connectable_item::SOUTH);
//               p->setPos(style::snapToRaster(QPointF(positionParam, boundingRect().height())));
//               positionParam -= style::portDefaultHeight() + padding;
//             }
              if(p->direction() == connectable_item::IN)
              {
                p->setOrientation(connectable_item::WEST);
                p->setPos(style::snapToRaster(QPointF(0.0, positionIn)));
                positionIn += style::portDefaultHeight() + padding;
              }
              else if(p->direction() == connectable_item::OUT)
              {
                p->setOrientation(connectable_item::EAST);
                p->setPos(style::snapToRaster(QPointF(boundingRect().width(), positionOut)));
                positionOut += style::portDefaultHeight() + padding;
              }
              else
              {
                p->setOrientation(connectable_item::NORTH);
                p->setPos(style::snapToRaster(QPointF(positionAny, 0.0)));
                positionAny += style::portDefaultHeight() + padding;
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

        data::proxy::type* transition::proxy (data::proxy::type* proxy_)
        {
          return _proxy = proxy_;
        }
        data::proxy::type* transition::proxy () const
        {
          return _proxy;
        }
      }
    }
  }
}
