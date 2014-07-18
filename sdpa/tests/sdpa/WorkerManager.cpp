#define BOOST_TEST_MODULE TestWorkerManager

#include <sdpa/daemon/WorkerManager.hpp>
#include <fhg/util/random_string.hpp>
#include <boost/test/unit_test.hpp>

std::vector<std::string> generate_worker_names (const int n)
{
  std::vector<std::string> workers (n);
  std::generate ( workers.begin()
                , workers.end()
                , fhg::util::random_string
                );
  return workers;
}

BOOST_AUTO_TEST_CASE (sorted_list_of_matching_workers)
{
  std::vector<std::string> workers (generate_worker_names (4));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker (workers[0], 1, {sdpa::capability_t ("A", workers[0])});
  worker_manager.addWorker (workers[1], 1, {sdpa::capability_t ("B", workers[1])});
  worker_manager.addWorker (workers[2], 1, { sdpa::capability_t ("A", workers[2])
                                           , sdpa::capability_t ("B", workers[2])
                                           , sdpa::capability_t ("C", workers[2])
                                           }
                           );

  worker_manager.addWorker (workers[3], 1, { sdpa::capability_t ("A", workers[3])
                                           , sdpa::capability_t ("B", workers[3])
                                           }
                           );

  sdpa::worker_id_list_t list_workers (worker_manager.getListWorkersNotReserved());

  job_requirements_t job_req ({{ we::type::requirement_t ("A", true)
                               , we::type::requirement_t ("B", false)
                               , we::type::requirement_t ("C", false)
                               }
                              , we::type::schedule_data()
                              }
                             );

  sdpa::worker_id_list_t
    list_matching_workers
      (worker_manager.getListMatchingWorkers (job_req, list_workers));

  BOOST_REQUIRE_EQUAL (list_matching_workers.size(), workers.size()-1);
  sdpa::worker_id_list_t::iterator it (list_matching_workers.begin());
  BOOST_REQUIRE_EQUAL (*it++, workers[2]);
  BOOST_REQUIRE_EQUAL (*it++, workers[3]);
  BOOST_REQUIRE_EQUAL (*it++, workers[0]);
}

BOOST_AUTO_TEST_CASE (add_worker)
{
  std::vector<std::string> workers (generate_worker_names (3));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker (workers[0], 1, {sdpa::capability_t ("A", workers[0])});
  worker_manager.addWorker (workers[1], 1, {sdpa::capability_t ("B", workers[1])});
  worker_manager.addWorker (workers[2], 1, {sdpa::capability_t ("C", workers[2])});

  BOOST_REQUIRE (worker_manager.hasWorker (workers[0]));
  BOOST_REQUIRE (worker_manager.hasWorker (workers[1]));
  BOOST_REQUIRE (worker_manager.hasWorker (workers[2]));
}

BOOST_AUTO_TEST_CASE (delete_worker)
{
  std::vector<std::string> workers (generate_worker_names (3));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker (workers[0], 1, {sdpa::capability_t ("A", workers[0])});
  worker_manager.addWorker (workers[1], 1, {sdpa::capability_t ("B", workers[1])});

  worker_manager.deleteWorker (workers[1]);

  BOOST_REQUIRE (worker_manager.hasWorker(workers[0]));
  BOOST_REQUIRE (!worker_manager.hasWorker(workers[1]));

  BOOST_REQUIRE_THROW (worker_manager.deleteWorker (workers[2]), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (get_capabilities)
{
  std::vector<std::string> workers (generate_worker_names (3));

  sdpa::capabilities_set_t expected_ccapabilities ({ sdpa::capability_t ("A", workers[0])
                                                   , sdpa::capability_t ("B", workers[1])
                                                   , sdpa::capability_t ("C", workers[2])
                                                   });

  sdpa::capabilities_set_t::iterator it (expected_ccapabilities.begin());

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker (workers[0], 1, {*it++});
  worker_manager.addWorker (workers[1], 1, {*it++});
  worker_manager.addWorker (workers[2], 1, {*it++});

  sdpa::capabilities_set_t acquired_capabilities;
  worker_manager.getCapabilities (acquired_capabilities);

  BOOST_REQUIRE (acquired_capabilities == expected_ccapabilities);
}

BOOST_AUTO_TEST_CASE (find_submitted_or_acknowledged_worker)
{
  std::vector<std::string> workers (generate_worker_names (1));

  sdpa::daemon::WorkerManager worker_manager;
  worker_manager.addWorker (workers[0], 1, {sdpa::capability_t ("A", workers[0])});

  sdpa::daemon::Worker::ptr_t ptrWorker (worker_manager.findWorker (workers[0]));
  const sdpa::job_id_t job_id (fhg::util::random_string());
  ptrWorker->submit (job_id);

  boost::optional<sdpa::worker_id_t> worker_id (worker_manager.findSubmOrAckWorker (job_id));
  BOOST_REQUIRE (worker_id);
  BOOST_REQUIRE_EQUAL (*worker_id, workers[0]);

  ptrWorker->acknowledge (job_id);
  worker_id = worker_manager.findSubmOrAckWorker (job_id);
  BOOST_REQUIRE (worker_id);
  BOOST_REQUIRE_EQUAL (*worker_id, workers[0]);
}
