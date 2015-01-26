// bernd.loerwald@itwm.fraunhofer.de

#ifndef PNETE_UI_GRAPH_ASSOCIATION_HPP
#define PNETE_UI_GRAPH_ASSOCIATION_HPP

#include <pnete/ui/graph/association.fwd.hpp>

#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/connectable_item.fwd.hpp>

#include <boost/optional.hpp>

#include <QList>
#include <QPointF>
#include <QObject>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPainterPath;
class QGraphicsSceneMouseEvent;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class association : public base_item
        {
          Q_OBJECT;

        public:
          association (connectable_item* start, connectable_item* end);
          ~association();

          connectable_item* start() const;
          connectable_item* end() const;

          QList<QPointF> all_points() const;

          void invert();

          void trigger_geometry_change();

          virtual bool is_movable() const override;

          virtual QPainterPath shape() const override;
          virtual void paint
            (QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

          enum { Type = association_graph_type };
          virtual int type() const override { return Type; }

        private:
          connectable_item* _start;
          connectable_item* _end;
        };
      }
    }
  }
}

#endif
