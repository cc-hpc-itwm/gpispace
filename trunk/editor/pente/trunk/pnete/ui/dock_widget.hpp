// bernd.loerwald@itwm.fraunhofer.de

#ifndef UI_DOCKABLE_DOCK_WIDGET_HPP
#define UI_DOCKABLE_DOCK_WIDGET_HPP 1

#include <QObject>
#include <QDockWidget>

#include <pnete/data/proxy.hpp>

#include <pnete/ui/base_editor_widget.hpp>

class QObject;
class QString;

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Scene;
    }
    namespace ui
    {
      class GraphView;

      class dock_widget : public QDockWidget
      {
        Q_OBJECT

      public:
        dock_widget (const QString& title);
      };
    }
  }
}

#endif
