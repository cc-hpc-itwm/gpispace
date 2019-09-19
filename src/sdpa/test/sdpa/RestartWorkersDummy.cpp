#include <utils.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

BOOST_DATA_TEST_CASE
  (restart_worker_with_dummy_workflow, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);
  const utils::agent agent (orchestrator, certificates);

  utils::client client (orchestrator, certificates);
  sdpa::job_id_t const job_id (client.submit_job (utils::module_call()));

  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  {
    fhg::util::thread::event<> job_submitted;

    const utils::fake_drts_worker_notifying_module_call_submission worker
      ( worker_id
      , [&job_submitted] (std::string) { job_submitted.notify(); }
      , agent
      , certificates
      );

    job_submitted.wait();
  }

  const utils::fake_drts_worker_directly_finishing_jobs restarted_worker
    (worker_id, agent, certificates);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
