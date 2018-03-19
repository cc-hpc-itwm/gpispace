#include <utils.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

void test_orchestrator_agent_worker
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  const utils::orchestrator orchestrator (_logger, certificates);

  const utils::agent agent (orchestrator, _logger, certificates);

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent, certificates);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator, certificates)
                      , sdpa::status::FINISHED
                      );
}

BOOST_FIXTURE_TEST_CASE (orchestrator_agent_worker, setup_logging)
{
  test_orchestrator_agent_worker (_logger, boost::none);

  if (test_certificates)
  {
    test_orchestrator_agent_worker (_logger, test_certificates);
  }
}

void test_chained_agents
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  const utils::orchestrator orchestrator (_logger, certificates);

  //! \note "variable agents #" was hardcoded to 1 when this test got
  //! rewritten. Probably should be more, so got bumped to 2.
  const utils::agent agent_0 (orchestrator, _logger, certificates);
  const utils::agent agent_1 (agent_0, _logger, certificates);

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent_1, certificates);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator, certificates)
                      , sdpa::status::FINISHED
                      );
}

BOOST_FIXTURE_TEST_CASE (chained_agents, setup_logging)
{
  test_chained_agents (_logger, boost::none);

  if (test_certificates)
  {
    test_chained_agents (_logger, test_certificates);
  }
}

void test_two_workers_with_seperate_master_agent
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  const utils::orchestrator orchestrator (_logger, certificates);

  const utils::agent agent_0 (orchestrator, _logger, certificates);
  const utils::agent agent_1 (agent_0, _logger, certificates);
  const utils::agent agent_2 (agent_0, _logger, certificates);

  const utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent_1, certificates);
  const utils::fake_drts_worker_directly_finishing_jobs worker_1 (agent_2, certificates);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator, certificates)
                      , sdpa::status::FINISHED
                      );
}

BOOST_FIXTURE_TEST_CASE (two_workers_with_seperate_master_agent, setup_logging)
{
  test_two_workers_with_seperate_master_agent (_logger, boost::none);

  if (test_certificates)
  {
    test_two_workers_with_seperate_master_agent (_logger, test_certificates);
  }
}

void test_agent_with_multiple_master_agents
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  const utils::orchestrator orchestrator (_logger, certificates);

  const utils::agent agent_0 (orchestrator, _logger, certificates);
  const utils::agent agent_1 (orchestrator, _logger, certificates);
  const utils::agent agent_2 (agent_0, agent_1, _logger, certificates);

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent_2, certificates);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator, certificates)
                      , sdpa::status::FINISHED
                      );
}

BOOST_FIXTURE_TEST_CASE (agent_with_multiple_master_agents, setup_logging)
{
  test_agent_with_multiple_master_agents (_logger, boost::none);

  if (test_certificates)
  {
    test_agent_with_multiple_master_agents (_logger, test_certificates);
  }
}
