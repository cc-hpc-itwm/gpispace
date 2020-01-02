#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/set.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_maximum_running_time.hpp>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

namespace
{
  unsigned long random_ulong()
  {
    return fhg::util::testing::random_integral<unsigned long>();
  }

  const double computational_cost = 1.0;

  Requirements_and_preferences require (std::string const& capability)
  {
    return { {we::type::requirement_t (capability, true)}
           , we::type::schedule_data()
           , null_transfer_cost
           , computational_cost
           , 0
           , {}
           };
  }
}

struct fixture_add_new_workers
{
  fixture_add_new_workers()
    : _worker_manager()
    , _scheduler
       ( std::bind ( &fixture_add_new_workers::requirements_and_preferences
                   , this
                   , std::placeholders::_1
                   )
       , _worker_manager
       )
  {}

  sdpa::daemon::WorkerManager _worker_manager;
  sdpa::daemon::CoallocationScheduler _scheduler;
  fhg::util::testing::unique_random<sdpa::worker_id_t> _worker_name_pool;

  void add_job ( const sdpa::job_id_t& job_id
               , const Requirements_and_preferences& reqs
               )
  {
    if (!_requirements_and_preferences.emplace (job_id, reqs).second)
    {
      throw std::runtime_error ("added job twice");
    }
  }

  Requirements_and_preferences requirements_and_preferences (sdpa::job_id_t id)
  {
    return _requirements_and_preferences.find (id)->second;
  }

  std::vector<sdpa::worker_id_t> add_new_workers
    ( std::unordered_set<std::string> const& cpbnames
    , unsigned int n
    )
  {
    std::vector<sdpa::worker_id_t> new_workers (n);
    std::generate_n ( new_workers.begin()
                    , n
                    , [this] {return _worker_name_pool();}
                    );

    for (sdpa::worker_id_t const& worker : new_workers)
    {
      sdpa::capabilities_set_t cpbset;
      for (std::string const& capability_name : cpbnames)
      {
        cpbset.emplace (capability_name, worker);
      }

      _worker_manager.addWorker ( worker
                                , cpbset
                                , random_ulong()
                                , false
                                , fhg::util::testing::random_string()
                                , fhg::util::testing::random_string()
                                );
    }

    return new_workers;
  }

  void request_scheduling()
  {
    _scheduler.assignJobsToWorkers();
    _scheduler.steal_work();
  }

private:
  std::map<sdpa::job_id_t, Requirements_and_preferences>
    _requirements_and_preferences;
};

BOOST_FIXTURE_TEST_CASE
  (performance_scheduling_tasks_with_no_preferences, fixture_add_new_workers)
{
  using fhg::util::testing::random_integral;

  fhg::util::testing::unique_random<sdpa::job_id_t> job_name_pool;
  fhg::util::testing::unique_random<std::string> capability_pool;

  std::unordered_set<std::string> const capabilities
    {capability_pool(), capability_pool()};

  unsigned int const num_workers (2000);
  unsigned int const num_tasks (15000);

  std::vector<sdpa::worker_id_t> const workers
    (add_new_workers (capabilities, num_workers));

  fhg::util::testing::require_maximum_running_time<std::chrono::seconds>
    const maxmimum_running_time (60);

  for (unsigned int i {0}; i < num_tasks; i++)
  {
    sdpa::job_id_t const task (job_name_pool());
    add_job
      ( task
      , require (*std::next ( capabilities.begin()
                            , random_integral<unsigned int>() % 2
                            )
                )
      );

    _scheduler.enqueueJob (task);
    request_scheduling();
  }
}
