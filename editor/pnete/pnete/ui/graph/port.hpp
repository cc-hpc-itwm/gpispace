// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_PORT_HPP
#define _PNETE_UI_GRAPH_PORT_HPP 1

#include <QObject>
#include <QMenu>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/orientation.hpp>

#include <boost/optional.hpp>

#include <xml/parse/types.hpp>

#include <pnete/weaver/weaver.hpp>

class QAction;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QPointF;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class transition_item;

        class port_item : public connectable_item
        {
          Q_OBJECT;

        public:
          typedef ITVAL(XMLTYPE(ports_type)) port_type;

          port_item ( port_type& port
                    , connectable::direction::type direction
                    , boost::optional< ::xml::parse::type::type_map_type&> type_map
                    = boost::none
                    , transition_item* parent = NULL
                    );

          const qreal& length() const;

          const port_type& port () const { return _port; }

          const std::string& name() const;
          const std::string& we_type () const;

          const port::orientation::type& orientation() const;
          const port::orientation::type& orientation(const port::orientation::type&);

          virtual bool is_connectable_with (const connectable_item*) const;

          enum { Type = port_graph_type };
          virtual int type() const { return Type; }

          QRectF bounding_rect(bool cap = true, int cap_factor = 0) const;
          virtual QPainterPath shape() const;
          virtual void
          paint (QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

          void
          set_just_orientation_but_not_in_property (const port::orientation::type&);

          virtual void setPos (const QPointF&);

          void setPos_no_collision_detection (const QPointF&);

        public slots:
          void slot_set_type();
          void refresh_tooltip();

        private:
          port_type& _port;

          QPointF fitting_position (QPointF position);

          port::orientation::type _orientation;

          qreal _length;
        };

        class top_level_port_item : public port_item
        {
        public:
          top_level_port_item ( port_type& port
                              , connectable::direction::type direction
                              , boost::optional< ::xml::parse::type::type_map_type&> type_map
                              = boost::none
                              , transition_item* parent = NULL
                              )
            : port_item (port, direction, type_map, parent)
            {}

          enum { Type = top_level_port_graph_type };
          virtual int type() const { return Type; }
        };
      }
    }
  }
}

#endif
