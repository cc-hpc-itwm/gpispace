#include "Style.hpp"
#include "Connection.hpp"
#include "Port.hpp"

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QTransform>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      static const qreal rasterSize = 10.0;
      
      static const qreal portHeight = 20.0;
      static const qreal portHeightHalf = portHeight / 2.0;
      
      static const qreal capLength = 10.0;
      
      // defined from the center of the end of the line to attach to. going clockwise.
      static const QPointF outgoingCap[] = { QPointF(0.0, -portHeightHalf), QPointF(capLength, 0.0), QPointF(0.0, portHeightHalf) };
      static const QPointF ingoingCap[] = { QPointF(0.0, -portHeightHalf), QPointF(capLength, -portHeightHalf), QPointF(0.0, 0.0), QPointF(capLength, portHeightHalf), QPointF(0.0, portHeightHalf) };
      
      const QPainterPath Style::portShape(const Port* port)
      {
        const qreal& length = port->length();
        const qreal lengthHalf = length / 2.0;
                
        QPolygonF poly;
        poly << QPointF(lengthHalf - capLength, portHeightHalf)
             << QPointF(-lengthHalf, portHeightHalf)
             << QPointF(-lengthHalf, -portHeightHalf)
             << QPointF(lengthHalf - capLength, -portHeightHalf);
        
        //! \todo put capping into a function
        if(port->direction() == ConnectableItem::IN)
        {
          for(size_t i = 0; i < sizeof(ingoingCap) / sizeof(QPointF); ++i)
          {
            poly << ingoingCap[i] + QPointF(lengthHalf - capLength, 0.0);
          }
        }
        else
        {
          for(size_t i = 0; i < sizeof(outgoingCap) / sizeof(QPointF); ++i)
          {
            poly << outgoingCap[i] + QPointF(lengthHalf - capLength, 0.0);
          }
        }
        
        static const qreal angles[] = { -90.0, 0.0, 90.0, 180.0 };
        
        QTransform rotation;
        rotation.rotate(angles[port->orientation()]);
        poly = rotation.map(poly);
        
        QPainterPath path;
        path.addPolygon(poly);
        path.closeSubpath();
        
        return path;
      }
      const QRectF Style::portBoundingRect(const Port* port)
      {
        const qreal& length = port->length();
        const qreal lengthHalf = length / 2.0;
        
        switch(port->orientation())
        {
          case ConnectableItem::NORTH:
          case ConnectableItem::SOUTH:
            return QRectF(-portHeightHalf, -lengthHalf, portHeight, length);
            
          case ConnectableItem::EAST:
          case ConnectableItem::WEST:
            return QRectF(-lengthHalf, -portHeightHalf, length, portHeight);
            
          case ConnectableItem::ANYORIENTATION:
          default:
            return QRectF();
        }
        
      }
      const void Style::portPaint(QPainter *painter, const Port* port)
      {
        painter->setPen(QPen(QBrush(port->highlighted() ? Qt::red : Qt::black), 2.0f));
        painter->setBackgroundMode(Qt::OpaqueMode);
        painter->setBrush(QBrush(Qt::white, Qt::SolidPattern));
        painter->drawPath(portShape(port));
      }
            
      const QPainterPath Style::connectionShape(const Connection* connection)
      {
        QPointF start = connection->startPosition();
        QPointF end = connection->endPosition();
        const QList<QPointF>& mid = connection->midpoints();
        
        const ConnectableItem* startItem = connection->start();
        const ConnectableItem* endItem = connection->end();
        
        ConnectableItem::eOrientation startOrientation = startItem ? startItem->orientation() : ConnectableItem::ANYORIENTATION;
        ConnectableItem::eOrientation endOrientation = endItem ? endItem->orientation() : ConnectableItem::ANYORIENTATION;
       
        //! \todo move this to connectableitem.
       
        switch(startOrientation)
        {
          case ConnectableItem::NORTH:
            start.setY(start.y() - 30.0f); 
            break;
          case ConnectableItem::EAST:
            start.setX(start.x() + 30.0f); 
            break;
          case ConnectableItem::SOUTH:
            start.setY(start.y() + 30.0f); 
            break;
          case ConnectableItem::WEST:
            start.setX(start.x() - 30.0f); 
            break;
        }
       
        switch(endOrientation)
        {
          case ConnectableItem::NORTH:
            end.setY(end.y() - 30.0f); 
            break;
          case ConnectableItem::EAST:
            end.setX(end.x() + 30.0f); 
            break;
          case ConnectableItem::SOUTH:
            end.setY(end.y() + 30.0f); 
            break;
          case ConnectableItem::WEST:
            end.setX(end.x() - 30.0f); 
            break;
        }
        
        QLineF dummyLine(QPointF(0.0, 0.0), QPointF(QLineF(start, end).length(), 0.0));
        
        QPolygonF poly;
        
        poly << (dummyLine.p1() + QPointF(0.0, -portHeightHalf))
             << (dummyLine.p2() + QPointF(0.0, -portHeightHalf));
        
        for(size_t i = 0; i < sizeof(outgoingCap) / sizeof(QPointF); ++i)
        {
          poly << outgoingCap[i] + dummyLine.p2();
        }
        
        poly << (dummyLine.p2() + QPointF(0.0, portHeightHalf))
             << (dummyLine.p1() + QPointF(0.0, portHeightHalf));
        
        for(size_t i = 0; i < sizeof(ingoingCap) / sizeof(QPointF); ++i)
        {
          poly << -ingoingCap[i] + dummyLine.p1();
        }
        
        QTransform transformation;
        transformation.translate(start.x(), start.y());
        transformation.rotate(-QLineF(start, end).angle());
        
        poly = transformation.map(poly);
        
        QPainterPath path;
        path.addPolygon(poly);
        path.closeSubpath();
        return path;
      }
      const QRectF Style::connectionBoundingRect(const Connection* connection)
      {
        return connectionShape(connection).boundingRect();
      }
      const void Style::connectionPaint(QPainter* painter, const Connection* connection)
      {        
        painter->setPen(QPen(QBrush(connection->highlighted() ? Qt::red : Qt::black), 2));
        painter->drawPath(connectionShape(connection));
      }
      
      const QPainterPath Style::transitionShape(const QSizeF& size)
      {
        QPainterPath path;
        path.addRect(transitionBoundingRect(size));
        return path;
      }
      const QRectF Style::transitionBoundingRect(const QSizeF& size)
      {
        return QRectF(0.0f, 0.0f, size.width(), size.height());
      }
      
      const qreal Style::raster()
      {
        return rasterSize;
      }
      const QPointF Style::snapToRaster(const QPointF& pos)
      {
        qreal raster = Style::raster();
        return QPointF(raster * static_cast<int>(pos.x() / raster), raster * static_cast<int>(pos.y() / raster));
      }
    }
  }
}
