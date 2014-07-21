#define BOOST_TEST_MODULE TestWorkerManager

#include <sdpa/daemon/WorkerManager.hpp>
#include <fhg/util/random_string.hpp>
#include <boost/test/unit_test.hpp>

std::vector<std::string> generate_worker_names (const int n)
{
  std::vector<std::string> worker_ids (n);
  std::generate ( worker_ids.begin()
                , worker_ids.end()
                , fhg::util::random_string
                );
  return worker_ids;
}

BOOST_AUTO_TEST_CASE (sorted_list_of_matching_workers)
{
  const std::vector<std::string> worker_ids (generate_worker_names (4));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker (worker_ids[0], 1, {sdpa::capability_t ("A", worker_ids[0])});
  worker_manager.addWorker (worker_ids[1], 1, {sdpa::capability_t ("B", worker_ids[1])});
  worker_manager.addWorker (worker_ids[2], 1, { sdpa::capability_t ("A", worker_ids[2])
                                              , sdpa::capability_t ("B", worker_ids[2])
                                              , sdpa::capability_t ("C", worker_ids[2])
                                              }
                           );

  worker_manager.addWorker (worker_ids[3], 1, { sdpa::capability_t ("A", worker_ids[3])
                                              , sdpa::capability_t ("B", worker_ids[3])
                                              }
                           );

  sdpa::worker_id_list_t list_workers (worker_manager.getListWorkersNotReserved());

  const job_requirements_t job_req ({{ we::type::requirement_t ("A", true)
                                     , we::type::requirement_t ("B", false)
                                     , we::type::requirement_t ("C", false)
                                     }
                                    , we::type::schedule_data()
                                    }
                                   );

  sdpa::mmap_match_deg_worker_id_t
    mmap_match_deg_worker_id
      (worker_manager.getListMatchingWorkers (job_req, list_workers));

  BOOST_REQUIRE_EQUAL (mmap_match_deg_worker_id.size(), worker_ids.size()-1);
  sdpa::mmap_match_deg_worker_id_t::iterator it (mmap_match_deg_worker_id.begin());
  BOOST_REQUIRE_EQUAL (it++->second, worker_ids[2]);
  BOOST_REQUIRE_EQUAL (it++->second, worker_ids[3]);
  BOOST_REQUIRE_EQUAL (it++->second, worker_ids[0]);
}

BOOST_AUTO_TEST_CASE (add_worker)
{
  const std::vector<std::string> worker_ids (generate_worker_names (3));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker (worker_ids[0], 1, {sdpa::capability_t ("A", worker_ids[0])});
  worker_manager.addWorker (worker_ids[1], 1, {sdpa::capability_t ("B", worker_ids[1])});
  worker_manager.addWorker (worker_ids[2], 1, {sdpa::capability_t ("C", worker_ids[2])});

  BOOST_REQUIRE (worker_manager.hasWorker (worker_ids[0]));
  BOOST_REQUIRE (worker_manager.hasWorker (worker_ids[1]));
  BOOST_REQUIRE (worker_manager.hasWorker (worker_ids[2]));
}

BOOST_AUTO_TEST_CASE (delete_worker)
{
  const std::vector<std::string> worker_ids (generate_worker_names (3));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker (worker_ids[0], 1, {sdpa::capability_t ("A", worker_ids[0])});
  worker_manager.addWorker (worker_ids[1], 1, {sdpa::capability_t ("B", worker_ids[1])});

  worker_manager.deleteWorker (worker_ids[1]);

  BOOST_REQUIRE (worker_manager.hasWorker(worker_ids[0]));
  BOOST_REQUIRE (!worker_manager.hasWorker(worker_ids[1]));

  BOOST_REQUIRE_THROW (worker_manager.deleteWorker (worker_ids[2]), std::runtime_error);
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
  worker_manager.addWorker (worker_ids[0], 1, {*it++});
  worker_manager.addWorker (worker_ids[1], 1, {*it++});
  worker_manager.addWorker (worker_ids[2], 1, {*it++});

  sdpa::capabilities_set_t acquired_capabilities;
  worker_manager.getCapabilities (acquired_capabilities);

  BOOST_REQUIRE (acquired_capabilities == expected_ccapabilities);
}

BOOST_AUTO_TEST_CASE (find_submitted_or_acknowledged_worker)
{
  const std::vector<std::string> worker_ids (generate_worker_names (1));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker (worker_ids[0], 1, {sdpa::capability_t ("A", worker_ids[0])});

  const sdpa::daemon::Worker::ptr_t ptrWorker (worker_manager.findWorker (worker_ids[0]));
  const sdpa::job_id_t job_id (fhg::util::random_string());
  ptrWorker->submit (job_id);

  boost::optional<sdpa::worker_id_t> worker_id (worker_manager.findSubmOrAckWorker (job_id));
  BOOST_REQUIRE (worker_id);
  BOOST_REQUIRE_EQUAL (*worker_id, worker_ids[0]);

  ptrWorker->acknowledge (job_id);
  worker_id = worker_manager.findSubmOrAckWorker (job_id);
  BOOST_REQUIRE (worker_id);
  BOOST_REQUIRE_EQUAL (*worker_id, worker_ids[0]);

  const sdpa::job_id_t job_not_submitted (fhg::util::random_string());
  worker_id = worker_manager.findSubmOrAckWorker (job_not_submitted);
  BOOST_REQUIRE_EQUAL (worker_id, boost::none);
}
