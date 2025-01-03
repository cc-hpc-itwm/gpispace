// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/daemon/WorkerManager.hpp>

#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <we/type/Requirement.hpp>
#include <we/type/schedule_data.hpp>

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
  std::vector<std::string> generate_worker_names (unsigned int n)
  {
    return fhg::util::testing::randoms<std::vector<std::string>>
      (n, &utils::random_peer_name);
  }
}

BOOST_AUTO_TEST_CASE (add_worker)
{
  const std::vector<std::string> worker_ids (generate_worker_names (3));

  sdpa::daemon::WorkerManager worker_manager;

  fhg::util::testing::random<std::string> const random_string;
  fhg::util::testing::random<unsigned short> const random_ushort;

  auto&& add_worker
    ( [&] ( sdpa::worker_id_t const& worker_id
          , std::string const& capability_name
          )
    {
      auto const hostname {random_string()};

        worker_manager.add_worker
          ( worker_id
          , {sdpa::Capability (capability_name)}
          , fhg::util::testing::random<unsigned long>{}()
          , hostname
          , fhg::com::p2p::address_t
              { fhg::com::host_t {hostname}
              , fhg::com::port_t {random_ushort()}
              }
          );
      }
    );

  for (auto const& worker : worker_ids)
  {
    BOOST_REQUIRE_NO_THROW
      (add_worker (worker, fhg::util::testing::random_string()));
  }

  for (auto const& worker : worker_ids)
  { //attempting to add a worker with an existing id will throw
    BOOST_REQUIRE_EXCEPTION
      ( add_worker (worker, fhg::util::testing::random_string())
      , std::runtime_error
      , [&worker] (std::runtime_error const& exc)
        {
          std::string expected_err ("worker '" + worker + "' already exists");
          return expected_err == std::string (exc.what(), expected_err.size());
        }
      );
  }
}

BOOST_AUTO_TEST_CASE (delete_worker)
{
  const std::vector<std::string> worker_ids (generate_worker_names (2));

  sdpa::daemon::WorkerManager worker_manager;

  fhg::util::testing::random<std::string> const random_string;
  fhg::util::testing::random<unsigned short> const random_ushort;

  auto&& add_worker
    ( [&] ( sdpa::worker_id_t const& worker_id
          , std::string const& capability_name
          )
      {
        auto const hostname {random_string()};

        worker_manager.add_worker
          ( worker_id
          , {sdpa::Capability (capability_name)}
          , fhg::util::testing::random<unsigned long>{}()
          , hostname
          , fhg::com::p2p::address_t
              { fhg::com::host_t {hostname}
              , fhg::com::port_t {random_ushort()}
              }
          );
      }
    );

  add_worker (worker_ids[0], fhg::util::testing::random_string());
  add_worker (worker_ids[1], fhg::util::testing::random_string());

  worker_manager.delete_worker (worker_ids[1]);

  //attempting to add a worker with an existing id will throw
  BOOST_REQUIRE_EXCEPTION
    ( add_worker (worker_ids[0], fhg::util::testing::random_string())
    , std::runtime_error
    , [&worker_ids] (std::runtime_error const& exc)
      {
        std::string expected_err
          ("worker '" + worker_ids[0] + "' already exists");
        return expected_err == std::string (exc.what(), expected_err.size());
      }
    );

  //after deletion of a worker, its worker id can be reused
  BOOST_REQUIRE_NO_THROW
    (add_worker (worker_ids[1], fhg::util::testing::random_string()));
}

BOOST_AUTO_TEST_CASE (find_submitted_or_acknowledged_worker)
{
  const std::vector<std::string> worker_ids (generate_worker_names (1));

  sdpa::daemon::WorkerManager worker_manager;
  fhg::util::testing::random<std::string> const random_string;
  fhg::util::testing::random<unsigned short> const random_ushort;
  auto const hostname {random_string()};
  worker_manager.add_worker ( worker_ids[0]
                            , {sdpa::Capability ("A")}
                            , fhg::util::testing::random<unsigned long>{}()
                            , hostname
                            , fhg::com::p2p::address_t
                                { fhg::com::host_t {hostname}
                                , fhg::com::port_t {random_ushort()}
                                }
                            );

  const sdpa::job_id_t job_id (fhg::util::testing::random_string());

  worker_manager.assign_job_to_worker (job_id, worker_ids[0], 1, {});
  std::unordered_set<sdpa::worker_id_t> workers
    (worker_manager.find_subm_or_ack_workers (job_id, {worker_ids[0]}));
  BOOST_REQUIRE (workers.empty());

  worker_manager.submit_job_to_worker (job_id, worker_ids[0]);

  workers = worker_manager.find_subm_or_ack_workers (job_id, {worker_ids[0]});
  BOOST_REQUIRE_EQUAL (workers.size(), 1);
  BOOST_REQUIRE (workers.count (worker_ids[0]));

  worker_manager.acknowledge_job_sent_to_worker (job_id, worker_ids[0]);
  workers = worker_manager.find_subm_or_ack_workers (job_id, {worker_ids[0]});
  BOOST_REQUIRE_EQUAL (workers.size(), 1);
  BOOST_REQUIRE (workers.count (worker_ids[0]));
}

