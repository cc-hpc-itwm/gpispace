#ifndef UIGRAPHVIEW_HPP
#define UIGRAPHVIEW_HPP 1

#include <QGraphicsView>
#include <QObject>

class QWidget;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QGraphicsScene;
class QWheelEvent;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class GraphView : public QGraphicsView
      {
        Q_OBJECT

        public:
          GraphView(QGraphicsScene* scene, QWidget* parent = NULL);

        public slots:
          void zoom(int to);

        signals:
          void zoomed(int to);

        protected:
          virtual void dragEnterEvent(QDragEnterEvent* event);
          virtual void dragMoveEvent(QDragMoveEvent* event);
          virtual void dropEvent(QDropEvent* event);
          virtual void wheelEvent(QWheelEvent* event);

        private:
          qreal _currentScale;
      };
    }
  }
}

#endif
