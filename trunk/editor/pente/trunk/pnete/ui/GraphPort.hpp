// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_GRAPH_PORT_HPP
#define _PNETE_GRAPH_PORT_HPP 1

#include <QObject>
#include <QMenu>

#include <pnete/ui/GraphConnectableItem.hpp>
#include <pnete/ui/graph_item.hpp>

class QAction;
class QMenu;
class QGraphicsSceneContextMenuEvent;

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Transition;

      class Port : public ConnectableItem
      {
        Q_OBJECT

        public:
        //        explicit Port ( internal& port, Transition* parent)
          Port (Transition* parent, eDirection direction);

          const bool& highlighted() const;
          const qreal& length() const;

          const QString& name() const;
          const QString& name(const QString&);

          virtual QRectF boundingRect() const;

          void deleteConnection();

          QPointF snapToEdge (const QPointF& position, eOrientation edge) const;
          eOrientation getNearestEdge (const QPointF& position) const;
          QPointF checkForMinimumDistance (const QPointF& position) const;

          enum
          {
            Type = PortType,
          };
          virtual int type() const
          {
            return Type;
          }

        public slots:
          void slot_set_type();
          void slot_delete();
          void refresh_tooltip();

        protected:
          virtual QPainterPath shape() const;
          virtual void paint ( QPainter* painter
                             , const QStyleOptionGraphicsItem* option
                             , QWidget* widget
                             );

          virtual void contextMenuEvent (QGraphicsSceneContextMenuEvent* event);
          virtual void hoverEnterEvent (QGraphicsSceneHoverEvent* event);
          virtual void hoverLeaveEvent (QGraphicsSceneHoverEvent* event);
          virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* event);
          virtual void mousePressEvent (QGraphicsSceneMouseEvent* event);
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);

        private:
          QString _name;

          QPointF _dragStart;

          bool _dragging;
          bool _highlighted;

          qreal _length;

          QMenu _menu_context;

          void init_menu_context();
      };
    }
  }
}

#endif
