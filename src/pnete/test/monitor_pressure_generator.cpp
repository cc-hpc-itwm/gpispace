// bernd.loerwald@itwm.fraunhofer.de
//! \note This "test" does not test anything, but is a pressure-generator for sdpa-gui only.

#include <fhglog/fhglog.hpp>
#include <cstdlib>
#include <boost/format.hpp>
#include <boost/optional.hpp>

#include <sdpa/daemon/NotificationService.hpp>

#include <we/mgmt/type/activity.hpp>
#include <we/type/transition.hpp>

using namespace sdpa::daemon;

static uint64_t id_counter;

struct activity
{
  activity (const std::string& worker)
    : _id ((boost::format ("%1%") % ++id_counter).str())
    , _workers()
    , _state (NotificationEvent::STATE_STARTED)
    , _act ( we::type::transition_t ( "activity-" + _id
                                    , we::type::expression_t()
                                    , condition::type ("true")
                                    , true
                                    , we::type::property::type()
                                    )
           , boost::none
           )
  {
    _workers.push_back (worker);
  }

  void send_out_notification ( const NotificationService* service_a
                             , const NotificationService* service_b
                             ) const
  {
    const NotificationEvent event (_workers, _id, _state, _act);
    service_a->notify (event);
    service_b->notify (event);
  }

  bool next_state()
  {
    if (_state != NotificationEvent::STATE_STARTED)
    {
      return false;
    }

    switch (lrand48() % 3)
    {
    case 0:
      _state = NotificationEvent::STATE_FINISHED;
      break;
    case 1:
      _state = NotificationEvent::STATE_FAILED;
      break;
    case 2:
      _state = NotificationEvent::STATE_CANCELED;
      break;
    }

    return true;
  }

  std::string _id;
  std::list<std::string> _workers;
  sdpa::daemon::NotificationEvent::state_t _state;
  we::mgmt::type::activity_t _act;
};

std::string worker_gen()
{
  static long ids[]          = {0,      0,      0,       0,     0,     0};
  static const char* names[] = {"calc", "load", "store", "foo", "bar", "baz"};

  const unsigned long r (lrand48() % sizeof(names)/sizeof(*names));
  return (boost::format ("%1%-%2%") % names[r] % ++ids[r]).str();
}

int main(int ac, char **av)
{
  if (ac < 3)
  {
    std::cerr << av[0]
              << " port_a port_b <worker_count=1> <notification_per_second<=1000=1>\n";
    return -1;
  }

  const int port_a (atoi (av[1]));
  const int port_b (atoi (av[2]));
  const int worker_count (ac >= 4 ? atoi (av[3]) : 1);
  const int duration (ac >= 5 ? 1000 / atoi (av[4]) : 1);

  const NotificationService service_a
    ((boost::format ("localhost:%1%") % port_a).str());
  const NotificationService service_b
    ((boost::format ("localhost:%1%") % port_b).str());

  std::vector<std::string> worker_names (worker_count);
  std::generate (worker_names.begin(), worker_names.end(), worker_gen);

  std::map<std::string, boost::optional<activity> > workers;

  for (;;)
  {
    const std::string worker (worker_names[lrand48() % worker_names.size()]);

    if (!workers[worker])
    {
      workers[worker] = activity (worker);
    }

    workers[worker]->send_out_notification (&service_a, &service_b);

    if (!workers[worker]->next_state())
    {
      workers[worker] = boost::none;
    }

    boost::this_thread::sleep (boost::posix_time::milliseconds (duration));
  }

  //! \note Wait for remote logger being done.
  //! \todo Wait more cleverly.
  boost::this_thread::sleep (boost::posix_time::seconds (2));

  return 0;
}
