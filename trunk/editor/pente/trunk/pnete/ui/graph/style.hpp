#ifndef _PNETE_UI_GRAPH_STYLE_HPP
#define _PNETE_UI_GRAPH_STYLE_HPP 1

#include <QPointF>
#include <QRectF>
#include <QPainterPath>
#include <QList>

class QPainter;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class connection;
        class transition;
        class port;

        class style
        {
        public:
          enum ePortArea
          {
            TAIL,
            MAIN,
          };
          static QPainterPath portShape(const port* port);
          static QRectF portBoundingRect(const port* port, bool withCap = true, int capFactor = 0);
          static void portPaint(QPainter *painter, const port* port);
          static ePortArea portHit(const port* port, const QPointF& point);
          static qreal portCapLength();
          static qreal portDefaultWidth();
          static qreal portDefaultHeight();

          static QPainterPath connectionShape(const connection* connection);
          static QRectF connectionBoundingRect(const connection* connection);
          static void connectionPaint(QPainter* painter, const connection* connection);

          static QPainterPath transitionShape(const QSizeF& size);
          static QRectF transitionBoundingRect(const QSizeF& size);
          static void transitionPaint(QPainter* painter, const transition* transition);

          static qreal raster();
          static QPointF snapToRaster(const QPointF& p);
        };
      }
    }
  }
}

#endif
