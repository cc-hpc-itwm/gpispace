// mirko.rahn@itwm.fraunhofer.de

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
        namespace size
        {
#define CONST(_t,_n,_v) const _t& _n () { static const _t _n (_v); return _n; }

          namespace detail
          {
            static CONST(qreal, phi, (1.0 + sqrt(5.0)) / 2.0);

            static qreal ratio_shorter (const qreal& x) { return x / phi(); }
            static qreal ratio_longer (const qreal& x) { return x * phi(); }
          }

          CONST (qreal, raster, 10) // hardcoded constant

          namespace transition
          {
            CONST (qreal, height, 100) // hardcoded constant
            CONST (qreal, width, detail::ratio_longer (height()))
          }

          namespace port
          {
            CONST ( qreal
                  , width
                  , detail::ratio_shorter
                    (detail::ratio_shorter (transition::height()))
                  )
            CONST (qreal, height, detail::ratio_shorter (width()))
          }

          namespace cap
          {
            CONST (qreal, length, detail::ratio_shorter (port::height() / 2.0));
          }

          namespace zoom
          {
            CONST (int, min_value, 30);
            CONST (int, max_value, 300);
            CONST (int, default_value, 100);

            namespace slider
            {
              CONST (int, max_length, 200);
            }
          }

#undef CONST
        }
      }
    }
  }
}
