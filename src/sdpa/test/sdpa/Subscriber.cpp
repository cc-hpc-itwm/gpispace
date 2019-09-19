#include <utils.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

BOOST_DATA_TEST_CASE
  (execute_workflow_with_subscribed_client, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);
  const utils::agent agent (orchestrator, certificates);
  const utils::fake_drts_worker_directly_finishing_jobs worker (agent, certificates);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (utils::module_call(), orchestrator, certificates)
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
  const utils::fake_drts_worker_directly_finishing_jobs worker (agent, certificates);

  sdpa::job_id_t job_id_user;
  {
    utils::client c (orchestrator, certificates);
    job_id_user = c.submit_job (utils::module_call());
  }

  utils::client c (orchestrator, certificates);
  BOOST_REQUIRE_EQUAL
    ( c.wait_for_terminal_state_and_cleanup (job_id_user)
    , sdpa::status::FINISHED
    );
}
