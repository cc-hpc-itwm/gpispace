// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_PORT_HPP
#define _PNETE_UI_GRAPH_PORT_HPP 1

#include <QObject>
#include <QMenu>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/item.hpp>

class QAction;
class QMenu;
class QGraphicsSceneContextMenuEvent;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class transition;

        class port : public connectable_item
        {
          Q_OBJECT;

        public:
          enum ORIENTATION
          {
            NORTH,
            EAST,
            SOUTH,
            WEST,
          };

          port (DIRECTION direction, transition* parent);

          const bool& highlighted() const;
          const qreal& length() const;

          const QString& name() const;
          const QString& name (const QString&);

          const ORIENTATION& orientation() const;
          const ORIENTATION& orientation(const ORIENTATION&);

          virtual bool is_connectable_with (const connectable_item*) const;

          enum { Type = port_graph_type };
          virtual int type() const { return Type; }

          virtual QRectF boundingRect() const;
          virtual QPainterPath shape() const;
          virtual void
          paint (QPainter*, const QStyleOptionGraphicsItem*, QWidget*);


        public slots:
          void slot_set_type();
          void slot_delete();
          void refresh_tooltip();

        protected:

          virtual void contextMenuEvent (QGraphicsSceneContextMenuEvent* event);
          virtual void hoverEnterEvent (QGraphicsSceneHoverEvent* event);
          virtual void hoverLeaveEvent (QGraphicsSceneHoverEvent* event);
          virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* event);
          virtual void mousePressEvent (QGraphicsSceneMouseEvent* event);
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);

        private:
          QPointF fitting_position (QPointF position);

          QString _name;

          ORIENTATION _orientation;

          bool _dragging;
          QPointF _drag_start;

          bool _highlighted;

          qreal _length;

          QMenu _menu_context;

          void init_menu_context();
        };
      }
    }
  }
}

#endif
