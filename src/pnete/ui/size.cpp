// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/size.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace size
      {
#define CONST(_t,_n,_v) const _t& _n () { static const _t _n (_v); return _n; }

        namespace zoom
        {
          CONST (int, min_value, 30);
          CONST (int, max_value, 300);
          CONST (int, default_value, 100);
          CONST (int, per_tick, 10);

          namespace slider
          {
            CONST (int, max_length, 200);
          }

#undef CONST
        }
      }
    }
  }
}
