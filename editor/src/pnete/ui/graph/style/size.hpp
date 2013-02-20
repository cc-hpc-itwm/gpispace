// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_SIZE_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_SIZE_HPP 1

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

#endif
