#ifndef _PNETE_UI_GRAPH_COGWHEEL_BUTTON_HPP
#define _PNETE_UI_GRAPH_COGWHEEL_BUTTON_HPP 1

#include <QGraphicsItem>
#include <QRectF>

class QPainter;
class QGraphicsSceneMouseEvent;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class transition;

        class cogwheel_button : public QGraphicsItem
        {
        public:
          cogwheel_button (transition* linked_transition);
          virtual QRectF boundingRect() const;
          virtual void paint (QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

        protected:
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);
          virtual void mousePressEvent (QGraphicsSceneMouseEvent* event);

        private:
          transition* _linked_transition;
        };
      }
    }
  }
}

#endif
