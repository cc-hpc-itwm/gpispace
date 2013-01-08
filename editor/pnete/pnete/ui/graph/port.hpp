// bernd.loerwald@itwm.fraunhofer.de

#ifndef PNETE_UI_GRAPH_PORT_HPP
#define PNETE_UI_GRAPH_PORT_HPP

#include <pnete/ui/graph/port.fwd.hpp>

#include <pnete/data/handle/port.hpp>
#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/orientation.hpp>
#include <pnete/ui/graph/transition.fwd.hpp>

#include <boost/optional.hpp>

#include <QObject>
#include <QMenu>

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
        class port_item : public connectable_item
        {
          Q_OBJECT;

        public:
          port_item ( const data::handle::port& handle
                    , transition_item* parent = NULL
                    );

          const qreal& length() const;

          virtual const data::handle::port& handle() const;

          const std::string& name() const;
          const std::string& we_type () const;

          port::orientation::type orientation() const;
          const port::orientation::type& orientation(const port::orientation::type&);

          virtual bool is_connectable_with (const connectable_item*) const;

          enum { Type = port_graph_type };
          virtual int type() const { return Type; }

          virtual void add_cap_for_direction (QPolygonF*, const QPointF&) const;

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
          void refresh_content();

          void type_or_name_changed
            (const QObject*, const data::handle::port&, const QString&);

          void property_changed
            ( const QObject* origin
            , const data::handle::port& changed_handle
            , const we::type::property::key_type& key
            , const we::type::property::value_type& value
            );

        private:
          data::handle::port _handle;

          QPointF fitting_position (QPointF position);

          port::orientation::type _orientation;

          qreal _length;
        };

        class top_level_port_item : public port_item
        {
          Q_OBJECT;

        public:
          top_level_port_item ( const data::handle::port& handle
                              , transition_item* parent = NULL
                              )
            : port_item (handle, parent)
            {}

          virtual void add_cap_for_direction (QPolygonF*, const QPointF&) const;

          enum { Type = top_level_port_graph_type };
          virtual int type() const { return Type; }
        };
      }
    }
  }
}

#endif
