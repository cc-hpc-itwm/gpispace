// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_PORT_HPP
#define _PNETE_UI_GRAPH_PORT_HPP 1

#include <QObject>
#include <QMenu>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/item.hpp>
#include <pnete/ui/graph/orientation.hpp>

#include <boost/optional.hpp>

#include <xml/parse/types.hpp>

#include <pnete/weaver/weaver.hpp>

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
          class item : public connectable::item
          {
            Q_OBJECT;

          public:
            typedef ITVAL(XMLTYPE(ports_type)) port_type;

            item ( port_type& port
                 , connectable::direction::type direction
                 , boost::optional< ::xml::parse::type::type_map_type&> type_map
                 = boost::none
                 , transition::item* parent = NULL
                 );

            const qreal& length() const;

            const port_type& port () const { return _port; }

            const std::string& name() const;
            const std::string& we_type () const;

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

            void
            set_just_orientation_but_not_in_property (const orientation::type&);

          public slots:
            void slot_set_type();
            void slot_delete();
            void refresh_tooltip();

          protected:

            virtual void contextMenuEvent (QGraphicsSceneContextMenuEvent* event);
            virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* event);
            virtual void mousePressEvent (QGraphicsSceneMouseEvent* event);
            virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);

          private:
            port_type& _port;

            QPointF fitting_position (QPointF position);

            orientation::type _orientation;

            bool _dragging;
            QPointF _drag_start;

            qreal _length;

            QMenu _menu_context;

            void init_menu_context();
          };

          namespace top_level
          {
            class item : public port::item
            {
            public:
              item ( port_type& port
                   , connectable::direction::type direction
                   , boost::optional< ::xml::parse::type::type_map_type&> type_map
                   = boost::none
                   , transition::item* parent = NULL
                   )
                : port::item (port, direction, type_map, parent)
              {}

              enum { Type = top_level_port_graph_type };
              virtual int type() const { return Type; }
            };
          }
        }
      }
    }
  }
}

#endif
