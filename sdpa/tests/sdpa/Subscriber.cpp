#define BOOST_TEST_MODULE TestSubscriber

#include <utils.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_CASE (execute_workflow_with_subscribed_client, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator);
  const utils::fake_drts_worker_directly_finishing_jobs worker (agent);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (utils::module_call(), orchestrator)
    , sdpa::status::FINISHED
    );
}

BOOST_FIXTURE_TEST_CASE (execute_workflow_and_subscribe_with_second_client, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator);
  const utils::fake_drts_worker_directly_finishing_jobs worker (agent);

  sdpa::job_id_t job_id_user;
  {
    utils::client c (orchestrator);
    job_id_user = c.submit_job (utils::module_call());
  }

  utils::client c (orchestrator);
  BOOST_REQUIRE_EQUAL
    ( c.wait_for_terminal_state_and_cleanup (job_id_user)
    , sdpa::status::FINISHED
    );
}
