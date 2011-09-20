#ifndef GRAPHCONNECTION_H
#define GRAPHCONNECTION_H 1

#include <QGraphicsItem>
#include <QPointF>
#include <QList>

#include "graph_item.hpp"

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPainterPath;
class QRectF;

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class ConnectableItem;

      class Connection : public graph_item
      {
      public:
        Connection();

        void setStart(ConnectableItem* start);
        void setEnd(ConnectableItem* end);

        void removeMe(ConnectableItem* item);

        const ConnectableItem* start() const;
        const ConnectableItem* end() const;

        const QPointF startPosition() const;
        const QPointF endPosition() const;

        const QList<QPointF>& midpoints() const;
        const bool& highlighted() const;

        enum
          {
            Type = ConnectionType,
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

        ConnectableItem* _start;
        ConnectableItem* _end;

        QList<QPointF> _midpoints;

        bool _highlighted;
      };
    }
  }
}

#endif
