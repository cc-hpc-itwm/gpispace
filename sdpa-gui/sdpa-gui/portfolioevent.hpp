#ifndef PORTFOLIO_EVENT_HPP
#define PORTFOLIO_EVENT_HPP

#include <QEvent>

#include <we/mgmt/type/activity.hpp>

class PortFolioEvent : public QEvent
{
public:
    explicit
    PortFolioEvent( int typ
                  , sdpa::daemon::NotificationEvent const & evt
                  , we::mgmt::type::activity_t const & act
                  )
      : QEvent ((QEvent::Type)typ)
      , notification (evt)
      , activity(act)
  {}

  sdpa::daemon::NotificationEvent notification;
  we::mgmt::type::activity_t activity;
};

#endif
