#ifndef TASKVIEW_TASK_VIEW_HPP
#define TASKVIEW_TASK_VIEW_HPP 1

#include <QObject>
#include <QGraphicsView>

namespace fhg
{
  namespace taskview
  {
    class TaskScene;

    class TaskView : public QGraphicsView
    {
      Q_OBJECT

    public:
      explicit
      TaskView (TaskScene *, QWidget *parent = 0);

      TaskView (QWidget *parent = 0);

      TaskScene *scene () const;
      void setScene(TaskScene *scene);
    private:
      void initialize();
    };
  }
}

#endif
