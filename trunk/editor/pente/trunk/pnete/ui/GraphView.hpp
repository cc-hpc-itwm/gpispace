#ifndef UIGRAPHVIEW_HPP
#define UIGRAPHVIEW_HPP 1

#include <QGraphicsView>
#include <QObject>

class QWidget;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;

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
          GraphView(QWidget* parent = NULL);
          
        protected:
          virtual void dragEnterEvent(QDragEnterEvent* event);
          virtual void dragMoveEvent(QDragMoveEvent* event);
          virtual void dropEvent(QDropEvent* event);
      };
    }
  }
}

#endif
