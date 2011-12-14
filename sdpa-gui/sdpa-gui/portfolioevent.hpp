#ifndef PORTFOLIO_EVENT_HPP
#define PORTFOLIO_EVENT_HPP

#include <QEvent>

class PortFolioEvent : public QEvent
{
public:
    explicit
    PortFolioEvent( int typ
                  , sdpa::daemon::NotificationEvent const & evt
                  , we::activity_t const & act
                  )
      : QEvent ((QEvent::Type)typ)
      , notification (evt)
      , activity(act)
  {}

  sdpa::daemon::NotificationEvent notification;
  we::activity_t activity;
};

#endif
