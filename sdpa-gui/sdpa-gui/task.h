#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>

//! [0]
class Task : public QGraphicsItem
{
public:
  Task( QString const & component
      , QString const & name
      , QString const & id
      , QGraphicsItem * parent = 0
      );
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
  QGraphicsSimpleTextItem *text;
  QColor color;
  qreal length;

  int m_state;
};
//! [0]

#endif
