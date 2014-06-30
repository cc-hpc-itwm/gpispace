#ifndef _PNETE_UI_GRAPH_COGWHEEL_BUTTON_HPP
#define _PNETE_UI_GRAPH_COGWHEEL_BUTTON_HPP 1

#include <QRectF>

class QPainter;
class QGraphicsSceneMouseEvent;

#include <pnete/ui/graph/base_item.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class transition_item;

        class cogwheel_button : public base_item
        {
        public:
          cogwheel_button (transition_item* linked_transition);
          virtual QRectF boundingRect() const override;
          virtual void paint (QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

        protected:
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event) override;
          virtual void mousePressEvent (QGraphicsSceneMouseEvent* event) override;

        private:
          //          transition_item* _linked_transition;
        };
      }
    }
  }
}

#endif
