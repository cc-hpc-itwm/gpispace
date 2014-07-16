// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_MINI_BUTTON_HPP
#define FHG_UTIL_QT_MINI_BUTTON_HPP

#include <util/qt/mini_button.fwd.hpp>

#include <QStyle>
#include <QToolButton>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class mini_button : public QToolButton
      {
        Q_OBJECT

      public:
        mini_button (QStyle::StandardPixmap icon, QWidget* parent = nullptr);
        mini_button (QAction*, QWidget* parent = nullptr);

        virtual QSize sizeHint() const override;
        virtual QSize minimumSizeHint() const override;
      };
    }
  }
}

#endif
