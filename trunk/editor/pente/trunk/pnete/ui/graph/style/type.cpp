// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/style/type.hpp>

#include <pnete/ui/graph/item.hpp>

#include <QPainter>
#include <QPen>
#include <QBrush>

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
          void draw_shape ( const type& style
                          , const graph::item* item
                          , QPainter* painter
                          )
          {
            painter->setPen (QPen ( QBrush ( item->highlighted()
                                           ? style.get<QColor> (item, "border_color_highlighted")
                                           : style.get<QColor> (item, "border_color_normal")
                                           )
                                  , style.get<qreal> (item, "border_thickness")
                                  )
                            );
            painter->setBackgroundMode (Qt::OpaqueMode);
            painter->setBrush (QBrush (style.get<QColor> (item, "background_color")));
            painter->drawPath (item->shape());
          }
        }
      }
    }
  }
}
