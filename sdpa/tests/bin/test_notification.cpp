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
  TestObservable ()
    : m_names()
  {
    m_names.push_back ("test-observable");
  }

    void activityStateUpdate(const std::string &id, const std::string &name, NotificationEvent::state_t s)
    {
      notifyObservers(NotificationEvent(m_names, id, name, s));
    }

private:
  std::list<std::string> m_names;
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

  NotificationService gui("SDPA", gui_location);

  daemon.attach_observer( &gui );

  for (size_t i = 0; i < 50 ; ++i)
  {
    std::ostringstream ostr;
    ostr << "activity-" << i;
    const std::string aid(ostr.str());

    daemon.activityStateUpdate(aid, "function placeholder", NotificationEvent::STATE_STARTED);

    int random_outcome = (int)(round(3 * drand48()));
	switch (random_outcome)
	{
	  case 0:
		daemon.activityStateUpdate(aid, "function placeholder", NotificationEvent::STATE_FINISHED);
		break;
	  case 1:
		daemon.activityStateUpdate(aid, "function placeholder", NotificationEvent::STATE_FAILED);
		break;
	  case 2:
		daemon.activityStateUpdate(aid, "function placeholder", NotificationEvent::STATE_CANCELLED);
		break;
	}

    if ( (i % 10) == 0 ) boost::this_thread::sleep(boost::posix_time::seconds(1));
  }

  for (size_t i = 0; i < 200 ; ++i)
  {
    std::ostringstream ostr;
    ostr << "activity-" << i;
    const std::string aid(ostr.str());

    int random_state = (int)(round(NotificationEvent::STATE_MAX * drand48()));
    NotificationEvent::state_t state = static_cast<NotificationEvent::state_t>(random_state);
    daemon.activityStateUpdate(aid, "function placeholder", state);
    if ( (i % 100) == 0 ) boost::this_thread::sleep(boost::posix_time::seconds(1));
  }
}
