#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/types.hpp>

#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>

#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <functional>
#include <random>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
  const double computational_cost = 1.0;

  std::vector<std::string> generate_worker_names (const int n)
  {
    std::vector<std::string> worker_ids (n);
    std::generate ( worker_ids.begin()
                  , worker_ids.end()
                  , fhg::util::testing::random_string
                  );
    return worker_ids;
  }

  bool random_bool()
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist (0.5);
    return dist (gen);
  }

  unsigned long random_ulong()
  {
    return fhg::util::testing::random_integral<unsigned long>();
  }
}

BOOST_AUTO_TEST_CASE (add_worker)
{
  const std::vector<std::string> worker_ids (generate_worker_names (3));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker ( worker_ids[0]
                           , {sdpa::capability_t ("A", worker_ids[0])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[1]
                           , {sdpa::capability_t ("B", worker_ids[1])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[2]
                           , {sdpa::capability_t ("C", worker_ids[2])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  BOOST_REQUIRE (worker_manager.hasWorker_INDICATES_A_RACE_TESTING_ONLY (worker_ids[0]));
  BOOST_REQUIRE (worker_manager.hasWorker_INDICATES_A_RACE_TESTING_ONLY (worker_ids[1]));
  BOOST_REQUIRE (worker_manager.hasWorker_INDICATES_A_RACE_TESTING_ONLY (worker_ids[2]));
}

BOOST_AUTO_TEST_CASE (delete_worker)
{
  const std::vector<std::string> worker_ids (generate_worker_names (3));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker ( worker_ids[0]
                           , {sdpa::capability_t ("A", worker_ids[0])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[1]
                           , {sdpa::capability_t ("B", worker_ids[1])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.deleteWorker (worker_ids[1]);

  BOOST_REQUIRE (worker_manager.hasWorker_INDICATES_A_RACE_TESTING_ONLY(worker_ids[0]));
  BOOST_REQUIRE (!worker_manager.hasWorker_INDICATES_A_RACE_TESTING_ONLY(worker_ids[1]));
}

BOOST_AUTO_TEST_CASE (get_capabilities)
{
  const std::vector<std::string> worker_ids (generate_worker_names (3));

  const sdpa::capabilities_set_t expected_ccapabilities ({ sdpa::capability_t ("A", worker_ids[0])
                                                         , sdpa::capability_t ("B", worker_ids[1])
                                                         , sdpa::capability_t ("C", worker_ids[2])
                                                        });

  sdpa::capabilities_set_t::iterator it (expected_ccapabilities.begin());

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker ( worker_ids[0]
                           , {*it++}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[1]
                           , {*it++}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[2]
                           , {*it++}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  sdpa::capabilities_set_t acquired_capabilities;
  worker_manager.getCapabilities (acquired_capabilities);

  BOOST_REQUIRE (acquired_capabilities == expected_ccapabilities);
}

BOOST_AUTO_TEST_CASE (find_submitted_or_acknowledged_worker)
{
  const std::vector<std::string> worker_ids (generate_worker_names (1));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker ( worker_ids[0]
                           , {sdpa::capability_t ("A", worker_ids[0])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  const sdpa::job_id_t job_id (fhg::util::testing::random_string());

  worker_manager.assign_job_to_worker (job_id, worker_ids[0], 1, {});
  std::unordered_set<sdpa::worker_id_t> workers (worker_manager.findSubmOrAckWorkers (job_id));
  BOOST_REQUIRE (workers.empty());

  worker_manager.submit_and_serve_if_can_start_job_INDICATES_A_RACE
    ( job_id
    , {worker_ids[0]}
    , boost::none
    , [] ( sdpa::daemon::WorkerSet const&
         , sdpa::daemon::Implementation const&
         , sdpa::job_id_t const&
         )
      {
        // do nothing, serve_job is merged with submit_if_can_start in
        // order to avoid races when workers are removed
      }
    );

  workers = worker_manager.findSubmOrAckWorkers (job_id);
  BOOST_REQUIRE_EQUAL (workers.size(), 1);
  BOOST_REQUIRE (workers.count (worker_ids[0]));

  worker_manager.acknowledge_job_sent_to_worker (job_id, worker_ids[0]);
  workers = worker_manager.findSubmOrAckWorkers (job_id);
  BOOST_REQUIRE_EQUAL (workers.size(), 1);
  BOOST_REQUIRE (workers.count (worker_ids[0]));
}

BOOST_AUTO_TEST_CASE (find_submitted_or_acknowledged_coallocated_workers)
{
  const unsigned int N (10);
  const std::vector<std::string> worker_ids (generate_worker_names (N));

  sdpa::daemon::WorkerManager worker_manager;

  sdpa::daemon::WorkerSet workers;

  for (unsigned int k=0; k<N; k++)
  {
    worker_manager.addWorker ( worker_ids[k]
                             , {sdpa::capability_t ("A", worker_ids[k])}
                             , random_ulong()
                             , random_bool()
                             , fhg::util::testing::random_string()
                             , fhg::util::testing::random_string()
                             );

    workers.emplace ( worker_ids[k]);
  }

  const sdpa::job_id_t job_id (fhg::util::testing::random_string());

  for (unsigned int i=0; i<N; i++)
  {
    worker_manager.assign_job_to_worker (job_id, worker_ids[i], 1.0, {});
    std::unordered_set<sdpa::worker_id_t> submitted_or_ack_workers (worker_manager.findSubmOrAckWorkers (job_id));
    BOOST_REQUIRE (submitted_or_ack_workers.empty());
  }

  worker_manager.submit_and_serve_if_can_start_job_INDICATES_A_RACE
    ( job_id
    , workers
    , boost::none
    , [] ( sdpa::daemon::WorkerSet const&
         , sdpa::daemon::Implementation const&
         , sdpa::job_id_t const&
         )
      {
        // do nothing, serve_job is merged with submit_if_can_start in
        // order to avoid races when workers are removed
      }
    );

  std::unordered_set<sdpa::worker_id_t> submitted_or_ack_workers
    (worker_manager.findSubmOrAckWorkers (job_id));

  BOOST_REQUIRE_EQUAL (submitted_or_ack_workers.size(), N);

  for (unsigned int k=0; k<N; k++)
  {
    BOOST_REQUIRE (submitted_or_ack_workers.count (worker_ids[k]));
  }

  for (unsigned int k=0; k<N; k++)
  {
    worker_manager.acknowledge_job_sent_to_worker (job_id, worker_ids[k]);
    submitted_or_ack_workers = worker_manager.findSubmOrAckWorkers (job_id);
    BOOST_REQUIRE_EQUAL (submitted_or_ack_workers.size(), N);

    for (unsigned int i=0; i<N; i++)
    {
      BOOST_REQUIRE (submitted_or_ack_workers.count (worker_ids[i]));
    }
  }
}

BOOST_AUTO_TEST_CASE (find_non_submitted_job)
{
  sdpa::daemon::WorkerManager worker_manager;

  const sdpa::job_id_t job_not_submitted (fhg::util::testing::random_string());
  std::unordered_set<sdpa::worker_id_t>  workers (worker_manager.findSubmOrAckWorkers (job_not_submitted));
  BOOST_REQUIRE (workers.empty());
}

BOOST_AUTO_TEST_CASE (issue_675_reference_to_popped_queue_element)
{
  // <boilerplate>
  sdpa::daemon::WorkerManager worker_manager;

  std::unordered_map<sdpa::job_id_t, std::unique_ptr<sdpa::daemon::scheduler::Reservation>> reservations;

  std::string const capability_name (fhg::util::testing::random_string());
  auto&& add_worker ( [&] (sdpa::worker_id_t worker_id)
                      {
                        worker_manager.addWorker
                          ( worker_id
                          , {sdpa::capability_t (capability_name, worker_id)}
                          , random_ulong()
                          , random_bool()
                          , fhg::util::testing::random_string()
                          , fhg::util::testing::random_string()
                          );
                      }
                    );

  auto&& add_pending_job
    ( [&] ( sdpa::worker_id_t worker_id, sdpa::job_id_t job_id
          , double cost, bool allowed_to_be_stolen
          )
      {
        reservations.emplace
          ( job_id
          , fhg::util::cxx14::make_unique<sdpa::daemon::scheduler::Reservation>
              ( allowed_to_be_stolen
              ? std::set<sdpa::worker_id_t> {worker_id}
              : std::set<sdpa::worker_id_t>{}
              , sdpa::daemon::Implementation{}
              , cost
              )
          );
        worker_manager.assign_job_to_worker (job_id, worker_id, cost, {});
      }
    );
  auto&& add_running_job
    ( [&] ( sdpa::worker_id_t worker_id, sdpa::job_id_t job_id
          , double cost, bool allowed_to_be_stolen
          )
      {
        add_pending_job (worker_id, job_id, cost, allowed_to_be_stolen);
        BOOST_REQUIRE
          ( worker_manager.submit_and_serve_if_can_start_job_INDICATES_A_RACE
            ( job_id
            , {worker_id}
            , boost::none
            , [] ( sdpa::daemon::WorkerSet const&
                 , sdpa::daemon::Implementation const&
                 , sdpa::job_id_t const&
                 )
              {}
            )
          );
      }
    );

  auto&& steal_work
    ( [&]
      {
        auto const reservation
          ( [&] (sdpa::job_id_t const& job_id)
            {
              return static_cast<sdpa::daemon::scheduler::Reservation*>
                (reservations.at (job_id).get());
            }
          );
        worker_manager.steal_work (reservation);
      }
    );

  fhg::util::testing::unique_random<sdpa::worker_id_t> worker_id_pool;
  fhg::util::testing::unique_random<sdpa::job_id_t> job_id_pool;
  // </boilerplate>


  // A reference to the top of a queue was held, which was then popped
  // but conditionally used after popping again. The condition was
  // that a worker may be stolen from again. This means that it has a
  // running and a pending job, or multiple pending jobs, after
  // stealing once.

  // This can be trivially triggered by having three jobs which are
  // all assigned to the same worker, and two workers without
  // jobs. Both workers will then try to steal from the first one,
  // which results in undefined behaviour as the reference will point
  // to an element in an empty queue when stealing the second time.

  // If now a fourth worker exists which can also be stolen from,
  // instead of pointing into an empty queue, it will be pointing to a
  // different element of the queue, which leads to emplacing a
  // different element of the queue a second time, rather than
  // emplacing the intended worker. As this is easier to detect than
  // an invalid-read, this test case only does this test.


  // 1 running; 2 stealable, cost 1 each
  {
    sdpa::worker_id_t const worker_id (worker_id_pool());
    add_worker (worker_id);
    add_running_job (worker_id, job_id_pool(), 1.0, false);
    add_pending_job (worker_id, job_id_pool(), 1.0, true);
    add_pending_job (worker_id, job_id_pool(), 1.0, true);
  }

  // 1 running; 1 stealable, cost 0 each
  {
    sdpa::worker_id_t const worker_id (worker_id_pool());
    add_worker (worker_id);
    add_running_job (worker_id, job_id_pool(), 0.0, false);
    add_pending_job (worker_id, job_id_pool(), 0.0, false);
  }

  add_worker (worker_id_pool());
  add_worker (worker_id_pool());


  // With the invalid-read, the second job is stolen from the
  // cheaper worker.
  // Note: If this throws a confusing "Asked to replace the non-existent
  // worker", this is **not** because the worker doesn't exist, but
  // because the test failed and we needed a way to let
  // `Reservation::replace_worker()` fail. We do so by letting all
  // those reservations we don't want to steal not have a worker to
  // replace.
  steal_work();
}
