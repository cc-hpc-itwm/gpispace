// bernd.loerwald@itwm.fraunhofer.de
//! \note This "test" does not test anything, but is a pressure-generator for sdpa-gui only.

#include <fhglog/LogMacros.hpp>
#include <logging/legacy/emitter.hpp>
#include <logging/stream_emitter.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <we/type/activity.hpp>
#include <we/type/transition.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>

#include <boost/format.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <thread>
#include <vector>

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
                                    , boost::none
                                    , we::type::property::type()
                                    , we::priority_type()
                                    )
           , boost::none
           )
  {
    _workers.push_back (worker);
  }

  void send_out_notification ( fhg::logging::stream_emitter& emitter
                             , fhg::logging::legacy::emitter& legacy_emitter
                             ) const
  {
    const NotificationEvent event (_workers, _id, _state, _act);
    emitter.emit_message ({event.encoded(), sdpa::daemon::gantt_log_category});
    legacy_emitter.trace (event.encoded());
    static char const* arr[4] = { fhg::logging::legacy::category_level_trace
                                , fhg::logging::legacy::category_level_info
                                , fhg::logging::legacy::category_level_warn
                                , fhg::logging::legacy::category_level_error
                                };
    emitter.emit_message ({_id, arr[lrand48() % 4]});
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
  we::type::activity_t _act;
};

std::string worker_gen()
{
  static long ids[]          = {0,      0,      0,       0,     0,     0};
  static const char* names[] = {"calc", "load", "store", "foo", "bar", "baz"};

  const unsigned long r (lrand48() % sizeof(names)/sizeof(*names));
  return (boost::format ("%1%-ip-127-0-0-1 %3% 50501-%2%") % names[r] % ++ids[r] % fhg::util::syscall::getpid()).str();
}

int main(int ac, char **av)
try
{
  if (ac < 2)
  {
    std::cerr << av[0]
              << " legacy_port <worker_count=1> <notification_per_second<=1000=1>\n";
    return -1;
  }

  const int legacy_port (atoi (av[1]));
  const int worker_count (ac >= 3 ? atoi (av[2]) : 1);
  const int duration (ac >= 4 ? 1000 / atoi (av[3]) : 1);

  fhg::logging::stream_emitter emitter;
  fhg::logging::legacy::emitter legacy_emitter ("localhost", legacy_port);

  std::vector<std::string> worker_names (worker_count);
  std::generate (worker_names.begin(), worker_names.end(), worker_gen);

  std::cout << emitter.local_endpoint().to_string() << "\n";

  std::map<std::string, boost::optional<activity>> workers;

  for (;;)
  {
    const std::string worker (worker_names[lrand48() % worker_names.size()]);

    if (!workers[worker])
    {
      workers[worker] = activity (worker);
    }

    workers[worker]->send_out_notification (emitter, legacy_emitter);

    if (!workers[worker]->next_state())
    {
      workers[worker] = boost::none;
    }

    std::this_thread::sleep_for (std::chrono::milliseconds (duration));
  }

  //! \note Wait for remote logger being done.
  //! \todo Wait more cleverly.
  std::this_thread::sleep_for (std::chrono::seconds (2));

  return 0;
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';
  return 1;
}
