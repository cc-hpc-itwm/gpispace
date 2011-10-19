// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/style/fallback.hpp>

#include <vector>

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
            template<typename T>
            std::vector<T> init (const T& value)
            {
              std::vector<T> x;

              for (int i (0); i < 5; ++i) { x.push_back (value); }

              return x;
            }

#define IMPL(_t, _n, _d)                                                \
            _t& _n (const mode::type& m)                                \
            {                                                           \
              static std::vector<_t> x (init<_t> (_d));                 \
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

            IMPL (qreal, border_thickness, 2.0)
            IMPL (QColor, border_color, Qt::black)
            IMPL (QColor, background_color, Qt::white)
            IMPL (qreal, text_line_thickness, 1.0)
            IMPL (QColor, text_color, Qt::black)
#undef IMPL

            namespace detail
            {
              static by_mode_by_key_type create_by_mode_by_key ()
              {
                by_mode_by_key_type by_mode_by_key;

#define INSERT(_n) by_mode_by_key.insert (_n ## _by_mode_by_key_value())

                INSERT (border_thickness);
                INSERT (border_color);
                INSERT (background_color);
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
