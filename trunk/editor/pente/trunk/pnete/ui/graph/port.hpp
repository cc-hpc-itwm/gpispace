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
          //        explicit port ( internal& port, Transition* parent)
          port (transition* parent, eDirection direction);

          const bool& highlighted() const;
          const qreal& length() const;

          const QString& name() const;
          const QString& name (const QString&);

          virtual QRectF boundingRect() const;

          void delete_connection();

          QPointF snap_to_edge (QPointF position, eOrientation edge) const;
          eOrientation get_nearest_edge (const QPointF& position) const;
          QPointF check_for_minimum_distance (const QPointF& position) const;

          enum
          {
            Type = port_graph_type,
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
}

#endif
