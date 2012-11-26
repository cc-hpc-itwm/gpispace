// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_UI_GRAPH_STYLE_ASSOCIATION_HPP
#define FHG_PNETE_UI_GRAPH_STYLE_ASSOCIATION_HPP

#include <QList>

class QPainterPath;
class QPointF;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace association
        {
          QPainterPath shape (const QList<QPointF>& points);
        }
      }
    }
  }
}

#endif
