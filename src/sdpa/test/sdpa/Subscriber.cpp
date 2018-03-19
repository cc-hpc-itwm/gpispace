#include <utils.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

void test_execute_workflow_with_subscribed_client
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  const utils::orchestrator orchestrator (_logger, certificates);
  const utils::agent agent (orchestrator, _logger, certificates);
  const utils::fake_drts_worker_directly_finishing_jobs worker (agent, certificates);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (utils::module_call(), orchestrator, certificates)
    , sdpa::status::FINISHED
    );
}

BOOST_FIXTURE_TEST_CASE (execute_workflow_with_subscribed_client, setup_logging)
{
  test_execute_workflow_with_subscribed_client (_logger, boost::none);

  if (test_certificates)
  {
    test_execute_workflow_with_subscribed_client (_logger, test_certificates);
  }
}

void test_execute_workflow_and_subscribe_with_second_client
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  const utils::orchestrator orchestrator (_logger, certificates);
  const utils::agent agent (orchestrator, _logger, certificates);
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

BOOST_FIXTURE_TEST_CASE (execute_workflow_and_subscribe_with_second_client, setup_logging)
{
  test_execute_workflow_and_subscribe_with_second_client (_logger, boost::none);

  if (test_certificates)
  {
    test_execute_workflow_and_subscribe_with_second_client (_logger, test_certificates);
  }
}
