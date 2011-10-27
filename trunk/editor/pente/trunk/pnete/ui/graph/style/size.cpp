// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/style/size.hpp>
#include <util/phi.hpp>

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

          CONST (qreal, raster, 10) // hardcoded constant

          namespace transition
          {
            CONST (qreal, height, 100) // hardcoded constant
            CONST (qreal, width, util::phi::ratio::bigger (height()))
          }

          namespace port
          {
            CONST ( qreal
                  , width
                  , util::phi::ratio::smaller
                    (util::phi::ratio::smaller (transition::height()))
                  )
            CONST (qreal, height, util::phi::ratio::smaller (width()))
          }

          namespace cap
          {
            CONST ( qreal
                  , length
                  , util::phi::ratio::smaller (port::height() / 2.0)
                  );
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
