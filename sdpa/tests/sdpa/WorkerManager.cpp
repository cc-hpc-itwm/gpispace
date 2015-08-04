#define BOOST_TEST_MODULE TestWorkerManager

#include <sdpa/daemon/WorkerManager.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random_string.hpp>
#include <util-generic/testing/random_integral.hpp>

#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

#include <random>

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

BOOST_AUTO_TEST_CASE (sorted_list_of_matching_workers)
{
  const std::vector<std::string> worker_ids (generate_worker_names (4));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker ( worker_ids[0]
                           , 1
                           , {sdpa::capability_t ("A", worker_ids[0])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[1]
                           , 1
                           , {sdpa::capability_t ("B", worker_ids[1])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );
  worker_manager.addWorker ( worker_ids[2]
                           , 1
                           , { sdpa::capability_t ("A", worker_ids[2])
                             , sdpa::capability_t ("B", worker_ids[2])
                             , sdpa::capability_t ("C", worker_ids[2])
                             }
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[3]
                           , 1
                           , { sdpa::capability_t ("A", worker_ids[3])
                             , sdpa::capability_t ("B", worker_ids[3])
                             }
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  std::set<sdpa::worker_id_t> set_workers (worker_manager.getAllNonReservedWorkers());

  const job_requirements_t job_req ({{ we::type::requirement_t ("A", true)
                                     , we::type::requirement_t ("B", false)
                                     , we::type::requirement_t ("C", false)
                                     }
                                    , we::type::schedule_data()
                                    , null_transfer_cost
                                    , computational_cost
                                    , 0
                                    }
                                   );

  sdpa::mmap_match_deg_worker_id_t
    mmap_match_deg_worker_id
      (worker_manager.getMatchingDegreesAndWorkers (job_req));

  BOOST_REQUIRE_EQUAL (mmap_match_deg_worker_id.size(), worker_ids.size()-1);
  sdpa::mmap_match_deg_worker_id_t::iterator it (mmap_match_deg_worker_id.begin());
  BOOST_REQUIRE_EQUAL (it++->second.worker_id(), worker_ids[2]);
  BOOST_REQUIRE_EQUAL (it++->second.worker_id(), worker_ids[3]);
  BOOST_REQUIRE_EQUAL (it++->second.worker_id(), worker_ids[0]);
}

BOOST_AUTO_TEST_CASE (add_worker)
{
  const std::vector<std::string> worker_ids (generate_worker_names (3));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker ( worker_ids[0]
                           , 1
                           , {sdpa::capability_t ("A", worker_ids[0])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[1]
                           , 1
                           , {sdpa::capability_t ("B", worker_ids[1])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[2]
                           , 1
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
                           , 1
                           , {sdpa::capability_t ("A", worker_ids[0])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[1]
                           , 1
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
                           , 1
                           , {*it++}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[1]
                           , 1
                           , {*it++}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  worker_manager.addWorker ( worker_ids[2]
                           , 1
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
                           , 1
                           , {sdpa::capability_t ("A", worker_ids[0])}
                           , random_ulong()
                           , random_bool()
                           , fhg::util::testing::random_string()
                           , fhg::util::testing::random_string()
                           );

  const sdpa::job_id_t job_id (fhg::util::testing::random_string());

  worker_manager.assign_job_to_worker (job_id, worker_ids[0]);
  boost::optional<sdpa::worker_id_t> worker_id (worker_manager.findSubmOrAckWorker (job_id));
  BOOST_REQUIRE (worker_id);
  BOOST_REQUIRE_EQUAL (*worker_id, worker_ids[0]);

  worker_manager.submit_and_serve_if_can_start_job_INDICATES_A_RACE
    ( job_id
    , {worker_ids[0]}
    , [] (std::list<sdpa::worker_id_t> const&, sdpa::job_id_t const&)
      {
        // do nothing, serve_job is merged with submit_if_can_start in
        // order to avoid races when workers are removed
      }
    );
  worker_id = worker_manager.findSubmOrAckWorker (job_id);
  BOOST_REQUIRE (worker_id);
  BOOST_REQUIRE_EQUAL (*worker_id, worker_ids[0]);

  worker_manager.acknowledge_job_sent_to_worker (job_id, worker_ids[0]);
  worker_id = worker_manager.findSubmOrAckWorker (job_id);
  BOOST_REQUIRE (worker_id);
  BOOST_REQUIRE_EQUAL (*worker_id, worker_ids[0]);
}

BOOST_AUTO_TEST_CASE (find_non_submitted_job)
{
  sdpa::daemon::WorkerManager worker_manager;

  const sdpa::job_id_t job_not_submitted (fhg::util::testing::random_string());
  boost::optional<sdpa::worker_id_t>  worker_id (worker_manager.findSubmOrAckWorker (job_not_submitted));
  BOOST_REQUIRE_EQUAL (worker_id, boost::none);
}
