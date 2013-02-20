#define BOOST_TEST_MODULE worker_manager_load_balancing

#include <boost/test/unit_test.hpp>

#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/JobId.hpp>

#include <algorithm>

BOOST_AUTO_TEST_CASE (load_balancing_to_nearly_average)
{
  static const size_t NWORKERS = 10;
  size_t worker_load[NWORKERS] = {100, 200, 10, 2, 30, 1000, 5, 300, 55, 701};

  const float average_load
    (std::accumulate (worker_load, worker_load + NWORKERS, 0) / float (NWORKERS));
  const float tolerance (std::ceil (average_load)/ average_load);

  sdpa::daemon::WorkerManager wm;

  std::cout << average_load << ", " << tolerance << std::endl;

  for (size_t k (0); k < NWORKERS; ++k)
  {
    const sdpa::JobId job_id;
    wm.addWorker (job_id.str(), 100, sdpa::capabilities_set_t(), 0, job_id);
  }

  for (size_t k (0); k < NWORKERS; ++k)
  {
    const sdpa::daemon::Worker::ptr_t& worker (wm.getNextWorker());

    for (size_t l (0); l < worker_load[k]; ++l)
    {
      worker->dispatch (sdpa::JobId());
    }
  }

  for (size_t k (0); k < NWORKERS; ++k)
  {
    BOOST_REQUIRE_EQUAL (wm.getNextWorker()->pending().size(), worker_load[k]);
  }

  wm.balanceWorkers();

  for (size_t k (0); k < NWORKERS; ++k)
  {
    BOOST_REQUIRE_CLOSE
      (wm.getNextWorker()->pending().size(), average_load, tolerance);
  }
}
