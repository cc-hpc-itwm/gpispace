#define BOOST_TEST_MODULE TestWorkerManager

#include <sdpa/daemon/WorkerManager.hpp>
#include <fhg/util/random_string.hpp>
#include <boost/test/unit_test.hpp>

std::vector<std::string> generate_worker_names(const int& n)
{
  std::vector<std::string> workers(n);
  std::transform ( workers.begin()
                 , workers.end()
                 , workers.begin()
                 , [](std::string) { return fhg::util::random_string(); }
                 );
  return workers;
}

BOOST_AUTO_TEST_CASE (sorted_list_of_matching_workers)
{
  std::vector<std::string> workers (generate_worker_names(4));

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
      ( worker_manager.getListMatchingWorkers(job_req, list_workers));

  BOOST_REQUIRE_EQUAL(list_matching_workers.size(), workers.size()-1);
  sdpa::worker_id_list_t::iterator it(list_matching_workers.begin());
  BOOST_REQUIRE_EQUAL(*it++, workers[2]);
  BOOST_REQUIRE_EQUAL(*it++, workers[3]);
  BOOST_REQUIRE_EQUAL(*it++, workers[0]);
}
