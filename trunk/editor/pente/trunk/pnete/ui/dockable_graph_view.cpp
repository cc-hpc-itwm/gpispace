// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/dockable_graph_view.hpp>

#include <QWidget>

#include <pnete/ui/GraphView.hpp>
#include <pnete/ui/GraphScene.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      dockable_graph_view::dockable_graph_view (graph::Scene* scene)
      : QDockWidget (scene->name())
      , _graph_view (new GraphView(scene, this))
      {
        setWidget (_graph_view);
        setFeatures ( QDockWidget::DockWidgetClosable
                    | QDockWidget::DockWidgetMovable
                    );
        setAllowedAreas (Qt::LeftDockWidgetArea);
      }

      //! \todo Its raise being called or something, not focusIn.
      void dockable_graph_view::focusInEvent (QFocusEvent* event)
      {
        emit focus_gained (this);
        QDockWidget::focusInEvent (event);
      }

      GraphView* dockable_graph_view::graph_view() const
      {
        return _graph_view;
      }
    }
  }
}
