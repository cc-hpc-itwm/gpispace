#ifndef GRAPHTRANSITIONCOGWHEELBUTTON_HPP
#define GRAPHTRANSITIONCOGWHEELBUTTON_HPP 1

#include <QGraphicsItem>
#include <QRectF>

class QPainter;
class QGraphicsSceneMouseEvent;

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Transition;

      class TransitionCogWheelButton : public QGraphicsItem
      {
        public:
          TransitionCogWheelButton(Transition* linkedTransition);
          virtual QRectF boundingRect() const;
          virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

        protected:
          virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
          virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);

        private:
          Transition* _linkedTransition;
      };
    }
  }
}

#endif
