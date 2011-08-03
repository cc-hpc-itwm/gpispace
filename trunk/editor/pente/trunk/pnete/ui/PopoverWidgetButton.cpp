#include "PopoverWidgetButton.hpp"

#include <QAction>
#include <QIcon>
#include <QToolButton>
#include <QGraphicsScene>

//! \note Only for testing content.
#include <QPushButton>

#include "PopoverWidget.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      PopoverWidgetButton::PopoverWidgetButton(QGraphicsItem* parent)
      : QGraphicsProxyWidget(parent)
      , _cogwheelButton(new QToolButton())
      , _popup(NULL)
      {
        QAction* cogwheelAction = new QAction( QIcon(":/lock.png"), tr("Preferences of this transition"), this );
        connect(cogwheelAction, SIGNAL(triggered()), SLOT(openPopover()));
        _cogwheelButton->setDefaultAction( cogwheelAction );
        _cogwheelButton->setAutoRaise(true);
        setWidget(_cogwheelButton);

        const qreal padding = 5.0;                                              // hardcoded constant

        QSizeF temp = parent->boundingRect().size() - _cogwheelButton->size();
        setPos( QPointF( temp.width() - padding, temp.height() - padding ) );
      }

      void PopoverWidgetButton::openPopover()
      {
        //! \todo Attach a parent-less PopoverWidget to the button's position.
        //! \note Do _not_ add this to the scene, so it wont get hidden when scrolling away.
        //! \note Still have this change it's position when scrolling, moving the item and stuff.
        if(_popup)
        {
          scene()->removeItem(_popup);
          delete _popup;
          _popup = NULL;
        }
        _popup = scene()->addWidget(new PopoverWidget(_cogwheelButton, new QPushButton("aloha")), Qt::Popup);
        _popup->setPos(scenePos() - QPointF( -10.0, 23.0 ));
      }
    }
  }
}
