#include "task.h"

#include <sdpa/daemon/NotificationEvent.hpp>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <cmath>
#include <math.h>

//! [0]
Task::Task( QString const & _c
          , QString const & _n
          , QString const & _id
          , QGraphicsItem * parent
          )
  : QGraphicsItem (parent)
  , m_component(_c)
  , m_name (_n)
  , m_id (_id)
  , text(0)
  , color(128, 128, 128)
  , length (50)
  , m_state (0)
{
  text = new QGraphicsSimpleTextItem(m_name, this);
  QFont font = text->font();
  font.setPointSize(7);
  text->setFont (font);

  setToolTip(m_name+" on "+m_component+" (id = "+m_id+")");

  update_task_state(sdpa::daemon::NotificationEvent::STATE_CREATED);
}

//! [0]

//! [1]

QRectF Task::boundingRect() const
{
  qreal adjust = 0.5;
  return QRectF(0 - adjust, 0 - adjust,
               length + adjust, 8 + adjust);
}

void Task::update_task_state(int state)
{
  typedef sdpa::daemon::NotificationEvent event_t;
  switch (state)
  {
  case event_t::STATE_STARTED:
    color = QColor(255,255,0);  // yellow
    break;
  case event_t::STATE_FINISHED:
    color = QColor(0,200,0);    // green
    break;
  case event_t::STATE_FAILED:
    color = QColor(255,0,0);    // red
    break;
  case event_t::STATE_CANCELLED:
    color = QColor(165,42,42);  // violett
    break;
  case event_t::STATE_CREATED:
  default:
    color = QColor(128,128,128); // grey
    break;
  }

  m_state = state;
}

void Task::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(color);
    painter->drawRect(0, 0, length, 8);
}

void Task::advance(int step)
{
  const qreal velocity = 1.0;
  if (!step)
    return;
  if (m_state < sdpa::daemon::NotificationEvent::STATE_FINISHED)
  {
    prepareGeometryChange();
    length += velocity;
  }
}
//! [11]
