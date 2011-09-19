// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/dock_widget.hpp>

#include <QWidget>
#include <QString>
#include <pnete/ui/GraphScene.hpp>
#include <pnete/ui/expression_widget.hpp>
#include <pnete/ui/module_call_widget.hpp>
#include <pnete/ui/net_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
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
