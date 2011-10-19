// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/style/fallback.hpp>

#include <boost/ref.hpp>

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
#define IMPL(_t, _n, _d)                                        \
            _t& _n() { static _t x(_d); return x; }             \
            namespace detail                                    \
            {                                                   \
              static map_type::value_type _n ## _map_value()    \
              {                                                 \
                return map_type::value_type (#_n, _n());        \
              }                                                 \
            }

            IMPL (qreal, border_thickness, 2.0)
            IMPL (QColor, border_color_normal, Qt::black)
            IMPL (QColor, border_color_highlighted, Qt::red)
            IMPL (QColor, background_color, Qt::white)
            IMPL (qreal, text_line_thickness, 1.0)
            IMPL (QColor, text_color, Qt::black)
#undef IMPL

            namespace detail
            {
              static map_type create_map ()
              {
                map_type map;

                map.insert (border_thickness_map_value());
                map.insert (border_color_normal_map_value());
                map.insert (border_color_highlighted_map_value());
                map.insert (background_color_map_value());
                map.insert (text_line_thickness_map_value());
                map.insert (text_color_map_value());

                return map;
              }

              const map_type& get_map ()
              {
                static map_type map (create_map());

                return map;
              }
            }
          }
        }
      }
    }
  }
}
