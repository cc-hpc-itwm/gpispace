#include "task.h"

#include <QGraphicsScene>

#include <cmath>
#include <stdexcept>

namespace
{
  QColor color_for_state (const sdpa::daemon::NotificationEvent::state_t& state)
  {
    typedef sdpa::daemon::NotificationEvent event;
    switch (state)
    {
    case event::STATE_CREATED:
      return QColor (128, 128, 128); // grey

    case event::STATE_STARTED:
      return QColor (255, 255, 0);   // yellow

    case event::STATE_FINISHED:
      return QColor (0, 200, 0);     // green

    case event::STATE_FAILED:
      return QColor (255, 0, 0);     // red

    case event::STATE_CANCELLED:
      return QColor (165, 42, 42);   // violet
    }

    throw std::runtime_error ("invalid state");
  }
}

Task::Task ( const QString& component
           , const QString& name
           , const QString& id
           , QGraphicsItem* parent
           )
  : QGraphicsRectItem (parent)
  , _do_advance (true)
{
  setToolTip (QObject::tr ("%1 on %2 (id = %3)").arg (name, component, id));
  setBrush (color_for_state (sdpa::daemon::NotificationEvent::STATE_CREATED));
}

void Task::update_task_state (sdpa::daemon::NotificationEvent::state_t state)
{
  setBrush (color_for_state (state));
  _do_advance = state < sdpa::daemon::NotificationEvent::STATE_FINISHED;
}

void Task::advance (const qreal scene_width)
{
  if (_do_advance)
  {
    static const qreal height (8.0);

    setRect (0.0, 0.0, std::floor (scene_width - pos().x() + 0.5), height);
  }
}
