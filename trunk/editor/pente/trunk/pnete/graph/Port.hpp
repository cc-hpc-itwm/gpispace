#ifndef GRAPHPORT_HPP
#define GRAPHPORT_HPP 1

#include <QObject>

#include "ConnectableItem.hpp"
#include "ItemTypes.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Transition;
      
      class Port : public ConnectableItem
      {
        Q_OBJECT
        
        public:
          Port(Transition* parent, eDirection direction, const QString& title, const QString& dataType, bool notConnectable = false);
          
          const bool& highlighted() const;
          const qreal& length() const;
          
          const QString& title() const;
          const QString& dataType() const;
          
          const bool& notConnectable() const;
          
          virtual QRectF boundingRect() const;
          
          void deleteConnection();
         
          virtual bool canConnectTo(ConnectableItem* other) const;
          virtual bool canConnectIn(eDirection thatDirection) const;
          
          QPointF snapToEdge(const QPointF& position, eOrientation edge) const;
          eOrientation getNearestEdge(const QPointF& position) const;
          QPointF checkForMinimumDistance(const QPointF& position) const;
    
          enum 
          {
            Type = PortType,
          };
          virtual int type() const
          {
            return Type;
          }
          
        protected:
          virtual QPainterPath shape() const;
          virtual void paint(QPainter*painter, const QStyleOptionGraphicsItem*option, QWidget*widget);
          
          virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
          virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
          virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
          virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
          virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    
        private:
          QString _title;
          QString _dataType;
          
          QPointF _dragStart;
          
          bool _dragging;
          bool _highlighted;
          bool _notConnectable;
          
          qreal _length;
      };
    }
  }
}

#endif
