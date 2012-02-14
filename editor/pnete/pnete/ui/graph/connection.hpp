// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_CONNECTION_HPP
#define _PNETE_UI_GRAPH_CONNECTION_HPP 1

#include <QList>
#include <QPointF>
#include <QRectF>
#include <QObject>

#include <pnete/ui/graph/item.hpp>

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
        class connectable_item;

        namespace connection
        {
          class item : public graph::item
          {
            Q_OBJECT;

          public:
            item (bool read = false);
            ~item();

            connectable::item* start() const;
            connectable::item* start (connectable::item*);
            connectable::item* end() const;
            connectable::item* end (connectable::item*);

            connectable::item* non_free_side() const;
            connectable::item* free_side(connectable::item*);

            const QList<QPointF>& fixed_points() const;
            const QList<QPointF>& fixed_points (const QList<QPointF>&);

            const bool& read() const;
            const bool& read (const bool&);

            virtual QPainterPath shape() const;
            virtual void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

            enum { Type = connection_graph_type };
            virtual int type() const { return Type; }

            virtual void mousePressEvent (QGraphicsSceneMouseEvent*);

          private:
            connectable::item* _start;
            connectable::item* _end;

            QList<QPointF> _fixed_points;

            bool _read;
          };
        }
      }
    }
  }
}

#endif
