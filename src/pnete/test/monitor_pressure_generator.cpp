// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

//! \note This "test" does not test anything, but is a pressure-generator for sdpa-gui only.

#include <logging/stream_emitter.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <we/type/Activity.hpp>
#include <we/type/Transition.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/unreachable.hpp>

#include <boost/optional.hpp>

#include <algorithm>
#include <fmt/core.h>
#include <iostream>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <thread>
#include <vector>

using namespace sdpa::daemon;

static uint64_t id_counter;

struct activity
{
  activity (std::string const& worker)
    : _id {fmt::format ("{}" , ++id_counter)}
    , _workers()
    , _act ( we::type::Transition ( "activity-" + _id
                                    , we::type::Expression()
                                    , ::boost::none
                                    , we::type::property::type()
                                    , we::priority_type()
                                    , ::boost::optional<we::type::eureka_id_type>{}
                                    , std::list<we::type::Preference>{}
                                    )
           )
  {
    _workers.push_back (worker);
  }

  void send_out_notification (fhg::logging::stream_emitter& emitter) const
  {
    const NotificationEvent event (_workers, _id, _state, _act);
    emitter.emit_message ({event.encoded(), sdpa::daemon::gantt_log_category});
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
  sdpa::daemon::NotificationEvent::state_t _state
    {NotificationEvent::STATE_STARTED};
  we::type::Activity _act;
};

static
std::string worker_gen()
{
  static long ids[]          = {0,      0,      0,       0,     0,     0};
  static const char* names[] = {"calc", "load", "store", "foo", "bar", "baz"};

  const auto r (fhg::util::testing::random<std::size_t>{}() % sizeof (names)/sizeof(*names));
  return fmt::format ( "{0}-ip-127-0-0-1 {2} 50501-{1}"
                     , names[r]
                     , ++ids[r]
                     , fhg::util::syscall::getpid()
                     );
}

int main (int ac, char **av)
try
{
  if (ac > 1 && std::string (av[1]) == "help")
  {
    std::cerr << av[0]
              << " <worker_count=1> <notification_per_second<=1000=1>\n";
    return -1;
  }

  const auto worker_count (ac >= 2 ? std::stoul (av[1]) : 1);
  const int duration (ac >= 3 ? 1000 / atoi (av[2]) : 1);

  fhg::logging::stream_emitter emitter;

  std::vector<std::string> worker_names (worker_count);
  std::generate (worker_names.begin(), worker_names.end(), worker_gen);

  std::cout << emitter.local_endpoint().to_string() << "\n";

  std::map<std::string, std::optional<activity>> workers;

  for (;;)
  {
    const std::string worker (worker_names[fhg::util::testing::random<decltype (worker_names)::size_type>{}() % worker_names.size()]);

    if (!workers[worker])
    {
      workers[worker] = activity (worker);
    }

    workers[worker]->send_out_notification (emitter);

    if (!workers[worker]->next_state())
    {
      workers[worker].reset();
    }

    std::this_thread::sleep_for (std::chrono::milliseconds (duration));
  }

  FHG_UTIL_UNREACHABLE();
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';
  return 1;
}
