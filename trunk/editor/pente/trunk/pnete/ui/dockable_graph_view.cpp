// bernd.loerwald@itwm.fraunhofer.de

#include "dockable_graph_view.hpp"

#include <QWidget>

#include "GraphView.hpp"
#include "GraphScene.hpp"

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

      GraphView* dockable_graph_view::graph_view() const
      {
        return _graph_view;
      }
    }
  }
}
