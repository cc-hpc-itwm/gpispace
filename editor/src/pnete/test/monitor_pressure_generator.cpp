// bernd.loerwald@itwm.fraunhofer.de
//! \note This "test" does not test anything, but is a pressure-generator for sdpa-gui only.

#include <fhglog/fhglog.hpp>
#include <cstdlib>
#include <boost/format.hpp>

#include <sdpa/daemon/Observable.hpp>
#include <sdpa/daemon/NotificationService.hpp>

using namespace sdpa::daemon;

static uint64_t id_counter;

struct activity
{
  activity (const std::string& worker)
    : _id (++id_counter)
    , _worker (worker)
    , _state (NotificationEvent::STATE_CREATED)
  { }

  void send_out_notification ( NotificationService* service_a
                             , NotificationService* service_b
                             ) const
  {
    const NotificationEvent event ( _worker
                                  , (boost::format ("%1%") % _id).str()
                                  , (boost::format ("activity-%1%") % _id).str()
                                  , _state
                                  );
    service_a->update (event);
    service_b->update (event);
  }

  bool next_state()
  {
    switch (_state)
    {
    case NotificationEvent::STATE_CREATED:
      _state = NotificationEvent::STATE_STARTED;
      break;

    case NotificationEvent::STATE_STARTED:
      switch (lrand48() % 3)
      {
      case 0:
        _state = NotificationEvent::STATE_FINISHED;
        break;
      case 1:
        _state = NotificationEvent::STATE_FAILED;
        break;
      case 2:
        _state = NotificationEvent::STATE_CANCELLED;
        break;
      }
      break;
    default:
      return false;
    }

    return true;
  }

  uint64_t _id;
  std::string _worker;
  sdpa::daemon::NotificationEvent::state_t _state;
};

std::string worker_gen()
{
  static long ids[]          = {0,      0,      0,       0,     0,     0};
  static const char* names[] = {"calc", "load", "store", "foo", "bar", "baz"};

  const long r (lrand48() % sizeof(names)/sizeof(*names));
  return (boost::format ("%1%-%2%") % names[r] % ++ids[r]).str();
}

int main(int ac, char **av)
{
  if (ac != 3)
  {
    std::cerr << av[0] << " port_a port_b\n";
    return -1;
  }

  const int port_a (atoi (av[1]));
  const int port_b (atoi (av[2]));

  NotificationService service_a
    ("service_a", (boost::format ("localhost:%1%") % port_a).str());
  NotificationService service_b
    ("service_b", (boost::format ("localhost:%1%") % port_b).str());
  service_a.open();
  service_b.open();

  std::vector<std::string> workers (2000);

  std::generate (workers.begin(), workers.end(), worker_gen);

  std::vector<activity> activities;

  for (;;)
  {
    switch (lrand48() % (activities.empty() ? 1 : 2))
    {
    case 0:
      activities.push_back (workers[lrand48() % workers.size()]);
      break;
    case 1:
      {
        const long r (lrand48() % activities.size());
        activity& a (activities[r]);
        a.send_out_notification (&service_a, &service_b);
        if (!a.next_state())
        {
          activities.erase (activities.begin() + r);
        }
      }
      break;
    }

    boost::this_thread::sleep (boost::posix_time::milliseconds (1));
  }

  //! \note Wait for remote logger being done.
  //! \todo Wait more cleverly.
  boost::this_thread::sleep (boost::posix_time::seconds (2));

  return 0;
}
