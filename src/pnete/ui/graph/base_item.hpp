// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <pnete/ui/graph/base_item.fwd.hpp>

#include <pnete/data/handle/base.fwd.hpp>
#include <pnete/ui/graph/mode.hpp>
#include <pnete/ui/graph/orientation.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/style/type.hpp>

#include <we/type/property.hpp>

#include <QGraphicsObject>
#include <QLinkedList>
#include <QPointF>
#include <QRectF>
#include <QStack>

class QGraphicsSceneHoverEvent;
class QGraphicsSceneMouseEvent;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class base_item : public QGraphicsObject
        {
          Q_OBJECT;

        public:
          enum item_types
          {
            connection_graph_type = QGraphicsItem::UserType + 1,
            port_graph_type,
            transition_graph_type,
            place_graph_type,
            top_level_port_graph_type,
            pending_connection_graph_type,
            association_graph_type,
            port_place_association_graph_type,
            place_map_graph_type,
          };

          base_item (base_item* parent = nullptr);

          scene_type* scene() const;

          void set_just_pos_but_not_in_property (qreal x, qreal y);
          void set_just_pos_but_not_in_property (const QPointF&);

          virtual void set_just_orientation_but_not_in_property
          (const port::orientation::type&) {}

          virtual void setPos (const QPointF&);
          virtual void setPos (const QPointF&, bool outer);

          virtual void no_undo_setPos (const QPointF&);

          void no_undo_no_raster_setPos (qreal x, qreal y);
          virtual void no_undo_no_raster_setPos (const QPointF&);

          void clear_style_cache();

          template<typename T>
            T style (const style::key_type& key) const
          {
            return _style.get<T> (this, mode(), key);
          }

          const mode::type& mode() const;
          void mode_push (const mode::type&);
          void mode_pop();

          virtual QLinkedList<base_item*> childs() const;
          virtual QPainterPath shape() const = 0;
          virtual QRectF boundingRect() const override;

          virtual void handle_property_change
            ( const ::we::type::property::path_type& path
            , const ::we::type::property::value_type& value
            );

          virtual bool is_movable() const;

        protected:
          style::type _style;
          QStack<mode::type> _mode;
          QPointF _move_start;

          virtual void hoverEnterEvent (QGraphicsSceneHoverEvent* event) override;
          virtual void hoverLeaveEvent (QGraphicsSceneHoverEvent* event) override;
          virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* event) override;
          virtual void mousePressEvent (QGraphicsSceneMouseEvent* event) override;
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event) override;

          virtual const data::handle::base& handle() const;
        };
      }
    }
  }
}
