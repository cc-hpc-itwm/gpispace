// bernd.loerwald@itwm.fraunhofer.de

#ifndef UI_DOCKABLE_GRAPH_VIEW_HPP
#define UI_DOCKABLE_GRAPH_VIEW_HPP 1

#include <QObject>
#include <QDockWidget>

class QObject;
class QFocusEvent;

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

      class dockable_graph_view : public QDockWidget
      {
        Q_OBJECT
        public:
          dockable_graph_view (graph::Scene* scene);

          GraphView* graph_view() const;

        protected:
          virtual void focusInEvent (QFocusEvent*);

        signals:
          void focus_gained(QWidget*);

        private:
          GraphView* _graph_view;
      };

    }
  }
}

#endif
