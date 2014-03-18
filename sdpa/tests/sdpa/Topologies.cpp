#define BOOST_TEST_MODULE TestTopologies

#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (setup_logging)

BOOST_AUTO_TEST_CASE (orchestrator_agent_worker)
{
  const utils::kvs_server kvs_server;
  const utils::orchestrator orchestrator (kvs_server);

  const utils::agent agent (orchestrator);

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_AUTO_TEST_CASE (chained_agents)
{
  const utils::kvs_server kvs_server;
  const utils::orchestrator orchestrator (kvs_server);

  //! \note "variable agents #" was hardcoded to 1 when this test got
  //! rewritten. Probably should be more, so got bumped to 2.
  const utils::agent agent_0 (orchestrator);
  const utils::agent agent_1 (agent_0);

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent_1);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_AUTO_TEST_CASE (two_workers_with_seperate_master_agent)
{
  const utils::kvs_server kvs_server;
  const utils::orchestrator orchestrator (kvs_server);

  const utils::agent agent_0 (orchestrator);
  const utils::agent agent_1 (agent_0);
  const utils::agent agent_2 (agent_0);

  const utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent_1);
  const utils::fake_drts_worker_directly_finishing_jobs worker_1 (agent_2);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_AUTO_TEST_CASE (agent_with_multiple_master_agents)
{
  const utils::kvs_server kvs_server;
  const utils::orchestrator orchestrator (kvs_server);

  const utils::agent agent_0 (orchestrator);
  const utils::agent agent_1 (orchestrator);
  const utils::agent agent_2 (agent_0, agent_1);

  const utils::fake_drts_worker_directly_finishing_jobs worker (agent_2);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (utils::module_call(), orchestrator)
                      , sdpa::status::FINISHED
                      );
}
