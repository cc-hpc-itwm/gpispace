// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_FALLBACK_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_FALLBACK_HPP 1

#include <pnete/ui/graph/style/type.fwd.hpp>
#include <pnete/ui/graph/mode.hpp>

#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>
#include <boost/function.hpp>

#include <QColor>

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
#define DECL(_t, _n) _t& _n (const mode::type&)

            DECL (qreal, border_thickness);
            DECL (QColor, border_color);
            DECL (QColor, background_color);
            DECL (qreal, text_line_thickness);
            DECL (QColor, text_color);

#undef DECL
            namespace detail
            {
              typedef boost::variant < const qreal&
                                     , const QColor&
                                     , const Qt::PenStyle&
                                     > type;

              typedef boost::function< type (const mode::type&)
                                     > by_mode_type;

              typedef boost::unordered_map< key_type
                                          , by_mode_type
                                          > by_mode_by_key_type;

              const by_mode_by_key_type& get_by_mode_by_key();
            }

            template<typename T>
            const T& get ( const style::key_type& key
                         , const mode::type& mode
                         )
            {
              const detail::by_mode_by_key_type& by_mode_by_key
                (detail::get_by_mode_by_key());

              const detail::by_mode_by_key_type::const_iterator by_mode
                (by_mode_by_key.find (key));

              if (by_mode == by_mode_by_key.end())
                {
                  throw std::runtime_error
                    ("STRANGE: No default values for " + key);
                }

              return boost::get<T> (by_mode->second (mode));
            };
          }
        }
      }
    }
  }
}

#endif
