#ifndef _PNETE_UI_GRAPH_COGWHEEL_BUTTON_HPP
#define _PNETE_UI_GRAPH_COGWHEEL_BUTTON_HPP 1

#include <QRectF>

class QPainter;
class QGraphicsSceneMouseEvent;

#include <pnete/ui/graph/item.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace transition { class item; }

        class cogwheel_button : public item
        {
        public:
          cogwheel_button (transition::item* linked_transition);
          virtual QRectF boundingRect() const;
          virtual void paint (QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

        protected:
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);
          virtual void mousePressEvent (QGraphicsSceneMouseEvent* event);

        private:
          transition::item* _linked_transition;
        };
      }
    }
  }
}

#endif
