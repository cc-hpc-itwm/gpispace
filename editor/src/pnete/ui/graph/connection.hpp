// bernd.loerwald@itwm.fraunhofer.de

#ifndef PNETE_UI_GRAPH_CONNECTION_HPP
#define PNETE_UI_GRAPH_CONNECTION_HPP

#include <pnete/ui/graph/connection.fwd.hpp>

#include <pnete/data/handle/connect.hpp>
#include <pnete/ui/graph/association.hpp>
#include <pnete/ui/graph/connectable_item.fwd.hpp>

#include <QList>
#include <QPointF>
#include <QRectF>
#include <QObject>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPainterPath;

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
                          , const data::handle::connect& handle
                          );

          virtual const data::handle::connect& handle() const;

          enum { Type = connection_graph_type };
          virtual int type() const { return Type; }

        public slots:
          void connection_removed (const data::handle::connect&);

          void connection_direction_changed (const data::handle::connect&);

        private:
          data::handle::connect _handle;
        };
      }
    }
  }
}

#endif
