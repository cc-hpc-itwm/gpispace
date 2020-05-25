#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <test/certificates_data.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

BOOST_DATA_TEST_CASE
  (execute_workflow_with_subscribed_client, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);
  const utils::agent agent (orchestrator, certificates);

  fhg::util::thread::event<std::string> job_submitted;
  utils::fake_drts_worker_waiting_for_finished_ack worker
    ( [&job_submitted] (std::string s) { job_submitted.notify (s); }
    , agent
    , certificates
    );

  utils::client client (orchestrator, certificates);
  auto const job_id (client.submit_job (utils::module_call()));

  worker.finish_and_wait_for_ack (job_submitted.wait());

  BOOST_REQUIRE_EQUAL
    ( client.wait_for_terminal_state_and_cleanup (job_id)
    , sdpa::status::FINISHED
    );
}

BOOST_DATA_TEST_CASE
  ( execute_workflow_and_subscribe_with_second_client
  , certificates_data
  , certificates
  )
{
  const utils::orchestrator orchestrator (certificates);
  const utils::agent agent (orchestrator, certificates);

  fhg::util::thread::event<std::string> job_submitted;
  utils::fake_drts_worker_waiting_for_finished_ack worker
    ( [&job_submitted] (std::string s) { job_submitted.notify (s); }
    , agent
    , certificates
    );

  sdpa::job_id_t job_id_user;
  {
    utils::client c (orchestrator, certificates);
    job_id_user = c.submit_job (utils::module_call());
  }

  worker.finish_and_wait_for_ack (job_submitted.wait());

  utils::client c (orchestrator, certificates);
  BOOST_REQUIRE_EQUAL
    ( c.wait_for_terminal_state_and_cleanup (job_id_user)
    , sdpa::status::FINISHED
    );
}
