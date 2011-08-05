#include "GraphTransitionCogWheelButton.hpp"

#include <QSizeF>
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QImage>

//! \note popover content. to be removed.
#include <QWidget>
#include <QPushButton>

#include "GraphTransition.hpp"
#include "PopoverWidget.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      TransitionCogWheelButton::TransitionCogWheelButton(Transition* linkedTransition)
      : QGraphicsItem(linkedTransition)
      , _linkedTransition(linkedTransition)
      {
        const QPointF padding(5.0, 5.0);                                        // hardcoded constant
        setPos(parentItem()->boundingRect().bottomRight() - boundingRect().bottomRight() - padding);
      }

      QRectF TransitionCogWheelButton::boundingRect() const
      {
        return QRectF(0.0, 0.0, 22.0, 22.0);                                    // hardcoded constant
      }

      void TransitionCogWheelButton::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
      {
        //! \todo Use licenced icon.
        painter->drawImage(QPointF(), QImage(":/cogwheel.png"), QRectF(0.0, 0.0, 22.0, 22.0));
      }

      void TransitionCogWheelButton::mousePressEvent(QGraphicsSceneMouseEvent* event)
      {
        //! \note Needed to get the release event later.
        event->accept();
      }

      void TransitionCogWheelButton::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
      {
        //! \todo Real content of the popover.
        QWidget* content = new QPushButton("aloha");
        content->setMinimumSize(QSize(100,100));

        QPointF clickPosition = event->pos();
        QPointF cursorPosition = QCursor::pos();

        ui::PopoverWidget* popupWidget = new ui::PopoverWidget(content);
        popupWidget->move((cursorPosition - clickPosition).toPoint() + popupWidget->arrowAdjustment());
        popupWidget->show();
      }
    }
  }
}
