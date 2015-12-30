#define BOOST_TEST_MODULE TestTopologies

#include <utils.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_CASE (orchestrator_agent_worker, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);

  const utils::agent agent (orchestrator, _logger);

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_FIXTURE_TEST_CASE (chained_agents, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);

  //! \note "variable agents #" was hardcoded to 1 when this test got
  //! rewritten. Probably should be more, so got bumped to 2.
  const utils::agent agent_0 (orchestrator, _logger);
  const utils::agent agent_1 (agent_0, _logger);

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent_1);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_FIXTURE_TEST_CASE (two_workers_with_seperate_master_agent, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);

  const utils::agent agent_0 (orchestrator, _logger);
  const utils::agent agent_1 (agent_0, _logger);
  const utils::agent agent_2 (agent_0, _logger);

  const utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent_1);
  const utils::fake_drts_worker_directly_finishing_jobs worker_1 (agent_2);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_FIXTURE_TEST_CASE (agent_with_multiple_master_agents, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);

  const utils::agent agent_0 (orchestrator, _logger);
  const utils::agent agent_1 (orchestrator, _logger);
  const utils::agent agent_2 (agent_0, agent_1, _logger);

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent_2);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}
