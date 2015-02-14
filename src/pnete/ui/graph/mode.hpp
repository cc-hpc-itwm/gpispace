// mirko.rahn@itwm.fraunhofer.de

#pragma once

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace mode
        {
          enum type
            { NORMAL
            , HIGHLIGHT
            , DRAG
            , MOVE
            , CONNECT
            , CONNECT_ALLOWED
            , CONNECT_FORBIDDEN
            };
        }
      }
    }
  }
}

#include <functional>

namespace std
{
  template<> struct hash<fhg::pnete::ui::graph::mode::type>
  {
    size_t operator() (const fhg::pnete::ui::graph::mode::type& v) const
    {
      return std::hash
        <std::underlying_type<fhg::pnete::ui::graph::mode::type>::type>() (v);
    }
  };
};
