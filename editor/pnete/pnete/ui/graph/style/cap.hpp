// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_STYLE_CAP_HPP
#define _FHG_PNETE_UI_GRAPH_STYLE_CAP_HPP 1

#include <QtGlobal>
#include <QPoint>

class QPainterPath;
class QPolygonF;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace cap
        {
          void add_outgoing ( QPainterPath* path
                            , bool middle
                            , QPointF offset = QPointF()
                            , qreal rotation = 0.0
                            );
          void add_outgoing ( QPolygonF* poly
                            , QPointF offset = QPointF()
                            , qreal rotation = 0.0
                            );
          void add_incoming ( QPainterPath* path
                            , bool middle
                            , QPointF offset = QPointF()
                            , qreal rotation = 0.0
                            );
          void add_incoming ( QPolygonF* poly
                            , QPointF offset = QPointF()
                            , qreal rotation = 0.0
                            );
        } // namespace cap
      }
    }
  }
}

#endif
