// bernd.loerwald@itwm.fraunhofer.de

#ifndef PNETE_UI_GRAPH_CONNECTION_HPP
#define PNETE_UI_GRAPH_CONNECTION_HPP

#include <pnete/ui/graph/connection.fwd.hpp>

#include <pnete/data/handle/connect.hpp>
#include <pnete/ui/graph/association.hpp>
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
        class connection_item : public association
        {
          Q_OBJECT;

        public:
          connection_item ( connectable_item* start
                          , connectable_item* end
                          , const boost::optional<data::handle::connect>& handle
                          , bool read = false
                          );

          const bool& read() const;
          const bool& read (const bool&);

          virtual QPainterPath shape() const;
          virtual void paint
            (QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

          enum { Type = connection_graph_type };
          virtual int type() const { return Type; }

        private:
          boost::optional<data::handle::connect> _handle;

          bool _read;
        };
      }
    }
  }
}

#endif
