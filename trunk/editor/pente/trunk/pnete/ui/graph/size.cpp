// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/size.hpp>

#include <cmath>

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
#define CONST(_name,_value)                    \
          const qreal& _name ()                \
          {                                    \
            static const qreal _name (_value); \
                                               \
            return _name;                      \
          }

          namespace detail
          {
            static CONST(phi, (1.0 + sqrt(5.0)) / 2.0);

            qreal ratio_shorter (const qreal& x) { return x / phi(); }
            qreal ratio_longer (const qreal& x) { return x * phi(); }
          }

          CONST (raster, 10) // hardcoded constant

          namespace transition
          {
            CONST (height, 100) // hardcoded constant
            CONST (width, detail::ratio_longer (height()))
          }

          namespace port
          {
            CONST ( width
                  , detail::ratio_shorter
                    (detail::ratio_shorter (transition::height()))
                  )
            CONST (height, detail::ratio_shorter (width()))
          }

          namespace cap
          {
            CONST (length, detail::ratio_shorter (port::height()));
          }

#undef CONST
        }
      }
    }
  }
}
