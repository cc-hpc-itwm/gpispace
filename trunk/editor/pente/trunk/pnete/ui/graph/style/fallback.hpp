// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_FALLBACK_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_FALLBACK_HPP 1

#include <pnete/ui/graph/style/style.fwd.hpp>

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
            namespace detail
            {
              typedef boost::variant < qreal
                                     , QColor
                                     > type;
            }

            class type
            {
            private:
              typedef boost::unordered_map<key_type, detail::type> map_type;

              map_type _map;

              static qreal border_thickness;
              static QColor border_color_normal;
              static QColor border_color_highlighted;
              static QColor background_color;
              static qreal text_line_thickness;
              static QColor text_color;

            public:
              type();

              template<typename T>
              const T& get (const style::key_type& key) const
              {
                const map_type::const_iterator pos (_map.find (key));

                if (pos == _map.end())
                  {
                    throw std::runtime_error
                      ("STRANGE: No default value for " + key);
                  }

                return boost::get<T> (pos->second);
              }
            };
          }
        }
      }
    }
  }
}

#endif
