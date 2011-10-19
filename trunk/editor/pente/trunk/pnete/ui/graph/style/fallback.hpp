// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_FALLBACK_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_FALLBACK_HPP 1

#include <pnete/ui/graph/style/type.fwd.hpp>

#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>

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
#define DECL(_t, _n) _t& _n ();
              DECL (qreal, border_thickness)
              DECL (QColor, border_color_normal)
              DECL (QColor, border_color_highlighted)
              DECL (QColor, background_color)
              DECL (qreal, text_line_thickness)
              DECL (QColor, text_color)
#undef DECL

            namespace detail
            {
              typedef boost::variant < const qreal&
                                     , const QColor&
                                     > type;
              typedef boost::unordered_map<key_type, type> map_type;

              const map_type& get_map ();
            }

            template<typename T>
            const T& get (const style::key_type& key)
            {
              const detail::map_type& map (detail::get_map());

              const detail::map_type::const_iterator pos (map.find (key));

              if (pos == map.end())
                {
                  throw std::runtime_error
                    ("STRANGE: No default value for " + key);
                }

              return boost::get<T> (pos->second);
            };
          }
        }
      }
    }
  }
}

#endif
