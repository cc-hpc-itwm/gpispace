// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/ui/graph/style/size.hpp>

#include <cmath>

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
            qreal snap (const qreal& x)
            {
              return size::raster() * std::ceil (x / size::raster());
            }

            QPointF snap (const QPointF& pos)
            {
              return QPointF (snap (pos.x()), snap (pos.y()));
            }
          } // namespace raster
        }
      }
    }
  }
}
