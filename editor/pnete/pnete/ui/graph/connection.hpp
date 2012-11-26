// bernd.loerwald@itwm.fraunhofer.de

#ifndef PNETE_UI_GRAPH_CONNECTION_HPP
#define PNETE_UI_GRAPH_CONNECTION_HPP

#include <pnete/data/handle/connect.hpp>
#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/connectable_item.fwd.hpp>

#include <boost/optional.hpp>

#include <QList>
#include <QPointF>
#include <QRectF>
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
        class connection_item : public base_item
        {
          Q_OBJECT;

        public:
          connection_item ( const boost::optional<data::handle::connect>& handle
                          , bool read = false
                          );
          ~connection_item();

          connectable_item* start() const;
          connectable_item* start (connectable_item*);
          connectable_item* end() const;
          connectable_item* end (connectable_item*);

          connectable_item* non_free_side() const;
          connectable_item* free_side(connectable_item*);

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
          boost::optional<data::handle::connect> _handle;

          connectable_item* _start;
          connectable_item* _end;

          QList<QPointF> _fixed_points;

          bool _read;
        };
      }
    }
  }
}

#endif
