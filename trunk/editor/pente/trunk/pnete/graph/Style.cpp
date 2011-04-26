#include "Style.hpp"
#include "Connection.hpp"
#include "Port.hpp"
#include "Transition.hpp"

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
      
      const Qt::GlobalColor queryColorForType(const QString& type)
      {
        if(type == "trace")
        {
          return Qt::blue;
        }
        else if(type == "velocity")
        {
          return Qt::green;
        }
        else
        {
          return Qt::white;
        }
      }
      
      const void Style::portPaint(QPainter *painter, const Port* port)
      {
        painter->setPen(QPen(QBrush(port->highlighted() ? Qt::red : Qt::black), 2.0f));
        painter->setBackgroundMode(Qt::OpaqueMode);
        painter->setBrush(QBrush(queryColorForType(port->dataType()), Qt::SolidPattern));
        painter->drawPath(portShape(port));
        
        painter->setPen(QPen(QBrush(Qt::black), 1.0));
        painter->setBackgroundMode(Qt::TransparentMode);
        painter->drawText(port->boundingRect(), Qt::AlignCenter, port->title());
      }
            
      const QPainterPath Style::connectionShape(const Connection* connection)
      {
        //! \todo This whole thing might be done easier, better, nicer, cleaner, less buggy.
        
        const ConnectableItem* startItem = connection->start();
        const ConnectableItem* endItem = connection->end();
        if(!startItem && !endItem)
        {
          return QPainterPath();
        }
        
        QPointF start = connection->startPosition();
        QPointF end = connection->endPosition();
        const QList<QPointF>& mid = connection->midpoints();
        
        QPainterPath path;
        
        QList<QPointF> allPoints;
        allPoints.push_back(start);
        for(QList<QPointF>::const_iterator it = mid.begin(); it != mid.end(); ++it)
        {
          allPoints.push_back(*it);
        }
        allPoints.push_back(end);
        
        QList<QLineF> linesForward;
        QList<QLineF> linesBackward;
        
        for(QList<QPointF>::const_iterator first = allPoints.begin(), second = allPoints.begin() + 1; second != allPoints.end(); ++first, ++second)
        {
          qreal dummyLength = QLineF(*first, *second).length();
          QLineF dummyLineForward(QPointF(0.0, -portHeightHalf), QPointF(dummyLength, -portHeightHalf));
          QLineF dummyLineBackward(QPointF(0.0, portHeightHalf), QPointF(dummyLength, portHeightHalf));
          
          QTransform transformation;
          transformation.translate(first->x(), first->y());
          transformation.rotate(-QLineF(*first, *second).angle());
          
          linesForward.push_back(transformation.map(dummyLineForward));
          linesBackward.push_front(transformation.map(dummyLineBackward));
        }
        
        for(QList<QLineF>::iterator line = linesForward.begin(), nextLine = line + 1; line != linesForward.end(); ++line, ++nextLine)
        {
          if(nextLine != linesForward.end())
          {
            QPointF intersection;
            if(line->intersect(*nextLine, &intersection) != QLineF::NoIntersection)
            {
              line->setP2(intersection);
              nextLine->setP1(intersection);
            }
          }
          QPolygonF poly;
          poly << line->p1() << line->p2();
          path.addPolygon(poly);
        }
        
        {
          const QLineF& lastForward = linesForward.last();
          QTransform transformation;
          transformation.rotate(-lastForward.angle());
          
          QPolygonF cap;
          for(size_t i = 0; i < sizeof(outgoingCap) / sizeof(QPointF); ++i)
          {
            cap << transformation.map(QPointF(outgoingCap[i].x(), outgoingCap[i].y() + portHeightHalf)) + lastForward.p2();
          }
          path.addPolygon(cap);
        }
        
        for(QList<QLineF>::iterator line = linesBackward.begin(), nextLine = line + 1; line != linesBackward.end(); ++line, ++nextLine)
        {
          if(nextLine != linesBackward.end())
          {
            QPointF intersection;
            if(line->intersect(*nextLine, &intersection) != QLineF::NoIntersection)
            {
              line->setP1(intersection);
              nextLine->setP2(intersection);
            }
          }
          QPolygonF poly;
          poly << line->p1() << line->p2();
          path.addPolygon(poly);
        }
        
        {
          const QLineF& lastBackward = linesBackward.last();
          QTransform transformation;
          transformation.rotate(180-lastBackward.angle());
          
          QPolygonF cap;
          for(size_t i = 0; i < sizeof(ingoingCap) / sizeof(QPointF); ++i)
          {
            cap << transformation.map(QPointF(ingoingCap[i].x(), ingoingCap[i].y() + portHeightHalf)) + lastBackward.p1();
          }
          path.addPolygon(cap);
        }
      
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
      const void Style::transitionPaint(QPainter* painter, const Transition* transition)
      {
        painter->setPen(QPen(QBrush(transition->highlighted() ? Qt::red : Qt::black), 2.0));
        painter->setBackgroundMode(Qt::OpaqueMode);
        painter->setBrush(QBrush(Qt::white, Qt::SolidPattern));
        painter->drawPath(transition->shape());
        
        painter->setPen(QPen(QBrush(Qt::black), 1.0));
        painter->setBackgroundMode(Qt::TransparentMode);
        painter->drawText(transition->boundingRect(), Qt::AlignCenter, transition->title());
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
