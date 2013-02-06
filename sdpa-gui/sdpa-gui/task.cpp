#include "task.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

#include <cmath>

#include <stdexcept>
//! [0]
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

Task::Task( QString const & _c
          , QString const & _n
          , QString const & _id
          , QGraphicsItem * parent
          )
  : QGraphicsItem (parent)
  , m_component(_c)
  , m_name (_n)
  , m_id (_id)
  , length (1)
  , m_state (sdpa::daemon::NotificationEvent::STATE_CREATED)
  , color (color_for_state (m_state))
{
  setToolTip(m_name+" on "+m_component+" (id = "+m_id+")");
}

QRectF Task::boundingRect() const
{
  qreal adjust = 0.5;
  return QRectF(0 - adjust, 0 - adjust,
               length + adjust, 8 + adjust);
}

void Task::update_task_state(int state)
{
  m_state = sdpa::daemon::NotificationEvent::state_t (state);
  color = color_for_state (m_state);
}

void Task::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(color);
    //    painter->setPen(Qt::NoPen);
    painter->drawRect(0, 0, (int)std::floor(length+0.5), 8);
}

void Task::advance(int step)
{
  const qreal velocity = 1.0;
  if (!step)
    return;
  if (m_state < sdpa::daemon::NotificationEvent::STATE_FINISHED)
  {
    /*
    prepareGeometryChange();
    //    length += velocity;
    length = std::max( scene()->width() - pos().x()
                     , length + velocity
                     );
    */
    length = scene()->width() - pos().x();
  }
}
//! [11]
