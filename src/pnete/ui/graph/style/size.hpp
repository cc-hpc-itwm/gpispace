// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <QtGlobal>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace size
        {
          const qreal& raster();

          namespace port
          {
            const qreal& width();
            const qreal& height();
          }

          namespace transition
          {
            const qreal& width();
            const qreal& height();
          }

          namespace cap
          {
            const qreal& length();
          }
        }
      }
    }
  }
}
