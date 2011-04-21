#ifndef GRAPHSTYLE
#define GRAPHSTYLE 1

#include <QPointF>
#include <QRectF>
#include <QPainterPath>
#include <QList>

class QPainter;

#include "Port.hpp"

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
          static const QPainterPath portShape(const Port* port);
          static const QRectF portBoundingRect(const Port* port);
          static const void portPaint(QPainter *painter, const Port* portN);
          
          static const QPainterPath connectionShape(const Connection* connection);
          static const QRectF connectionBoundingRect(const Connection* connection);
          static const void connectionPaint(QPainter* painter, const Connection* connection);
          
          static const QPainterPath transitionShape(const QSizeF& size);
          static const QRectF transitionBoundingRect(const QSizeF& size);
          
          static const qreal raster();
          static const QPointF snapToRaster(const QPointF& p);
      };
    }
  }
}

#endif
