#include "taskview.hpp"
#include "taskscene.hpp"

namespace fhg
{
  namespace taskview
  {
    TaskView::TaskView(TaskScene *scene, QWidget *parent)
      : QGraphicsView(scene, parent)
    {
      initialize();
    }

    TaskView::TaskView (QWidget *parent)
      : QGraphicsView (parent)
    {
      initialize();
    }

    TaskScene *TaskView::scene () const
    {
      return qobject_cast<TaskScene*>(QGraphicsView::scene());
    }

    void TaskView::setScene(TaskScene *scene)
    {
      QGraphicsView::setScene(scene);
    }

    void TaskView::initialize()
    {
      //      setAttribute (Qt::WA_AlwaysShowToolTips);
      setTransformationAnchor(QGraphicsView::AnchorViewCenter);
      setAlignment(Qt::AlignRight | Qt::AlignTop);
      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
      setCacheMode(QGraphicsView::CacheNone);
      setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
      setDragMode(QGraphicsView::ScrollHandDrag);
    }
  }
}
