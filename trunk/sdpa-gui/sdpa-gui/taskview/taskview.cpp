#include "taskview.hpp"
#include "taskscene.hpp"

namespace fhg
{
  namespace taskview
  {
    TaskView::TaskView(TaskScene *scene, QWidget *parent)
      : QGraphicsView(scene, parent)
    {}

    TaskView::TaskView (QWidget *parent)
      : QGraphicsView (parent)
    {}

    TaskScene *TaskView::scene () const
    {
      return qobject_cast<TaskScene*>(QGraphicsView::scene());
    }

    void TaskView::setScene(TaskScene *scene)
    {
      QGraphicsView::setScene(scene);
    }
  }
}
