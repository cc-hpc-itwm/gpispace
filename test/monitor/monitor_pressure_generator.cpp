// Copyright (C) 2013-2016,2019-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

//! \note This "test" does not test anything, but is a pressure-generator for scheduler-gui only.

#include <gspc/logging/stream_emitter.hpp>

#include <gspc/scheduler/daemon/NotificationEvent.hpp>

#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/Transition.hpp>

#include <gspc/util/print_exception.hpp>
#include <gspc/util/syscall.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/util/unreachable.hpp>

#include <optional>

#include <algorithm>
#include <fmt/core.h>
#include <iostream>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <thread>
#include <vector>

using namespace gspc::scheduler::daemon;

static uint64_t id_counter;

struct activity
{
  activity (std::string const& worker)
    : _id {fmt::format ("{}", ++id_counter)}
    , _workers()
    , _act ( gspc::we::type::Transition ( "activity-" + _id
                                    , gspc::we::type::Expression()
                                    , std::nullopt
                                    , gspc::we::type::property::type()
                                    , gspc::we::priority_type()
                                    , std::optional<gspc::we::type::eureka_id_type>{}
                                    , std::list<gspc::we::type::Preference>{}
                                    , gspc::we::type::track_shared{}
                                    )
           )
  {
    _workers.push_back (worker);
  }

  void send_out_notification (gspc::logging::stream_emitter& emitter) const
  {
    const NotificationEvent event (_workers, _id, _state, _act);
    emitter.emit_message ({event.encoded(), gspc::scheduler::daemon::gantt_log_category});
    static char const* arr[4] = { gspc::logging::legacy::category_level_trace
                                , gspc::logging::legacy::category_level_info
                                , gspc::logging::legacy::category_level_warn
                                , gspc::logging::legacy::category_level_error
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
  gspc::scheduler::daemon::NotificationEvent::state_t _state
    {NotificationEvent::STATE_STARTED};
  gspc::we::type::Activity _act;
};

static
std::string worker_gen()
{
  static long ids[]          = {0,      0,      0,       0,     0,     0};
  static const char* names[] = {"calc", "load", "store", "foo", "bar", "baz"};

  const auto r (gspc::testing::random<std::size_t>{}() % sizeof (names)/sizeof(*names));
  return fmt::format ( "{0}-ip-127-0-0-1 {2} 50501-{1}"
                     , names[r]
                     , ++ids[r]
                     , gspc::util::syscall::getpid()
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

  gspc::logging::stream_emitter emitter;

  std::vector<std::string> worker_names (worker_count);
  std::generate (worker_names.begin(), worker_names.end(), worker_gen);

  std::cout << emitter.local_endpoint().to_string() << "\n";

  std::map<std::string, std::optional<activity>> workers;

  for (;;)
  {
    const std::string worker (worker_names[gspc::testing::random<decltype (worker_names)::size_type>{}() % worker_names.size()]);

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
  std::cerr << "EX: " << gspc::util::current_exception_printer() << '\n';
  return 1;
}
