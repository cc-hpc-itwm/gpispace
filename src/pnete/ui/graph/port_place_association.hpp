// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_UI_GRAPH_PORT_PLACE_ASSOCIATION_HPP
#define FHG_PNETE_UI_GRAPH_PORT_PLACE_ASSOCIATION_HPP

#include <pnete/ui/graph/port_place_association.fwd.hpp>

#include <pnete/data/handle/port.hpp>
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
        class port_place_association : public association
        {
          Q_OBJECT;

        public:
          port_place_association
            (port_item*, place_item*, const data::handle::port&);

          virtual const data::handle::port& handle() const;

          virtual QPainterPath shape() const;

          enum { Type = port_place_association_graph_type };
          virtual int type() const { return Type; }

        public slots:
          void place_association_set
            ( const data::handle::port&
            , const boost::optional<std::string>& place
            );

        private:
          data::handle::port _handle;
        };
      }
    }
  }
}

#endif