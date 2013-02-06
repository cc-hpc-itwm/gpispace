#ifndef TASK_H
#define TASK_H

#include <sdpa/daemon/NotificationEvent.hpp>

#include <QString>
#include <QGraphicsItem>

//! [0]
class Task : public QGraphicsItem
{
public:
  Task( QString const & component
      , QString const & name
      , QString const & id
      , QGraphicsItem * parent = 0
      );
  virtual ~Task () {}

  QRectF boundingRect() const;
  void paint( QPainter *painter
            , const QStyleOptionGraphicsItem *option
            , QWidget *widget
            );

  void update_task_state (int state);

 protected:
  void advance(int step);

private:
  QString m_component;
  QString m_name;
  QString m_id;

  sdpa::daemon::NotificationEvent::state_t m_state;
  QColor color;
  qreal length;
};
//! [0]

#endif
