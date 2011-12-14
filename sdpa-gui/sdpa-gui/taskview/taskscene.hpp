#ifndef TASKVIEW_TASK_SCENE_HPP
#define TASKVIEW_TASK_SCENE_HPP 1

#include <QObject>
#include <QGraphicsScene>

namespace fhg
{
  namespace taskview
  {
    class TaskScene : public QGraphicsScene
    {
      Q_OBJECT
    public:
      TaskScene (QObject * parent = 0);
    };
  }
}

#endif
