#ifndef GRAPHSTYLE
#define GRAPHSTYLE 1

#include <QPointF>
#include <QRectF>
#include <QPainterPath>
#include <QList>

class QPainter;

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Connection;
      class Transition;
      class Port;
      
      class Style
      {
        public:
          enum ePortArea
          {
            TAIL,
            MAIN,
          };
          static QPainterPath portShape(const Port* port);
          static QRectF portBoundingRect(const Port* port, bool withCap = true, int capFactor = 0);
          static void portPaint(QPainter *painter, const Port* port);
          static ePortArea portHit(const Port* port, const QPointF& point);
          static qreal portCapLength();
          static qreal portDefaultWidth();
          static qreal portDefaultHeight();
          
          static QPainterPath connectionShape(const Connection* connection);
          static QRectF connectionBoundingRect(const Connection* connection);
          static void connectionPaint(QPainter* painter, const Connection* connection);
          
          static QPainterPath transitionShape(const QSizeF& size);
          static QRectF transitionBoundingRect(const QSizeF& size);
          static void transitionPaint(QPainter* painter, const Transition* transition);
          
          static qreal raster();
          static QPointF snapToRaster(const QPointF& p);
      };
    }
  }
}

#endif
