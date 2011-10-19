// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/style/fallback.hpp>

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
          namespace fallback
          {
            qreal type::border_thickness = 2.0;
            QColor type::border_color_normal = Qt::black;
            QColor type::border_color_highlighted = Qt::red;
            QColor type::background_color = Qt::gray;
            qreal type::text_line_thickness = 1.0;
            QColor type::text_color = Qt::black;

            type::type ()
              : _map()
            {
              _map["border_thickness"] = border_thickness;
              _map["border_color_normal"] = border_color_normal;
              _map["border_color_highlighted"] = border_color_highlighted;
              _map["background_color"] = background_color;
              _map["text_line_thickness"] = text_line_thickness;
              _map["text_color"] = text_color;
            }
          }
        }
      }
    }
  }
}
