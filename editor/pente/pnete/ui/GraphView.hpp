// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UI_GRAPH_VIEW_HPP
#define _PNETE_UI_GRAPH_VIEW_HPP 1

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
    namespace ui
    {
      namespace graph
      {
        namespace scene { class type; }
      }

      class GraphView : public QGraphicsView
      {
        Q_OBJECT

        public:
        GraphView (graph::scene::type* scene, QWidget* parent = NULL);

        void emit_current_zoom_level();

        graph::scene::type* scene() const;

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
