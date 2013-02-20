// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_SIZE_HPP
#define _FHG_PNETE_UI_SIZE_HPP 1

#include <QtGlobal>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace size
      {
        namespace zoom
        {
          const int& min_value();
          const int& max_value();
          const int& default_value();
          const int& per_tick();

          namespace slider
          {
            const int& max_length();
          }
        }
      }
    }
  }
}

#endif
