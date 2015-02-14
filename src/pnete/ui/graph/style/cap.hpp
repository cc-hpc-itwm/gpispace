// mirko.rahn@itwm.fraunhofer.de

#pragma once

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
