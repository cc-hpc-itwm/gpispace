// bernd.loerwald@itwm.fraunhofer.de

#ifndef PNETE_UI_GRAPH_ASSOCIATION_HPP
#define PNETE_UI_GRAPH_ASSOCIATION_HPP

#include <pnete/ui/graph/association.fwd.hpp>

#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/connectable_item.fwd.hpp>

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

          const QList<QPointF>& fixed_points() const;
          const QList<QPointF>& fixed_points (const QList<QPointF>&);

          virtual void mousePressEvent (QGraphicsSceneMouseEvent*);

          virtual QPainterPath shape() const;
          virtual void paint
            (QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

          enum { Type = association_graph_type };
          virtual int type() const { return Type; }

        private:
          connectable_item* _start;
          connectable_item* _end;

          QList<QPointF> _fixed_points;
        };
      }
    }
  }
}

#endif
