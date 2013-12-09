// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_STYLE_RASTER_HPP
#define _PNETE_UI_GRAPH_STYLE_RASTER_HPP 1

#include <QPointF>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace style
        {
          namespace raster
          {
            qreal snap (const qreal&);
            QPointF snap (const QPointF&);
          }
        }
      }
    }
  }
}

#endif
