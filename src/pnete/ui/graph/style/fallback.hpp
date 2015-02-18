// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <pnete/ui/graph/mode.hpp>
#include <pnete/ui/graph/style/type.fwd.hpp>

#include <boost/variant.hpp>

#include <QBrush>
#include <QColor>

#include <functional>
#include <unordered_map>

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
              typedef boost::variant<qreal, QColor, Qt::PenStyle, QBrush> return_type;

              typedef std::function<return_type (const mode::type&)> by_mode_type;

              typedef std::unordered_map< key_type
                                        , by_mode_type
                                        > by_mode_by_key_type;

              const by_mode_by_key_type& get_by_mode_by_key();
            }

            template<typename T>
              T get (const style::key_type& key, const mode::type& mode)
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
