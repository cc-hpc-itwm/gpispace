// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_UI_GRAPH_ISC13_HPP
#define FHG_PNETE_UI_GRAPH_ISC13_HPP

#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/style/type.hpp>

#include <QColor>
#include <QString>

#include <boost/function.hpp>
#include <boost/optional.hpp>

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
          namespace isc13
          {
            void add_colors_for_types ( style::type* style
                                      , const boost::function
                                        < boost::optional<QColor>
                                          ( const base_item*
                                          , const QColor&
                                          , const QString&
                                          )>& fun
                                      );
          }
        }
      }
    }
  }
}

#endif
