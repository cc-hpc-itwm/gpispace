#ifndef GRAPHTRANSITION_HPP
#define GRAPHTRANSITION_HPP 1

#include <QGraphicsItem>
#include <QPainterPath>
#include <QRectF>
#include <QPointF>
#include <QSizeF>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

#include "ItemTypes.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Transition : public QGraphicsItem
      {
        public:
          Transition(const QString& title, QGraphicsItem* parent = NULL);
          
          virtual QRectF boundingRect() const;
          virtual QPainterPath shape() const;
          
          const QString& title() const;
          bool highlighted() const;
          
          enum 
          {
            Type = TransitionType,
          };
          virtual int type() const
          {
            return Type;
          }
      
        protected:
          virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
          
          virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
          virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
          virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
          virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
          virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
          
          QString _title;
          
          //! \todo größe verstellbar.
      
          QPointF _dragStart;
          QSizeF _size;
          
          bool _highlighted;
          bool _dragging;
      };
    }
  }
}

#endif
