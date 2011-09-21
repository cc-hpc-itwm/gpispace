#ifndef GRAPHCONNECTION_H
#define GRAPHCONNECTION_H 1

#include <QGraphicsItem>
#include <QPointF>
#include <QList>

#include <pnete/ui/graph/item.hpp>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPainterPath;
class QRectF;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class connectable_item;

        class connection : public item
        {
        public:
          connection (bool reading_only = false);

          void setStart(connectable_item* start);
          void setEnd(connectable_item* end);

          void removeMe(connectable_item* item);

          const connectable_item* start() const;
          const connectable_item* end() const;

          const QPointF startPosition() const;
          const QPointF endPosition() const;

          const QList<QPointF>& midpoints() const;
          const bool& highlighted() const;

          enum
          {
            Type = connection_graph_type,
          };
          virtual int type() const
          {
            return Type;
          }

          virtual QRectF boundingRect() const;
        protected:
          virtual QPainterPath shape() const;
          virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        private:
          void recalcMidpoints();

          connectable_item* _start;
          connectable_item* _end;

          QList<QPointF> _midpoints;

          bool _highlighted;
          bool _reading_only;
        };
      }
    }
  }
}

#endif
