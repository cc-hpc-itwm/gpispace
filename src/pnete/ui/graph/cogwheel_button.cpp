#include <pnete/ui/graph/cogwheel_button.hpp>

#include <QSizeF>
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QImage>

//! \note popover content. to be removed.
#include <QWidget>
#include <QPushButton>

#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/popover_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        cogwheel_button::cogwheel_button (transition_item* linked_transition)
          : base_item (linked_transition)
            //          , _linked_transition (linked_transition)
        {
          const QPointF padding (5.0, 5.0);                                        // hardcoded constant
          //! \todo set on resize
          setPos ( parentItem()->boundingRect().bottomRight()
                 - boundingRect().bottomRight()
                 - padding
                 );
        }

        QRectF cogwheel_button::boundingRect() const
        {
          return QRectF (0.0, 0.0, 22.0, 22.0);                                    // hardcoded constant
        }

        void cogwheel_button::paint ( QPainter* painter
                                    , const QStyleOptionGraphicsItem*
                                    , QWidget*
                                    )
        {
          //! \todo Use licenced icon.
          painter->drawImage ( QPointF()
                             , QImage(":/cogwheel.png")
                             , QRectF(0.0, 0.0, 22.0, 22.0)
                             );
        }

        void cogwheel_button::mousePressEvent (QGraphicsSceneMouseEvent* event)
        {
          //! \note Needed to get the release event later.
          event->accept();
        }

        void cogwheel_button::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
        {
          event->accept();

          //! \todo Real content of the popover.
          QWidget* content (new QPushButton ("aloha"));
          content->setMinimumSize (QSize (100,100));

          const QPointF clickPosition (event->pos());
          const QPointF cursorPosition (QCursor::pos());

          popover_widget* popupWidget (new popover_widget (content));
          popupWidget->move( (cursorPosition - clickPosition).toPoint()
                           + popupWidget->arrowAdjustment()
                           );
          popupWidget->show();
        }
      }
    }
  }
}
