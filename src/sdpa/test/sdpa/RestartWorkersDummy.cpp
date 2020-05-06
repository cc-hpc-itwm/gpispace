#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <test/certificates_data.hpp>

#include <fhg/util/thread/event.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <string>

BOOST_DATA_TEST_CASE
  (restart_worker_with_dummy_workflow, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);
  const utils::agent agent (orchestrator, certificates);

  utils::client client (orchestrator, certificates);
  sdpa::job_id_t const job_id (client.submit_job (utils::module_call()));

  sdpa::worker_id_t worker_id;

  {
    fhg::util::thread::event<> job_submitted;

    const utils::fake_drts_worker_notifying_module_call_submission worker
      ( [&job_submitted] (std::string) { job_submitted.notify(); }
      , agent
      , certificates
      );

    worker_id = worker.name();

    job_submitted.wait();
  }

  fhg::util::thread::event<std::string> job_submitted_to_restarted_worker;
  utils::fake_drts_worker_notifying_module_call_submission restarted_worker
    ( utils::reused_component_name (worker_id)
    , [&job_submitted_to_restarted_worker] (std::string s)
      { job_submitted_to_restarted_worker.notify (s); }
    , agent
    , certificates
    );

  restarted_worker.finish
    (job_submitted_to_restarted_worker.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
