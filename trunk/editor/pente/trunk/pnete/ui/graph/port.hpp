// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_PORT_HPP
#define _PNETE_UI_GRAPH_PORT_HPP 1

#include <QObject>
#include <QMenu>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/item.hpp>

#include <boost/optional.hpp>

#include <xml/parse/types.hpp>

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
        namespace transition { class item; }

        namespace port
        {
          namespace orientation
          {
            enum type
              { NORTH
              , EAST
              , SOUTH
              , WEST
              };
          }

          class item : public connectable::item
          {
            Q_OBJECT;

          public:
            item ( DIRECTION direction
                 , boost::optional< ::xml::parse::type::type_map_type&> type_map
                 = boost::none
                 , transition::item* parent = NULL
                 );

            const bool& highlighted() const;
            const qreal& length() const;

            const QString& name() const;
            const QString& name (const QString&);

            const orientation::type& orientation() const;
            const orientation::type& orientation(const orientation::type&);

            virtual bool is_connectable_with (const connectable::item*) const;

            enum { Type = port_graph_type };
            virtual int type() const { return Type; }

            QRectF bounding_rect(bool cap = true, int cap_factor = 0) const;
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

            orientation::type _orientation;

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
}

#endif
