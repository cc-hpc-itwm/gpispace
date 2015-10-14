// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <pnete/ui/graph/place_map.fwd.hpp>

#include <pnete/data/handle/place_map.hpp>
#include <pnete/ui/graph/association.hpp>
#include <pnete/ui/graph/place.fwd.hpp>
#include <pnete/ui/graph/port.fwd.hpp>

#include <QList>
#include <QObject>
#include <QPointF>
#include <QRectF>

class QPainter;
class QPainterPath;
class QStyleOptionGraphicsItem;
class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class place_map : public association
        {
          Q_OBJECT

        public:
          place_map (port_item*, place_item*, const data::handle::place_map&);

          virtual const data::handle::place_map& handle() const override;

          virtual QPainterPath shape() const override;

          enum { Type = place_map_graph_type };
          virtual int type() const override { return Type; }

        public slots:
          void place_map_removed (const data::handle::place_map&);

        private:
          data::handle::place_map _handle;
        };
      }
    }
  }
}
