// bernd.loerwald@itwm.fraunhofer.de

#pragma once

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
        namespace style
        {
          namespace association
          {
            QPainterPath shape (const QList<QPointF>& points);
            QPainterPath shape_no_cap (const QList<QPointF>& points);
          }
        }
      }
    }
  }
}
