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
            //! \todo There are seven modes by now.
#define IMPL(_t, _n, _d0, _d1, _d2, _d3, _d4)                           \
            _t _n (const mode::type& m)                                 \
            {                                                           \
              static _t x[5] = {_d0,_d1,_d2,_d3,_d4};                   \
                                                                        \
              return x[m];                                              \
            }                                                           \
            namespace detail                                            \
            {                                                           \
              static by_mode_by_key_type::value_type                    \
                _n ## _by_mode_by_key_value()                           \
              {                                                         \
                return by_mode_by_key_type::value_type (#_n, &_n);      \
              }                                                         \
            }

            IMPL (qreal,  border_thickness,      1.5,       1.5,        1.5,       1.5,       1.5     )
            IMPL (Qt::PenStyle,  border_style,   Qt::SolidLine, Qt::SolidLine, Qt::SolidLine, Qt::SolidLine, Qt::SolidLine)
            IMPL (QColor, border_color,          Qt::black, Qt::gray,   Qt::gray,  Qt::gray,  Qt::gray)
            IMPL (QBrush, background_brush,      Qt::white, Qt::white,  Qt::gray,  Qt::gray,  Qt::gray)
            IMPL (qreal,  text_line_thickness,   1.0,       1.0,        1.0,       1.0,       1.0     )
            IMPL (QColor, text_color,            Qt::black, Qt::black,  Qt::white, Qt::white, Qt::gray)
#undef IMPL

            namespace detail
            {
              static by_mode_by_key_type create_by_mode_by_key ()
              {
                by_mode_by_key_type by_mode_by_key;

#define INSERT(_n) by_mode_by_key.insert (_n ## _by_mode_by_key_value())

                INSERT (border_thickness);
                INSERT (border_style);
                INSERT (border_color);
                INSERT (background_brush);
                INSERT (text_line_thickness);
                INSERT (text_color);

#undef INSERT

                return by_mode_by_key;
              }

              const by_mode_by_key_type& get_by_mode_by_key ()
              {
                static by_mode_by_key_type by_mode_by_key (create_by_mode_by_key());

                return by_mode_by_key;
              }
            }
          }
        }
      }
    }
  }
}
