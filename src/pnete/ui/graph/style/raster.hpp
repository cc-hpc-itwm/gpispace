// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

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
