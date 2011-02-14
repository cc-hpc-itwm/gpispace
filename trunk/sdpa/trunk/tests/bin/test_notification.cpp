/*
 * =====================================================================================
 *
 *       Filename:  test_notification.cpp
 *
 *    Description:  test the notification service
 *
 *        Version:  1.0
 *        Created:  11/19/2009 12:27:14 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <fhglog/fhglog.hpp>
#include <cstdlib>

#include <sdpa/daemon/Observable.hpp>
#include <sdpa/daemon/NotificationService.hpp>

using namespace sdpa::daemon;

class TestObservable : public sdpa::daemon::Observable
{
  public:
    virtual ~TestObservable() {}

    void activityStateUpdate(const std::string &id, const std::string &name, NotificationEvent::state_t s)
    {
      notifyObservers(NotificationEvent(id, name, s));
    }

    void activityStarted(const std::string &id, const std::string &name)
    {
      notifyObservers(NotificationEvent(id, name, NotificationEvent::STATE_STARTED));
    }
    void activityCreated(const std::string &id, const std::string &name)
    {
      notifyObservers(NotificationEvent(id, name, NotificationEvent::STATE_CREATED));
    }
};

int main(int ac, char **av)
{
  std::string gui_location("127.0.0.1:9000");
  if (ac > 1)
  {
    gui_location = av[1];
  }

  std::clog << "initializing notification service..." << std::endl;
  TestObservable daemon;

  typedef NotificationService gui_service;
  gui_service gui("SDPA", gui_location);

  daemon.attach_observer( &gui );

  for (size_t i = 0; i < 50 ; ++i)
  {
    std::ostringstream ostr;
    ostr << "activity-" << i;
    const std::string aid(ostr.str());

    daemon.activityStateUpdate(aid, "function placeholder", gui_service::event_t::STATE_CREATED);
    daemon.activityStateUpdate(aid, "function placeholder", gui_service::event_t::STATE_STARTED);

    int random_outcome = (int)(round(3 * drand48()));
	switch (random_outcome)
	{
	  case 0:
		daemon.activityStateUpdate(aid, "function placeholder", gui_service::event_t::STATE_FINISHED);
		break;
	  case 1:
		daemon.activityStateUpdate(aid, "function placeholder", gui_service::event_t::STATE_FAILED);
		break;
	  case 2:
		daemon.activityStateUpdate(aid, "function placeholder", gui_service::event_t::STATE_CANCELLED);
		break;
	}

    if ( (i % 10) == 0 ) boost::this_thread::sleep(boost::posix_time::seconds(1));
  }

  for (size_t i = 0; i < 200 ; ++i)
  {
    std::ostringstream ostr;
    ostr << "activity-" << i;
    const std::string aid(ostr.str());

    int random_state = (int)(round(gui_service::event_t::STATE_MAX * drand48()));
    gui_service::event_t::state_t state = static_cast<gui_service::event_t::state_t>(random_state);
    daemon.activityStateUpdate(aid, "function placeholder", state);
    if ( (i % 100) == 0 ) boost::this_thread::sleep(boost::posix_time::seconds(1));
  }
}
