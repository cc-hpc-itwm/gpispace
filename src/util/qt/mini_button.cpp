// bernd.loerwald@itwm.fraunhofer.de

#include <util/qt/mini_button.hpp>

#include <QAction>
#include <QPainter>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      mini_button::mini_button (QStyle::StandardPixmap icon, QWidget* parent)
        : QToolButton (parent)
      {
        QAction* action (new QAction (this));
        action->setIcon (style()->standardIcon (icon));
        setDefaultAction (action);
      }
      mini_button::mini_button (QAction* action, QWidget* parent)
        : QToolButton (parent)
      {
        setDefaultAction (action);
      }

      QSize mini_button::sizeHint() const
      {
        if (icon().isNull())
        {
          return QToolButton::sizeHint();
        }

        ensurePolished();

        const int icon_size
          (style()->pixelMetric (QStyle::PM_SmallIconSize, 0, this));
        const QSize actual_icon_size
          (icon().actualSize (QSize (icon_size, icon_size)));
        const int side_length
          ( 2
          * style()->pixelMetric (QStyle::PM_DockWidgetTitleBarButtonMargin, 0, this)
          + qMax (actual_icon_size.width(), actual_icon_size.height())
          );
        return QSize (side_length, side_length);
      }
      QSize mini_button::minimumSizeHint() const
      {
        return sizeHint();
      }
    }
  }
}
