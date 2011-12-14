// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/dock_widget.hpp>

#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      //! \todo When tabbed, do not show title bar, let be dragged via tab.
      dock_widget::dock_widget (const QString& title)
        : QDockWidget (title, 0, 0)
      {
        init();
      }
      dock_widget::dock_widget (const QString& title, QWidget* widget)
        : QDockWidget (title, 0, 0)
      {
        init();
        setWidget (widget);
        hide();
      }

      void dock_widget::init()
      {
        setAllowedAreas (Qt::LeftDockWidgetArea);

        setFeatures ( QDockWidget::DockWidgetClosable
                    | QDockWidget::DockWidgetMovable
                    );
      }
    }
  }
}
