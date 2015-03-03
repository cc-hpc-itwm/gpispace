#define BOOST_TEST_MODULE restart_worker_with_dummy_workflow

#include <utils.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_CASE (restart_worker_with_dummy_workflow, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator, _logger);

  utils::client client (orchestrator);
  sdpa::job_id_t const job_id (client.submit_job (utils::module_call()));

  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  {
    fhg::util::thread::event<> job_submitted;

    const utils::fake_drts_worker_notifying_module_call_submission worker
      ( worker_id
      , [&job_submitted] (std::string) { job_submitted.notify(); }
      , agent
      );

    job_submitted.wait();
  }

  const utils::fake_drts_worker_directly_finishing_jobs restarted_worker
    (worker_id, agent);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
