// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_MODE_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_MODE_HPP 1

#include <boost/variant.hpp>

class QString;

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
          namespace mode
          {
            enum type
              { NORMAL
              , HIGHLIGHT
              , DRAG
              , MOVE
              , CONNECT
              };
          }
        }
      }
    }
  }
}

#endif
