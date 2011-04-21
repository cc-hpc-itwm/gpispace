#ifndef GRAPHPORT_HPP
#define GRAPHPORT_HPP 1

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
        public:          
          Port(Transition* parent, eDirection direction);
          
          const bool& highlighted() const;
          const qreal& length() const;
    
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
          virtual QRectF boundingRect() const;
          virtual void paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
          
          virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
          virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
          virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
          virtual void mousePressEvent (QGraphicsSceneMouseEvent * event );
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent * event );
    
        private:
          bool createPendingConnectionIfPossible();
          bool connectPendingConnection();
          
          QPointF _dragStart;
          
          bool _dragging;
          bool _highlighted;
          
          qreal _length;
      };
    }
  }
}

#endif
