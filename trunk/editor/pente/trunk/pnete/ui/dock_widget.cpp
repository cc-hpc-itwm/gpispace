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
        : QDockWidget (title)
      {
        setFeatures ( QDockWidget::DockWidgetClosable
                    | QDockWidget::DockWidgetMovable
                    );
        setAllowedAreas (Qt::LeftDockWidgetArea);
      }
    }
  }
}
