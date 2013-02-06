#ifndef TASK_H
#define TASK_H

#include <sdpa/daemon/NotificationEvent.hpp>

#include <QGraphicsRectItem>
#include <QString>

class Task : public QGraphicsRectItem
{
public:
  Task ( const QString& component
       , const QString& name
       , const QString& id
       , QGraphicsItem* parent = NULL
       );
  virtual ~Task () {}

  void update_task_state (sdpa::daemon::NotificationEvent::state_t);

  void advance (const qreal scene_width);

private:
  bool _do_advance;
};

#endif
