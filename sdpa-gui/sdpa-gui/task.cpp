#include "task.h"

#include <QGraphicsScene>
#include <QSettings>

#include <cmath>
#include <stdexcept>

namespace
{
  QColor color_for_state (const sdpa::daemon::NotificationEvent::state_t& state)
  {
    typedef sdpa::daemon::NotificationEvent event;

    QSettings settings;

    switch (state)
    {
    case event::STATE_CREATED:
      return settings.value ("gantt/created").value<QColor>();

    case event::STATE_STARTED:
      return settings.value ("gantt/started").value<QColor>();

    case event::STATE_FINISHED:
      return settings.value ("gantt/finished").value<QColor>();

    case event::STATE_FAILED:
      return settings.value ("gantt/failed").value<QColor>();

    case event::STATE_CANCELLED:
      return settings.value ("gantt/cancelled").value<QColor>();
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
  _state = state;
  _do_advance = _state < sdpa::daemon::NotificationEvent::STATE_FINISHED;
  reset_color();
}

void Task::reset_color()
{
  setBrush (color_for_state (_state));
}

void Task::advance (const qreal scene_width)
{
  if (_do_advance)
  {
    static const qreal height (8.0);

    setRect (0.0, 0.0, std::floor (scene_width - pos().x() + 0.5), height);
  }
}
