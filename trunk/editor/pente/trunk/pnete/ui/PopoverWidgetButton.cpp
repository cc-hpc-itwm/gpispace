#include "PopoverWidgetButton.hpp"

#include <QSizeF>
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

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
        const qreal padding = 5.0;                                              // hardcoded constant
        const QSizeF temp = parentItem()->boundingRect().size() - boundingRect().size();
        setPos( QPointF( temp.width() - padding, temp.height() - padding ) );
      }

      QRectF TransitionCogWheelButton::boundingRect() const
      {
        return QRectF(0.0, 0.0, 20.0, 20.0);
      }

      void TransitionCogWheelButton::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
      {
        //! \todo Draw cogwheel.
        painter->drawRect(boundingRect());
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
