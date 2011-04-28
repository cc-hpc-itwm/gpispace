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
      static const qreal defaultPortWidth = 60.0;
      
      static const qreal capLength = 10.0;
      
      // defined from the center of the end of the line to attach to. going clockwise.
      static const QPointF outgoingCap[] = { QPointF(0.0, -portHeightHalf), QPointF(capLength, 0.0), QPointF(0.0, portHeightHalf) };
      static const QPointF ingoingCap[] = { QPointF(0.0, -portHeightHalf), QPointF(capLength, -portHeightHalf), QPointF(0.0, 0.0), QPointF(capLength, portHeightHalf), QPointF(0.0, portHeightHalf) };
      
      const qreal Style::portCapLength()
      {
        return capLength;
      }
      const qreal Style::portDefaultWidth()
      {
        return defaultPortWidth;
      }
      
      void addOutgoingCap(QPainterPath* path, bool middle, QPointF offset = QPointF(), qreal rotation = 0.0)
      {
        QTransform transformation;
        transformation.rotate(rotation);
        
        for(size_t i = 0; i < sizeof(outgoingCap) / sizeof(QPointF); ++i)
        {
          path->lineTo(transformation.map(QPointF(outgoingCap[i].x(), outgoingCap[i].y() + (middle ? portHeightHalf : 0.0))) + offset);
        }
      }
      
      void addOutgoingCap(QPolygonF* poly, QPointF offset = QPointF(), qreal rotation = 0.0)
      {
        QPainterPath path;
        addOutgoingCap(&path, false, offset, rotation);
        *poly = poly->united(path.toFillPolygon());
      }
      
      void addIngoingCap(QPainterPath* path, bool middle, QPointF offset = QPointF(), qreal rotation = 0.0)
      {
        QTransform transformation;
        transformation.rotate(rotation);
        
        for(size_t i = 0; i < sizeof(ingoingCap) / sizeof(QPointF); ++i)
        {
          path->lineTo(transformation.map(QPointF(ingoingCap[i].x(), ingoingCap[i].y() + (middle ? portHeightHalf : 0.0))) + offset);
        }
      }
      
      void addIngoingCap(QPolygonF* poly, QPointF offset = QPointF(), qreal rotation = 0.0)
      {
        QPainterPath path;
        addIngoingCap(&path, false, offset, rotation);
        *poly = poly->united(path.toFillPolygon());
      }
      
      const QPainterPath Style::portShape(const Port* port)
      {
        const qreal& length = port->length();
        const qreal lengthHalf = length / 2.0;
                
        QPolygonF poly;
        poly << QPointF(lengthHalf - portCapLength(), portHeightHalf)
             << QPointF(-lengthHalf, portHeightHalf)
             << QPointF(-lengthHalf, -portHeightHalf)
             << QPointF(lengthHalf - portCapLength(), -portHeightHalf);
        
        if(port->direction() == ConnectableItem::IN)
        {
          addIngoingCap(&poly, QPointF(lengthHalf - portCapLength(), 0.0));
        }
        else
        {
          addOutgoingCap(&poly, QPointF(lengthHalf - portCapLength(), 0.0));
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
      
      const QColor queryColorForType(const QString& type)
      {
        //! \note Colors shamelessly stolen from PSPro.
        //! \todo Maybe also do a gradient? Maybe looks awesome.
        if(type.startsWith("seismic"))
        {
          return QColor(0, 130, 250); 
        }
        else if(type.startsWith("velocity"))
        {
          return QColor(248, 248, 6); 
        }
        else
        {
          return QColor(255, 255, 255);
        }
      }
      
      const void Style::portPaint(QPainter *painter, const Port* port)
      {
        painter->setPen(QPen(QBrush(port->highlighted() ? Qt::red : Qt::black), 2.0));
        painter->setBackgroundMode(Qt::OpaqueMode);
        painter->setBrush(QBrush(queryColorForType(port->dataType()), Qt::SolidPattern));
        painter->drawPath(portShape(port));
        
        painter->setPen(QPen(QBrush(Qt::black), 1.0));
        painter->setBackgroundMode(Qt::TransparentMode);
        
        if(port->orientation() == ConnectableItem::NORTH || port->orientation() == ConnectableItem::SOUTH)
        {
          qreal degrees = 90.0;
          
          QTransform antirotation;
          antirotation.rotate(-degrees);
          
          painter->save();
          painter->rotate(degrees); 
          painter->drawText(antirotation.mapRect(port->boundingRect()), Qt::AlignCenter, port->title());
          painter->restore();
        }
        else
        {
          painter->drawText(port->boundingRect(), Qt::AlignCenter, port->title());
        }
      }
     
      const Style::ePortArea Style::portHit(const Port* port, const QPointF& point)
      {
        const qreal& length = port->length();
        const qreal lengthHalf = length / 2.0;
        
        QRectF area;
        switch(port->orientation())
        {
          case ConnectableItem::NORTH:
            area = QRectF(-portHeightHalf, -lengthHalf + portCapLength() * 2, portHeight, length - portCapLength() * 2);
            break;
            
          case ConnectableItem::SOUTH:
            area = QRectF(-portHeightHalf, -lengthHalf, portHeight, length - portCapLength() * 2);
            break;
            
          case ConnectableItem::EAST:
            area = QRectF(-lengthHalf, -portHeightHalf, length - portCapLength() * 2, portHeight);
            break;
            
          case ConnectableItem::WEST:
            area = QRectF(-lengthHalf + portCapLength() * 2, -portHeightHalf, length - portCapLength() * 2, portHeight);
            break;
            
          case ConnectableItem::ANYORIENTATION:
          default:
            area = QRectF();
            break;
        }
        
        if(area.contains(point))
        {
          return MAIN;
        }
        else
        {
          return TAIL;
        }
      }
            
      const QPainterPath Style::connectionShape(const Connection* connection)
      {
        const ConnectableItem* startItem = connection->start();
        const ConnectableItem* endItem = connection->end();
        if(!startItem && !endItem)
        {
          return QPainterPath();
        }
        
        QPointF start = connection->startPosition();
        QPointF end = connection->endPosition();
        const QList<QPointF>& mid = connection->midpoints();
        
        //! \todo indicate connection with blob?
        if(start == end)
        {
          return QPainterPath();
        }
        
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        
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
          
          QTransform transformation;
          transformation.translate(first->x(), first->y());
          transformation.rotate(-QLineF(*first, *second).angle());
          
          linesForward.push_back(transformation.map(QLineF(QPointF(0.0, -portHeightHalf), QPointF(dummyLength, -portHeightHalf))));
          linesBackward.push_front(transformation.map(QLineF(QPointF(0.0, portHeightHalf), QPointF(dummyLength, portHeightHalf))));
        }
        
        path.moveTo(linesForward.first().p1());
        
        QPointF intersection;
        
        for(QList<QLineF>::iterator line = linesForward.begin(); line != linesForward.end(); ++line)
        {
          QPointF target = line->p2();
          
          QList<QLineF>::iterator nextLine = line + 1;
          if(nextLine != linesForward.end() && line->intersect(*nextLine, &intersection) != QLineF::NoIntersection)
          {
            target = intersection;
          }
          
          path.lineTo(target);
        }
        
        addOutgoingCap(&path, true, linesForward.last().p2(), -linesForward.last().angle());
        
        path.lineTo(linesBackward.first().p2());
        
        for(QList<QLineF>::iterator line = linesBackward.begin(); line != linesBackward.end(); ++line)
        {
          QPointF target = line->p1();
          
          QList<QLineF>::iterator nextLine = line + 1;
          if(nextLine != linesBackward.end() && line->intersect(*nextLine, &intersection) != QLineF::NoIntersection)
          {
            target = intersection;
          }
          
          path.lineTo(target);
        }
        
        addIngoingCap(&path, true, linesBackward.last().p1(), 180.0 - linesBackward.last().angle());
        
        path.lineTo(linesForward.first().p1());
                
        return path;
      }
      const QRectF Style::connectionBoundingRect(const Connection* connection)
      {
        return connectionShape(connection).boundingRect();
      }
      const void Style::connectionPaint(QPainter* painter, const Connection* connection)
      {        
        painter->setPen(QPen(QBrush(connection->highlighted() ? Qt::red : Qt::black), 2));
        painter->setBackgroundMode(Qt::OpaqueMode);
        painter->setBrush(QBrush(Qt::white, Qt::SolidPattern));
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
        return QRectF(0.0, 0.0, size.width(), size.height());
      }
      const void Style::transitionPaint(QPainter* painter, const Transition* transition)
      {
        painter->setPen(QPen(QBrush(transition->highlighted() ? Qt::red : Qt::black), 2.0));
        painter->setBackgroundMode(Qt::OpaqueMode);
        painter->setBrush(QBrush(Qt::white, Qt::SolidPattern));
        painter->drawPath(transition->shape());
        
        painter->setPen(QPen(QBrush(Qt::black), 1.0));
        painter->setBackgroundMode(Qt::TransparentMode);
        
        QRectF boundingRect = transition->boundingRect();
        boundingRect.setWidth(boundingRect.width() - portDefaultWidth());
        boundingRect.setHeight(boundingRect.height() - portDefaultWidth());
        boundingRect.translate(portDefaultWidth() / 2.0, portDefaultWidth() / 2.0);
        
        painter->drawText(boundingRect, Qt::AlignCenter | Qt::TextWordWrap, transition->title());
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
