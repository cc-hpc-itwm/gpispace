// bernd.loerwald@itwm.fraunhofer.de

#ifndef UI_GRAPHVIEW_HPP
#define UI_GRAPHVIEW_HPP 1

#include <QGraphicsView>
#include <QObject>

class QWidget;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QGraphicsScene;
class QWheelEvent;
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
      class GraphView : public QGraphicsView
      {
        Q_OBJECT

        public:
          GraphView (graph::Scene* scene, QWidget* parent = NULL);

          void emit_current_zoom_level();

          graph::Scene* scene() const;
          void setScene(graph::Scene* scene);

        public slots:
          void zoom (int to);
          void zoom_in();
          void zoom_out();
          void reset_zoom();

        signals:
          void zoomed (int to);
          void focus_gained (QWidget* me);

        protected:
          virtual void dragEnterEvent (QDragEnterEvent* event);
          virtual void dragMoveEvent (QDragMoveEvent* event);
          virtual void dropEvent (QDropEvent* event);
          virtual void wheelEvent (QWheelEvent* event);
          virtual void focusInEvent (QFocusEvent* event);

          virtual QSize sizeHint() const;

        private:
          int _currentScale;
      };
    }
  }
}

#endif