BOOST_AUTO_TEST_CASE (find_submitted_or_acknowledged_coallocated_workers)
{
  const unsigned int N (10);
  const std::vector<std::string> worker_ids (generate_worker_names (N));

  sdpa::daemon::WorkerManager worker_manager;

  sdpa::daemon::WorkerSet workers;

  fhg::util::testing::random<std::string> const random_string;
  fhg::util::testing::random<unsigned short> const random_ushort;

  for (unsigned int k=0; k<N; k++)
  {
    auto const hostname {random_string()};
    worker_manager.add_worker ( worker_ids[k]
                              , {sdpa::Capability ("A")}
                              , fhg::util::testing::random<unsigned long>{}()
                              , hostname
                              , fhg::com::p2p::address_t
                                  { fhg::com::host_t {hostname}
                                  , fhg::com::port_t {random_ushort()}
                                  }
                              );

    workers.emplace ( worker_ids[k]);
  }

  const sdpa::job_id_t job_id (fhg::util::testing::random_string());

  for (unsigned int i=0; i<N; i++)
  {
    worker_manager.assign_job_to_worker (job_id, worker_ids[i], 1.0, {});
    std::unordered_set<sdpa::worker_id_t> submitted_or_ack_workers
      ( worker_manager.find_subm_or_ack_workers
         ( job_id
         , std::set<sdpa::worker_id_t> (worker_ids.begin(), worker_ids.end())
         )
      );
    BOOST_REQUIRE (submitted_or_ack_workers.empty());
  }

  for (auto const& worker : workers)
  {
    worker_manager.submit_job_to_worker (job_id, worker);
  }

  std::unordered_set<sdpa::worker_id_t> submitted_or_ack_workers
    ( worker_manager.find_subm_or_ack_workers
        ( job_id
        , std::set<sdpa::worker_id_t> (worker_ids.begin(), worker_ids.end())
        )
    );

  BOOST_REQUIRE_EQUAL (submitted_or_ack_workers.size(), N);

  for (unsigned int k=0; k<N; k++)
  {
    BOOST_REQUIRE (submitted_or_ack_workers.count (worker_ids[k]));
  }

  for (unsigned int k=0; k<N; k++)
  {
    worker_manager.acknowledge_job_sent_to_worker (job_id, worker_ids[k]);
    submitted_or_ack_workers = worker_manager.find_subm_or_ack_workers
      (job_id, std::set<sdpa::worker_id_t> (worker_ids.begin(), worker_ids.end()));
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
  std::unordered_set<sdpa::worker_id_t>
    workers (worker_manager.find_subm_or_ack_workers (job_not_submitted, {}));
  BOOST_REQUIRE (workers.empty());
}

BOOST_AUTO_TEST_CASE (issue_675_reference_to_popped_queue_element)
{
  // <boilerplate>
  sdpa::daemon::WorkerManager worker_manager;

  std::unordered_map<sdpa::job_id_t, std::unique_ptr<sdpa::daemon::scheduler::Reservation>> reservations;

  fhg::util::testing::random<std::string> const random_string;
  fhg::util::testing::random<unsigned short> const random_ushort;

  std::string const capability_name (fhg::util::testing::random_string());
  auto&& add_worker ( [&] (sdpa::worker_id_t worker_id)
                      {
                        auto const hostname {random_string()};

                        worker_manager.add_worker
                          ( worker_id
                          , {sdpa::Capability (capability_name)}
                          , fhg::util::testing::random<unsigned long>{}()
                          , hostname
                          , fhg::com::p2p::address_t
                              { fhg::com::host_t {hostname}
                              , fhg::com::port_t {random_ushort()}
                              }
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
          , std::make_unique<sdpa::daemon::scheduler::Reservation>
              ( allowed_to_be_stolen
              ? std::set<sdpa::worker_id_t> {worker_id}
              : std::set<sdpa::worker_id_t>{}
              , sdpa::daemon::Implementation{}
              , Preferences{}
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
        worker_manager.submit_job_to_worker (job_id, worker_id);
        BOOST_REQUIRE
          (worker_manager.find_subm_or_ack_workers (job_id, {worker_id}).count (worker_id));
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
        worker_manager.steal_work (sdpa::daemon::CostModel{}, reservation);
      }
    );

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
    sdpa::worker_id_t const worker_id (utils::random_peer_name());
    add_worker (worker_id);
    add_running_job (worker_id, job_id_pool(), 1.0, false);
    add_pending_job (worker_id, job_id_pool(), 1.0, true);
    add_pending_job (worker_id, job_id_pool(), 1.0, true);
  }

  // 1 running; 1 stealable, cost 0 each
  {
    sdpa::worker_id_t const worker_id (utils::random_peer_name());
    add_worker (worker_id);
    add_running_job (worker_id, job_id_pool(), 0.0, false);
    add_pending_job (worker_id, job_id_pool(), 0.0, false);
  }

  add_worker (utils::random_peer_name());
  add_worker (utils::random_peer_name());


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
